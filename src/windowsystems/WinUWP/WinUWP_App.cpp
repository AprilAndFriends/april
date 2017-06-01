/// @file
/// @version 4.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifdef _WINUWP_WINDOW
#include "pch.h"

#include <hltypes/hfile.h>
#include <hltypes/hlog.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hstring.h>
#include <hltypes/hthread.h>

#include "april.h"
#include "DirectX12_RenderSystem.h"
#include "Platform.h"
#include "RenderSystem.h"
#include "Window.h"
#include "WinUWP.h"
#include "WinUWP_Cursor.h"
#include "WinUWP_Window.h"
#include "WinUWP_App.h"

using namespace concurrency;
using namespace Windows::ApplicationModel;
using namespace Windows::Foundation;
using namespace Windows::UI::Core;
using namespace Windows::UI::ViewManagement;

#define DX12_RENDERSYS ((DirectX12_RenderSystem*)rendersys)

namespace april
{
	WinUWP_App::WinUWP_App()
	{
		this->running = true;
#ifndef _WINP8
		this->defaultCursor = ref new CoreCursor(CoreCursorType::Arrow, 0);
#else
		this->defaultCursor = nullptr;
#endif
		/*
		this->backgroundColor = Color::Black;
		this->launched = false;
		this->activated = false;
		*/
		this->scrollHorizontal = false;
		this->startTime = (unsigned int)htickCount();
		this->currentButton = Key::None;
	}

	void WinUWP_App::Initialize(Core::CoreApplicationView^ applicationView)
	{
		Windows::Globalization::ApplicationLanguages::PrimaryLanguageOverride = ref new Platform::String(L"");
		// Register event handlers for app lifecycle. This example includes Activated, so that we
		// can make the CoreWindow active and start rendering on the window.
		applicationView->Activated += ref new TypedEventHandler<CoreApplicationView^, IActivatedEventArgs^>(this, &WinUWP_App::OnActivated);
		CoreApplication::Suspending += ref new EventHandler<SuspendingEventArgs^>(this, &WinUWP_App::OnSuspending);
		CoreApplication::Resuming += ref new EventHandler<Platform::Object^>(this, &WinUWP_App::OnResuming);


		/*
#ifdef _DEBUG
		this->UnhandledException += ref new UnhandledExceptionEventHandler([](Object^ sender, UnhandledExceptionEventArgs^ args)
		{
			hlog::error("FATAL", _HL_PSTR_TO_HSTR(args->Message));
		});
#endif
*/
	}

	void WinUWP_App::SetWindow(CoreWindow^ window)
	{
		DisplayInformation::AutoRotationPreferences = (DisplayOrientations::Landscape | DisplayOrientations::LandscapeFlipped);
		window->SizeChanged += ref new TypedEventHandler<CoreWindow^, WindowSizeChangedEventArgs^>(this, &WinUWP_App::OnWindowSizeChanged);
		window->VisibilityChanged += ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(this, &WinUWP_App::OnVisibilityChanged);
		window->Closed += ref new TypedEventHandler<CoreWindow^, CoreWindowEventArgs^>(this, &WinUWP_App::OnWindowClosed);
		window->PointerPressed += ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &WinUWP_App::OnTouchDown);
		window->PointerReleased += ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &WinUWP_App::OnTouchUp);
		window->PointerMoved += ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &WinUWP_App::OnTouchMove);
		window->PointerWheelChanged += ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &WinUWP_App::OnMouseScroll);
		window->KeyDown += ref new TypedEventHandler<CoreWindow^, KeyEventArgs^>(this, &WinUWP_App::OnKeyDown);
		window->KeyUp += ref new TypedEventHandler<CoreWindow^, KeyEventArgs^>(this, &WinUWP_App::OnKeyUp);
		window->CharacterReceived += ref new TypedEventHandler<CoreWindow^, CharacterReceivedEventArgs^>(this, &WinUWP_App::OnCharacterReceived);
		DisplayInformation^ displayInformation = DisplayInformation::GetForCurrentView();
		displayInformation->DpiChanged += ref new TypedEventHandler<DisplayInformation^, Object^>(this, &WinUWP_App::OnDpiChanged);
		displayInformation->OrientationChanged += ref new TypedEventHandler<DisplayInformation^, Object^>(this, &WinUWP_App::OnOrientationChanged);
		DisplayInformation::DisplayContentsInvalidated += ref new TypedEventHandler<DisplayInformation^, Object^>(this, &WinUWP_App::OnDisplayContentsInvalidated);
	}

	void WinUWP_App::Load(Platform::String^ entryPoint)
	{
		(*WinUWP::Init)(WinUWP::Args);
	}

	void WinUWP_App::Run()
	{
		if (april::window != NULL)
		{
			april::window->enterMainLoop();
			(*WinUWP::Destroy)();
		}
		// On WinP8 there is a weird bug where this callback stops being called if it takes too long to process at some point so it
		// is unregistered and registered again in the main thread. Oddly enough, normal WinRT has huge problems with this code.
#ifdef _WINP8
		CoreWindow::GetForCurrentThread()->Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([this]()
		{
			this->_tryAddRenderToken();
		}));
#endif
	}

	void WinUWP_App::Uninitialize()
	{
	}

	// Application lifecycle events
	void WinUWP_App::OnActivated(CoreApplicationView^ applicationView, IActivatedEventArgs^ args)
	{
		CoreWindow::GetForCurrentThread()->Activate();
	}

	void WinUWP_App::OnSuspending(Platform::Object^ sender, SuspendingEventArgs^ args)
	{
		SuspendingDeferral^ deferral = args->SuspendingOperation->GetDeferral();
		create_task([this, deferral]()
		{
			hlog::write(logTag, "WinUWP suspending...");
			this->_handleFocusChange(false);
			deferral->Complete();
		});
	}

	void WinUWP_App::OnResuming(Platform::Object^ sender, Platform::Object^ args)
	{
		hlog::write(logTag, "WinUWP resuming...");
		this->_handleFocusChange(true);
	}

	// CoreWIndow events
	void WinUWP_App::OnWindowSizeChanged(CoreWindow^ sender, WindowSizeChangedEventArgs^ args)
	{
		args->Handled = true;
		this->_updateWindowSize(sender->Bounds.Width, sender->Bounds.Height);
	}

	void WinUWP_App::OnVisibilityChanged(CoreWindow^ sender, VisibilityChangedEventArgs^ args)
	{
		hlog::write(logTag, "WinUWP visibility change: " + hstr(args->Visible ? "true" : "false"));
		args->Handled = true;
		this->_handleFocusChange(args->Visible);
	}

	void WinUWP_App::OnWindowClosed(CoreWindow^ sender, CoreWindowEventArgs^ args)
	{
		args->Handled = true;
		if (april::window != NULL)
		{
			april::window->handleQuitRequestEvent(false);
		}
	}

	// DisplayInformation events
	void WinUWP_App::OnDpiChanged(DisplayInformation^ sender, Platform::Object^ args)
	{
		CoreWindow^ window = CoreWindow::GetForCurrentThread();
		this->_updateWindowSize(window->Bounds.Width, window->Bounds.Height);
	}

	void WinUWP_App::OnOrientationChanged(DisplayInformation^ sender, Platform::Object^ args)
	{
		CoreWindow^ window = CoreWindow::GetForCurrentThread();
		this->_updateWindowSize(window->Bounds.Width, window->Bounds.Height);
	}

	void WinUWP_App::OnDisplayContentsInvalidated(DisplayInformation^ sender, Platform::Object^ args)
	{
		//GetDeviceResources()->ValidateDevice();
	}

	void WinUWP_App::_updateWindowSize(float width, float height)
	{
		hlog::errorf("OK", "%g %g", width, height);
		getSystemInfo(); // so the displayResolution value gets updated
		this->_resetTouches();
		if (april::window != NULL)
		{
			april::window->setResolution((int)width, (int)height, ApplicationView::GetForCurrentView()->IsFullScreenMode);
		}
	}

	void WinUWP_App::_handleFocusChange(bool focused)
	{
		this->_resetTouches();
		if (april::window != NULL)
		{
			if (april::window->isFocused() != focused)
			{
				april::window->handleFocusChangeEvent(focused);
			}
		}
	}

	void WinUWP_App::_resetTouches()
	{
		for_iter_r (i, this->pointerIds.size(), 0)
		{
			april::window->queueTouchEvent(Window::MouseInputEvent::Type::Cancel, gvec2(), i);
		}
		this->pointerIds.clear();
	}

	void WinUWP_App::refreshCursor()
	{
		if (april::window != NULL)
		{
			CoreCursor^ cursor = nullptr;
			if (april::window->isCursorVisible())
			{
				Cursor* windowCursor = april::window->getCursor();
				if (windowCursor != NULL)
				{
					cursor = ((WinUWP_Cursor*)windowCursor)->getCursor();
				}
				if (cursor == nullptr)
				{
					cursor = this->defaultCursor;
				}
			}
#ifndef _WINP8
			CoreWindow::GetForCurrentThread()->PointerCursor = cursor;
#endif
		}
	}

	/*
	void WinUWP_App::OnLaunched(LaunchActivatedEventArgs^ args)
	{
		// don't repeat app initialization when already launched
		if (!this->launched)
		{
			this->launched = true;
			CoreWindow^ window = Windows::UI::Core::CoreWindow::GetForCurrentThread();
			InputPane::GetForCurrentView()->Showing +=
				ref new TypedEventHandler<InputPane^, InputPaneVisibilityEventArgs^>(
					this, &WinUWP_App::OnVirtualKeyboardShow);
			InputPane::GetForCurrentView()->Hiding +=
				ref new TypedEventHandler<InputPane^, InputPaneVisibilityEventArgs^>(
					this, &WinUWP_App::OnVirtualKeyboardHide);
#ifdef _WINP8
			HardwareButtons::BackPressed +=
				ref new EventHandler<BackPressedEventArgs^>(
					this, &WinUWP_App::OnBackButtonPressed);
			StatusBar::GetForCurrentView()->HideAsync();
#endif
			this->refreshCursor();
			this->overlay = ref new WinRT_XamlOverlay();
			Windows::UI::Xaml::Window::Current->Content = this->overlay;
			Windows::UI::Xaml::Window::Current->Activated += ref new WindowActivatedEventHandler(this, &WinUWP_App::OnWindowActivationChanged);
			Windows::UI::Input::PointerVisualizationSettings::GetForCurrentView()->IsContactFeedbackEnabled = false;
			(*WinRT::Init)(getArgs());
			if (rendersys != NULL && april::window != NULL)
			{
				float delaySplash = (float)april::window->getParam(WINRT_DELAY_SPLASH);
				if (delaySplash > 0.0f && delaySplash - (htickCount() - this->startTime) * 0.001f > 0.0f)
				{
					this->_tryLoadSplashTexture();
					this->_tryRenderSplashTexture(3);
				}
			}
		}
		Windows::UI::Xaml::Window::Current->Activate();
	}

	void WinUWP_App::OnWindowActivationChanged(_In_ Object^ sender, _In_ WindowActivatedEventArgs^ args)
	{
		args->Handled = true;
		hstr state = "unknown";
		if (args->WindowActivationState == CoreWindowActivationState::Deactivated)
		{
			state = "deactivated";
		}
		else if (args->WindowActivationState == CoreWindowActivationState::CodeActivated)
		{
			state = "code-activated";
		}
		else if (args->WindowActivationState == CoreWindowActivationState::PointerActivated)
		{
			state = "pointer-activated";
		}
		hlog::write(logTag, "WinRT window activation state changing: " + state);
		if (!this->activated)
		{
			this->activated = true;
			this->eventToken = CompositionTarget::Rendering::add(ref new EventHandler<Object^>(this, &WinUWP_App::OnRender));
			if (rendersys != NULL)
			{
				bool rendered = false;
				if (april::window != NULL)
				{
					float delaySplash = (float)april::window->getParam(WINRT_DELAY_SPLASH);
					if (delaySplash > 0.0f)
					{
						float delay = delaySplash - (htickCount() - this->startTime) * 0.001f;
						if (delay > 0.0f)
						{
							hlog::write(logTag, "Rendering splash screen for: " + hstr(delay));
							this->_tryLoadSplashTexture();
							this->_tryRenderSplashTexture(3);
							if (this->splashTexture != NULL)
							{
								rendered = true;
							}
							delay = delaySplash - (htickCount() - this->startTime) / 1000.0f;
							if (delay > 0.0f) // if there's still time left after rendering
							{
								hthread::sleep(delay * 1000.0f);
							}
						}
					}
				}
				if (this->splashTexture != NULL)
				{
					rendersys->destroyTexture(this->splashTexture);
					this->splashTexture = NULL;
				}
				if (!rendered)
				{
					// clearing all backbuffers, just in case
					for_iter (i, 0, 3)
					{
						rendersys->clear();
						rendersys->presentFrame();
					}
					rendersys->reset();
				}
			}
		}
		else if (args->WindowActivationState == CoreWindowActivationState::Deactivated)
		{
			this->_handleFocusChange(false);
		}
		else if (args->WindowActivationState == CoreWindowActivationState::CodeActivated ||
			args->WindowActivationState == CoreWindowActivationState::PointerActivated)
		{
			this->_handleFocusChange(true);
		}
	}

	void WinUWP_App::OnRender(_In_ Object^ sender, _In_ Object^ args)
	{
		if (!this->running || april::window == NULL)
		{
			this->firstFrameAfterActivateHack = false;
			return;
		}
		this->running = april::window->updateOneFrame();
		rendersys->presentFrame();
		this->firstFrameAfterActivateHack = false;
		if (!this->running)
		{
			(*WinRT::Destroy)();
			CompositionTarget::Rendering::remove(this->eventToken);
			Application::Current->Exit();
			return;
		}
		// On WinP8 there is a weird bug where this callback stops being called if it takes too long to process at some point so it
		// is unregistered and registered again in the main thread. Oddly enough, normal WinRT has huge problems with this code.
#ifdef _WINP8
		CoreWindow::GetForCurrentThread()->Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([this]()
		{
			this->_tryAddRenderToken();
		}));
#endif
	}

	void WinUWP_App::OnWindowClosed(_In_ CoreWindow^ sender, _In_ CoreWindowEventArgs^ args)
	{
		args->Handled = true;
		if (april::window != NULL)
		{
			april::window->handleQuitRequestEvent(false);
		}
	}

	void WinUWP_App::OnVirtualKeyboardShow(_In_ InputPane^ sender, _In_ InputPaneVisibilityEventArgs^ args)
	{
		if (april::window != NULL)
		{
			april::window->handleVirtualKeyboardChangeEvent(true, args->OccludedRect.Height / CoreWindow::GetForCurrentThread()->Bounds.Height);
		}
		this->_resetTouches();
	}

	void WinUWP_App::OnVirtualKeyboardHide(_In_ InputPane^ sender, _In_ InputPaneVisibilityEventArgs^ args)
	{
		if (april::window != NULL)
		{
			april::window->handleVirtualKeyboardChangeEvent(false, 0.0f);
		}
		this->_resetTouches();
	}
	*/

	void WinUWP_App::OnTouchDown(_In_ CoreWindow^ sender, _In_ PointerEventArgs^ args)
	{
		args->Handled = true;
		if (april::window == NULL || !april::window->isFocused())
		{
			return;
		}
		unsigned int id;
		int index;
		gvec2 position = this->_transformPosition(args->CurrentPoint->Position.X, args->CurrentPoint->Position.Y);
#ifndef _WINP8
		this->currentButton = Key::MouseL;
		switch (args->CurrentPoint->PointerDevice->PointerDeviceType)
		{
		case Windows::Devices::Input::PointerDeviceType::Mouse:
			april::window->setInputMode(InputMode::Mouse);
			if (args->CurrentPoint->Properties->IsRightButtonPressed)
			{
				this->currentButton = Key::MouseR;
			}
			else if (args->CurrentPoint->Properties->IsMiddleButtonPressed)
			{
				this->currentButton = Key::MouseM;
			}
			april::window->queueMouseEvent(Window::MouseInputEvent::Type::Down, position, this->currentButton);
			break;
		case Windows::Devices::Input::PointerDeviceType::Touch:
		case Windows::Devices::Input::PointerDeviceType::Pen:
#endif
			april::window->setInputMode(InputMode::Touch);
			id = args->CurrentPoint->PointerId;
			index = this->pointerIds.indexOf(id);
			if (index < 0)
			{
				index = this->pointerIds.size();
				this->pointerIds += id;
			}
			april::window->queueTouchEvent(Window::MouseInputEvent::Type::Down, position, index);
#ifndef _WINP8
			break;
		}
#endif
	}

	void WinUWP_App::OnTouchUp(_In_ CoreWindow^ sender, _In_ PointerEventArgs^ args)
	{
		args->Handled = true;
		if (april::window == NULL || !april::window->isFocused())
		{
			return;
		}
		unsigned int id;
		int index;
		gvec2 position = this->_transformPosition(args->CurrentPoint->Position.X, args->CurrentPoint->Position.Y);
#ifndef _WINP8
		switch (args->CurrentPoint->PointerDevice->PointerDeviceType)
		{
		case Windows::Devices::Input::PointerDeviceType::Mouse:
			april::window->setInputMode(InputMode::Mouse);
			april::window->queueMouseEvent(Window::MouseInputEvent::Type::Up, position, this->currentButton);
			break;
		case Windows::Devices::Input::PointerDeviceType::Touch:
		case Windows::Devices::Input::PointerDeviceType::Pen:
#endif
			april::window->setInputMode(InputMode::Touch);
			id = args->CurrentPoint->PointerId;
			index = this->pointerIds.indexOf(id);
			if (index < 0)
			{
				index = this->pointerIds.size();
			}
			else
			{
				this->pointerIds.removeAt(index);
			}
			april::window->queueTouchEvent(Window::MouseInputEvent::Type::Up, position, index);
#ifndef _WINP8
			break;
		}
		this->currentButton = Key::None;
#endif
	}

	void WinUWP_App::OnTouchMove(_In_ CoreWindow^ sender, _In_ PointerEventArgs^ args)
	{
		args->Handled = true;
		if (april::window == NULL || !april::window->isFocused())
		{
			return;
		}
		unsigned int id;
		int index;
		gvec2 position = this->_transformPosition(args->CurrentPoint->Position.X, args->CurrentPoint->Position.Y);
#ifndef _WINP8
		switch (args->CurrentPoint->PointerDevice->PointerDeviceType)
		{
		case Windows::Devices::Input::PointerDeviceType::Mouse:
			april::window->setInputMode(InputMode::Mouse);
			april::window->queueMouseEvent(Window::MouseInputEvent::Type::Move, position, this->currentButton);
			break;
		case Windows::Devices::Input::PointerDeviceType::Touch:
		case Windows::Devices::Input::PointerDeviceType::Pen:
#endif
			april::window->setInputMode(InputMode::Touch);
			id = args->CurrentPoint->PointerId;
			index = this->pointerIds.indexOf(id);
			if (index < 0)
			{
				index = this->pointerIds.size();
			}
			april::window->queueTouchEvent(Window::MouseInputEvent::Type::Move, position, index);
#ifndef _WINP8
			break;
		}
#endif
	}

	void WinUWP_App::OnMouseScroll(_In_ CoreWindow^ sender, _In_ PointerEventArgs^ args)
	{
		args->Handled = true;
		if (april::window == NULL || !april::window->isFocused())
		{
			return;
		}
		april::window->setInputMode(InputMode::Mouse);
		float _wheelDelta = (float)args->CurrentPoint->Properties->MouseWheelDelta / WHEEL_DELTA;
		if (this->scrollHorizontal ^ args->CurrentPoint->Properties->IsHorizontalMouseWheel)
		{
			april::window->queueMouseEvent(Window::MouseInputEvent::Type::Scroll, gvec2(-(float)_wheelDelta, 0.0f), Key::None);
		}
		else
		{
			april::window->queueMouseEvent(Window::MouseInputEvent::Type::Scroll, gvec2(0.0f, -(float)_wheelDelta), Key::None);
		}
	}

	void WinUWP_App::OnKeyDown(_In_ CoreWindow^ sender, _In_ KeyEventArgs^ args)
	{
		args->Handled = true;
		if (april::window == NULL || !april::window->isFocused())
		{
			return;
		}
		Key key = Key::fromInt((int)args->VirtualKey);
		april::window->queueKeyEvent(Window::KeyInputEvent::Type::Down, key, 0);
		if (key == Key::Control || key == Key::ControlL || key == Key::ControlR)
		{
			this->scrollHorizontal = true;
		}
	}

	void WinUWP_App::OnKeyUp(_In_ CoreWindow^ sender, _In_ KeyEventArgs^ args)
	{
		args->Handled = true;
		if (april::window == NULL || !april::window->isFocused())
		{
			return;
		}
		Key key = Key::fromInt((int)args->VirtualKey);
		april::window->queueKeyEvent(Window::KeyInputEvent::Type::Up, key, 0);
		if (key == Key::Control || key == Key::ControlL || key == Key::ControlR)
		{
			this->scrollHorizontal = false;
		}
		else if (key == Key::Return)
		{
			april::window->hideVirtualKeyboard();
		}
	}

	void WinUWP_App::OnCharacterReceived(_In_ CoreWindow^ sender, _In_ CharacterReceivedEventArgs^ args)
	{
		args->Handled = true;
		if (april::window == NULL || !april::window->isFocused())
		{
			return;
		}
		april::window->queueKeyEvent(Window::KeyInputEvent::Type::Down, Key::None, args->KeyCode);
	}

	gvec2 WinUWP_App::_transformPosition(float x, float y)
	{
		// WinUWP is dumb
		return (gvec2(x, y) * WinUWP::getDpiRatio());
	}

	/*
	void WinUWP_App::_tryRenderSplashTexture(int count)
	{
		if (this->splashTexture == NULL)
		{
			return;
		}
		static gmat4 storedProjectionMatrix;
		static gmat4 storedModelviewMatrix;
		static grect drawRect(0.0f, 0.0f, 1.0f, 1.0f);
		static grect viewport(0.0f, 0.0f, 1.0f, 1.0f);
		static grect src(0.0f, 0.0f, 1.0f, 1.0f);
		static gvec2 windowSize;
		static gvec2 textureSize;
		storedProjectionMatrix = rendersys->getProjectionMatrix();
		storedModelviewMatrix = rendersys->getModelviewMatrix();
		windowSize = april::window->getSize();
		viewport.setSize(windowSize);
		rendersys->setOrthoProjection(viewport);
#ifndef _WINP8
		textureSize.set(SPLASH_WIDTH, SPLASH_HEIGHT);
		textureSize *= (float)DisplayInformation::GetForCurrentView()->ResolutionScale * 0.01f;
#else // on WinP8 the splash graphic is rotated by -90° and needs to be stretched over the entire screen
		rendersys->translate(windowSize.x * 0.5f, windowSize.y * 0.5f);
		hswap(windowSize.x, windowSize.y);
		hswap(viewport.w, viewport.h);
		rendersys->rotate(90.0f);
		rendersys->translate(-windowSize.x * 0.5f, -windowSize.y * 0.5f);
		textureSize.set(windowSize.x, windowSize.x * this->splashTexture->getHeight() / this->splashTexture->getWidth());
#endif
		drawRect.set(hroundf(windowSize.x - textureSize.x) * 0.5f, hroundf(windowSize.y - textureSize.y) * 0.5f, textureSize);
		rendersys->setBlendMode(BlendMode::Alpha);
		rendersys->setColorMode(ColorMode::Multiply);
		// rendering X times to avoid buffer swap problems
		for_iter (i, 0, count)
		{
			rendersys->drawFilledRect(viewport, this->backgroundColor);
			if (this->splashTexture != NULL)
			{
				rendersys->setTexture(this->splashTexture);
				rendersys->drawTexturedRect(drawRect, src);
			}
			rendersys->presentFrame();
		}
		rendersys->reset();
		rendersys->setProjectionMatrix(storedProjectionMatrix);
		rendersys->setModelviewMatrix(storedModelviewMatrix);
	}

	Texture* WinUWP_App::_tryLoadTexture(chstr nodeName, chstr attributeName)
	{
		if (rendersys == NULL)
		{
			return NULL;
		}
		hstr data;
		int index = 0;
		if (!this->_findVisualElements(nodeName, data, index))
		{
			return NULL;
		}
		// finding the logo entry in XML
		int logoIndex = data.indexOf(attributeName + "=\"");
		if (logoIndex >= 0)
		{
			index = logoIndex + hstr(attributeName + "=\"").size();
			hstr logoFilename = data(index, -1);
			index = logoFilename.indexOf("\"");
			if (index >= 0)
			{
				logoFilename = logoFilename(0, index);
				harray<hstr> filenames;
				index = logoFilename.rindexOf('.');
				// adding those ".scale-x" things here, because my prayers went unanswered and Microsoft decided to change the format after all
#ifdef _WINUWP
				filenames += logoFilename(0, index) + ".scale-" + hstr((int)DisplayInformation::GetForCurrentView()->ResolutionScale);
				filenames += logoFilename(0, index) + ".scale-400";
				filenames += logoFilename(0, index) + ".scale-200";
				filenames += logoFilename(0, index) + ".scale-150";
				filenames += logoFilename(0, index) + ".scale-125";
				filenames += logoFilename(0, index) + ".scale-100";
#else
#ifndef _WINP8 // for some unknown reason, on WinP8 "ResolutionScale" keeps throwing deprecated warnings and "RawPixelsPerViewPixel" is not available on normal WinRT
				filenames += logoFilename(0, index) + ".scale-" + hstr((int)DisplayInformation::GetForCurrentView()->ResolutionScale);
				filenames += logoFilename(0, index) + ".scale-180";
#else
				filenames += logoFilename(0, index) + ".scale-" + hstr((int)(DisplayInformation::GetForCurrentView()->RawPixelsPerViewPixel * 100));
				filenames += logoFilename(0, index) + ".scale-240";
#endif
				filenames += logoFilename(0, index) + ".scale-140";
				filenames += logoFilename(0, index) + ".scale-100";
#ifndef _WINP8
				filenames += logoFilename(0, index) + ".scale-80";
#endif
#endif
				filenames += logoFilename(0, index);
				filenames.removeDuplicates();
				foreach (hstr, it, filenames)
				{
					// loading the logo file
					Texture* texture = rendersys->createTextureFromResource((*it), Texture::Type::Immutable, Texture::LoadMode::Async);
					if (texture != NULL)
					{
						try
						{
							texture->load();
							return texture;
						}
						catch (hexception&)
						{
							rendersys->destroyTexture(texture);
						}
					}
				}
			}
		}
		return NULL;
	}

	void WinUWP_App::_tryLoadSplashTexture()
	{
		if (this->splashTexture == NULL)
		{
			this->splashTexture = this->_tryLoadTexture("SplashScreen", "Image");
		}
		this->_tryLoadBackgroundColor();
	}

	void WinUWP_App::_tryLoadBackgroundColor()
	{
		hstr data;
		int index = 0;
		if (!this->_findVisualElements("VisualElements", data, index))
		{
			return;
		}
		// finding the color indexOf in XML
		int colorIndex = data.indexOf("BackgroundColor=\"");
		if (colorIndex >= 0)
		{
			index = colorIndex + hstr("BackgroundColor=\"").size();
			hstr colorString = data(index, -1);
			index = colorString.indexOf('"');
			if (index >= 0)
			{
				// loading the color string
				colorString = colorString(0, index).trimmedLeft('#');
				this->backgroundColor = Color::Black;
				if (colorString.isHex() && colorString.size() >= 6)
				{
					if (colorString.size() > 6)
					{
						colorString = colorString(0, 6);
					}
					this->backgroundColor.set(colorString);
				}
			}
		}
	}

	bool WinUWP_App::_findVisualElements(chstr nodeName, hstr& data, int& index)
	{
		if (!hfile::exists(MANIFEST_FILENAME))
		{
			return false;
		}
		data = hfile::hread(MANIFEST_FILENAME); // lets hope Microsoft does not change the format of these
		// locating the right entry in XML
		index = data.indexOf("<Applications>");
		if (index < 0)
		{
			return false;
		}
		data = data(index, -1);
		index = data.indexOf("<Application ");
		if (index < 0)
		{
			return false;
		}
		data = data(index, -1);
		index = data.indexOf("<" NODE_PREFIX + nodeName + " ");
		if (index < 0)
		{
			return false;
		}
		data = data(index, -1);
		return true;
	}
	*/

IFrameworkView^ FrameworkViewSource::CreateView()
{
	WinUWP::App = ref new WinUWP_App();
	return WinUWP::App;
}

}
#endif
