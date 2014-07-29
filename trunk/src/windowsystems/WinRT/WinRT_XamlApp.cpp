/// @file
/// @version 3.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifdef _WINRT_WINDOW
#include "pch.h"

#include <hltypes/hfile.h>
#include <hltypes/hlog.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hstring.h>
#include <hltypes/hthread.h>

#include "april.h"
#include "DirectX11_RenderSystem.h"
#include "Platform.h"
#include "RenderSystem.h"
#include "Window.h"
#include "WinRT.h"
#include "WinRT_XamlApp.h"
#include "WinRT_Cursor.h"
#include "WinRT_Window.h"
#include "WinRT_XamlApp.h"

using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::Foundation;
using namespace Windows::UI::Core;
using namespace Windows::UI::ViewManagement;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Media;

#define MANIFEST_FILENAME "AppxManifest.xml"
#ifndef _WINP8
#define NODE_PREFIX "m2:"
#define SPLASH_WIDTH 620
#define SPLASH_HEIGHT 300
#else
#define NODE_PREFIX "m3:"
#define SPLASH_WIDTH 800
#define SPLASH_HEIGHT 480
#endif

#define DX11_RENDERSYS ((DirectX11_RenderSystem*)april::rendersys)

namespace april
{
	WinRT_XamlApp::WinRT_XamlApp() : Application()
	{
		DisplayInformation::AutoRotationPreferences = (DisplayOrientations::Landscape | DisplayOrientations::LandscapeFlipped);
		this->running = true;
		this->overlay = nullptr;
#ifndef _WINP8
		this->defaultCursor = ref new CoreCursor(CoreCursorType::Arrow, 0);
#else
		this->defaultCursor = nullptr;
#endif
		this->backgroundColor = april::Color::Black;
		this->launched = false;
		this->activated = false;
		this->firstFrameAfterActivateHack = false;
		this->scrollHorizontal = false;
		this->startTime = get_system_tick_count();
		this->currentButton = april::AK_NONE;
		this->Suspending += ref new SuspendingEventHandler(this, &WinRT_XamlApp::OnSuspend);
		this->Resuming += ref new EventHandler<Object^>(this, &WinRT_XamlApp::OnResume);
#ifdef _DEBUG
		this->UnhandledException += ref new UnhandledExceptionEventHandler([](Object^ sender, UnhandledExceptionEventArgs^ args)
		{
			hlog::error("FATAL", _HL_PSTR_TO_HSTR(args->Message));
		});
#endif
	}

	WinRT_XamlApp::~WinRT_XamlApp()
	{
	}

	void WinRT_XamlApp::refreshCursor()
	{
		if (april::window != NULL)
		{
			CoreCursor^ cursor = nullptr;
			if (april::window->isCursorVisible())
			{
				Cursor* windowCursor = april::window->getCursor();
				if (windowCursor != NULL)
				{
					cursor = ((WinRT_Cursor*)windowCursor)->getCursor();
				}
				if (cursor == nullptr)
				{
					cursor = this->defaultCursor;
				}
			}
#ifndef _WINP8
			Windows::UI::Xaml::Window::Current->CoreWindow->PointerCursor = cursor;
#endif
		}
	}

	void WinRT_XamlApp::Connect(int connectionId, Object^ target)
	{
	}

	void WinRT_XamlApp::OnLaunched(LaunchActivatedEventArgs^ args)
	{
		// don't repeat app initialization when already launched
		if (!this->launched)
		{
			this->launched = true;
			CoreWindow^ window = Windows::UI::Core::CoreWindow::GetForCurrentThread();
			window->SizeChanged +=
				ref new TypedEventHandler<CoreWindow^, WindowSizeChangedEventArgs^>(
					this, &WinRT_XamlApp::OnWindowSizeChanged);
			window->VisibilityChanged +=
				ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(
					this, &WinRT_XamlApp::OnVisibilityChanged);
			window->PointerPressed +=
				ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(
					this, &WinRT_XamlApp::OnTouchDown);
			window->PointerReleased +=
				ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(
					this, &WinRT_XamlApp::OnTouchUp);
			window->PointerMoved +=
				ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(
					this, &WinRT_XamlApp::OnTouchMove);
			window->PointerWheelChanged +=
				ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(
					this, &WinRT_XamlApp::OnMouseScroll);
			window->KeyDown +=
				ref new TypedEventHandler<CoreWindow^, KeyEventArgs^>(
					this, &WinRT_XamlApp::OnKeyDown);
			window->KeyUp +=
				ref new TypedEventHandler<CoreWindow^, KeyEventArgs^>(
					this, &WinRT_XamlApp::OnKeyUp);
			window->CharacterReceived +=
				ref new TypedEventHandler<CoreWindow^, CharacterReceivedEventArgs^>(
					this, &WinRT_XamlApp::OnCharacterReceived);
			window->Closed +=
				ref new TypedEventHandler<CoreWindow^, CoreWindowEventArgs^>(
					this, &WinRT_XamlApp::OnWindowClosed);
			DisplayInformation::GetForCurrentView()->OrientationChanged +=
				ref new TypedEventHandler<DisplayInformation^, Object^>(
					this, &WinRT_XamlApp::OnOrientationChanged);
			DisplayInformation::GetForCurrentView()->DpiChanged +=
				ref new TypedEventHandler<DisplayInformation^, Object^>(
					this, &WinRT_XamlApp::OnDpiChanged);
			InputPane::GetForCurrentView()->Showing +=
				ref new TypedEventHandler<InputPane^, InputPaneVisibilityEventArgs^>(
					this, &WinRT_XamlApp::OnVirtualKeyboardShow);
			InputPane::GetForCurrentView()->Hiding +=
				ref new TypedEventHandler<InputPane^, InputPaneVisibilityEventArgs^>(
					this, &WinRT_XamlApp::OnVirtualKeyboardHide);
			this->refreshCursor();
			this->overlay = ref new WinRT_XamlOverlay();
			Windows::UI::Xaml::Window::Current->Content = this->overlay;
			Windows::UI::Xaml::Window::Current->Activated += ref new WindowActivatedEventHandler(this, &WinRT_XamlApp::OnWindowActivationChanged);
			(*WinRT::Init)(WinRT::Args);
			if (april::rendersys != NULL && april::window != NULL)
			{
				float delaySplash = (float)april::window->getParam(WINRT_DELAY_SPLASH);
				if (delaySplash > 0.0f && delaySplash - (get_system_tick_count() - this->startTime) * 0.001f > 0.0f)
				{
					this->_tryLoadSplashTexture();
					this->_tryRenderSplashTexture();
				}
			}
		}
		Windows::UI::Xaml::Window::Current->Activate();
	}

	void WinRT_XamlApp::OnWindowActivationChanged( _In_ Object^ sender, _In_ WindowActivatedEventArgs^ args)
	{
		if (!this->activated)
		{
			this->activated = true;
			this->eventToken = CompositionTarget::Rendering::add(ref new EventHandler<Object^>(this, &WinRT_XamlApp::OnRender));
			if (april::rendersys != NULL)
			{
				bool rendered = false;
				if (april::window != NULL)
				{
					float delaySplash = (float)april::window->getParam(WINRT_DELAY_SPLASH);
					if (delaySplash > 0.0f)
					{
						float delay = delaySplash - (get_system_tick_count() - this->startTime) * 0.001f;
						if (delay > 0.0f)
						{
							hlog::write(april::logTag, "Rendering splash screen for: " + hstr(delay));
							this->_tryLoadSplashTexture();
							this->_tryRenderSplashTexture();
							if (this->splashTexture != NULL)
							{
								rendered = true;
							}
							delay = delaySplash - (get_system_tick_count() - this->startTime) / 1000.0f;
							if (delay > 0.0f) // if there's still time left after rendering
							{
								hthread::sleep(delay * 1000.0f);
							}
						}
					}
				}
				if (this->splashTexture != NULL)
				{
					delete this->splashTexture;
					this->splashTexture = NULL;
				}
				if (!rendered)
				{
					april::rendersys->clear();
					april::rendersys->presentFrame();
					april::rendersys->reset();
				}
			}
		}
		else if (args->WindowActivationState == CoreWindowActivationState::Deactivated)
		{
			this->_handleFocusChange(false);
			if (this->eventToken.Value != 0)
			{
				CompositionTarget::Rendering::remove(this->eventToken);
				this->eventToken.Value = 0;
			}
		}
		else if (args->WindowActivationState == CoreWindowActivationState::CodeActivated ||
			args->WindowActivationState == CoreWindowActivationState::PointerActivated)
		{
			this->firstFrameAfterActivateHack = true;
			this->_handleFocusChange(true);
			if (this->eventToken.Value == 0)
			{
				this->eventToken = CompositionTarget::Rendering::add(ref new EventHandler<Object^>(this, &WinRT_XamlApp::OnRender));
			}
		}
	}

	void WinRT_XamlApp::OnRender(_In_ Object^ sender, _In_ Object^ args)
	{
		if (!this->running || april::window == NULL)
		{
			this->firstFrameAfterActivateHack = false;
			return;
		}
		this->running = april::window->updateOneFrame();
		april::rendersys->presentFrame();
		this->firstFrameAfterActivateHack = false;
		if (!this->running)
		{
			(*WinRT::Destroy)();
			CompositionTarget::Rendering::remove(this->eventToken);
			hlog::warn(april::logTag, "Manual closing in WinRT apps should not be used!");
		}
	}

	void WinRT_XamlApp::OnSuspend(_In_ Object^ sender, _In_ SuspendingEventArgs^ args)
	{
		hlog::write(april::logTag, "WinRT suspending...");
		DX11_RENDERSYS->trim(); // required since Win 8.1
		this->_handleFocusChange(false);
	}

	void WinRT_XamlApp::OnResume(_In_ Object^ sender, _In_ Object^ args)
	{
		hlog::write(april::logTag, "WinRT resuming...");
		this->_handleFocusChange(true);
	}

	void WinRT_XamlApp::OnWindowClosed(_In_ CoreWindow^ sender, _In_ CoreWindowEventArgs^ args)
	{
		if (april::window != NULL)
		{
			april::window->handleQuitRequest(false);
		}
		args->Handled = true;
	}

	void WinRT_XamlApp::_handleFocusChange(bool focused)
	{
		this->_resetTouches();
		if (april::window != NULL && april::window->isFocused() != focused)
		{
			april::window->handleFocusChangeEvent(focused);
		}
	}

	void WinRT_XamlApp::OnWindowSizeChanged(_In_ CoreWindow^ sender, _In_ WindowSizeChangedEventArgs^ args)
	{
		this->_resetTouches();
		april::SystemInfo info = april::getSystemInfo(); // outside, because the displayResolution needs to be updated every time
		// these orientations are not supported, but Windows reports them anyway, even if the manifest says that they aren't supported
		if (DisplayInformation::GetForCurrentView()->CurrentOrientation == DisplayOrientations::Portrait ||
			DisplayInformation::GetForCurrentView()->CurrentOrientation == DisplayOrientations::PortraitFlipped)
		{
			return;
		}
		if (april::window != NULL)
		{
			int width = hround(args->Size.Width * info.displayDpi / 96.0f);
			int height = hround(args->Size.Height * info.displayDpi / 96.0f);
			((WinRT_Window*)april::window)->changeSize(width, height);
		}
		args->Handled = true;
	}

	void WinRT_XamlApp::OnVisibilityChanged(_In_ CoreWindow^ sender, _In_ VisibilityChangedEventArgs^ args)
	{
		this->_resetTouches();
		args->Handled = true;
	}

	void WinRT_XamlApp::OnOrientationChanged(_In_ DisplayInformation^ sender, _In_ Object^ args)
	{
		this->_resetTouches();
	}

	void WinRT_XamlApp::OnDpiChanged(_In_ DisplayInformation^ sender, _In_ Object^ args)
	{
		this->_resetTouches();
		april::getSystemInfo(); // so the DPI value gets updated
	}

	void WinRT_XamlApp::OnVirtualKeyboardShow(_In_ InputPane^ sender, _In_ InputPaneVisibilityEventArgs^ args)
	{
		if (april::window != NULL)
		{
			april::window->handleVirtualKeyboardChangeEvent(true, args->OccludedRect.Height / CoreWindow::GetForCurrentThread()->Bounds.Height);
		}
		this->_resetTouches();
	}

	void WinRT_XamlApp::OnVirtualKeyboardHide(_In_ InputPane^ sender, _In_ InputPaneVisibilityEventArgs^ args)
	{
		if (april::window != NULL)
		{
			april::window->handleVirtualKeyboardChangeEvent(false, 0.0f);
		}
		this->_resetTouches();
	}

	void WinRT_XamlApp::OnTouchDown(_In_ CoreWindow^ sender, _In_ PointerEventArgs^ args)
	{
		if (april::window == NULL || !april::window->isFocused() || this->firstFrameAfterActivateHack)
		{
			return;
		}
		unsigned int id;
		int index;
		gvec2 position = this->_transformPosition(args->CurrentPoint->Position.X, args->CurrentPoint->Position.Y);
#ifndef _WINP8
		this->currentButton = april::AK_LBUTTON;
		switch (args->CurrentPoint->PointerDevice->PointerDeviceType)
		{
		case Windows::Devices::Input::PointerDeviceType::Mouse:
			april::window->setInputMode(april::Window::MOUSE);
			if (args->CurrentPoint->Properties->IsRightButtonPressed)
			{
				this->currentButton = april::AK_RBUTTON;
			}
			else if (args->CurrentPoint->Properties->IsMiddleButtonPressed)
			{
				this->currentButton = april::AK_MBUTTON;
			}
			april::window->queueMouseEvent(april::Window::MOUSE_DOWN, position, this->currentButton);
			break;
		case Windows::Devices::Input::PointerDeviceType::Touch:
		case Windows::Devices::Input::PointerDeviceType::Pen:
#endif
			april::window->setInputMode(april::Window::TOUCH);
			id = args->CurrentPoint->PointerId;
			index = this->pointerIds.index_of(id);
			if (index < 0)
			{
				index = this->pointerIds.size();
				this->pointerIds += id;
			}
			april::window->queueTouchEvent(april::Window::MOUSE_DOWN, position, index);
#ifndef _WINP8
			break;
		}
#endif
		args->Handled = true;
	}

	void WinRT_XamlApp::OnTouchUp(_In_ CoreWindow^ sender, _In_ PointerEventArgs^ args)
	{
		if (april::window == NULL || !april::window->isFocused() || this->firstFrameAfterActivateHack)
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
			april::window->setInputMode(april::Window::MOUSE);
			april::window->queueMouseEvent(april::Window::MOUSE_UP, position, this->currentButton);
			break;
		case Windows::Devices::Input::PointerDeviceType::Touch:
		case Windows::Devices::Input::PointerDeviceType::Pen:
#endif
			april::window->setInputMode(april::Window::TOUCH);
			id = args->CurrentPoint->PointerId;
			index = this->pointerIds.index_of(id);
			if (index < 0)
			{
				index = this->pointerIds.size();
			}
			else
			{
				this->pointerIds.remove_at(index);
			}
			april::window->queueTouchEvent(april::Window::MOUSE_UP, position, index);
#ifndef _WINP8
			break;
		}
		this->currentButton = april::AK_NONE;
#endif
		args->Handled = true;
	}

	void WinRT_XamlApp::OnTouchMove(_In_ CoreWindow^ sender, _In_ PointerEventArgs^ args)
	{
		if (april::window == NULL || !april::window->isFocused() || this->firstFrameAfterActivateHack)
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
			april::window->setInputMode(april::Window::MOUSE);
			april::window->queueMouseEvent(april::Window::MOUSE_MOVE, position, this->currentButton);
			break;
		case Windows::Devices::Input::PointerDeviceType::Touch:
		case Windows::Devices::Input::PointerDeviceType::Pen:
#endif
			april::window->setInputMode(april::Window::TOUCH);
			id = args->CurrentPoint->PointerId;
			index = this->pointerIds.index_of(id);
			if (index < 0)
			{
				index = this->pointerIds.size();
			}
			april::window->queueTouchEvent(april::Window::MOUSE_MOVE, position, index);
#ifndef _WINP8
			break;
		}
#endif
		args->Handled = true;
	}

	void WinRT_XamlApp::OnMouseScroll(_In_ CoreWindow^ sender, _In_ PointerEventArgs^ args)
	{
		if (april::window == NULL || !april::window->isFocused())
		{
			return;
		}
		april::window->setInputMode(april::Window::MOUSE);
		float _wheelDelta = (float)args->CurrentPoint->Properties->MouseWheelDelta / WHEEL_DELTA;
		if (this->scrollHorizontal ^ args->CurrentPoint->Properties->IsHorizontalMouseWheel)
		{
			april::window->queueMouseEvent(april::Window::MOUSE_SCROLL,
				gvec2(-(float)_wheelDelta, 0.0f), april::AK_NONE);
		}
		else
		{
			april::window->queueMouseEvent(april::Window::MOUSE_SCROLL,
				gvec2(0.0f, -(float)_wheelDelta), april::AK_NONE);
		}
		args->Handled = true;
	}

	void WinRT_XamlApp::OnKeyDown(_In_ CoreWindow^ sender, _In_ KeyEventArgs^ args)
	{
		if (april::window == NULL || !april::window->isFocused())
		{
			return;
		}
		april::Key key = (april::Key)args->VirtualKey;
		april::window->queueKeyEvent(april::Window::KEY_DOWN, key, 0);
		if (key == AK_CONTROL || key == AK_LCONTROL || key == AK_RCONTROL)
		{
			this->scrollHorizontal = true;
		}
		args->Handled = true;
	}

	void WinRT_XamlApp::OnKeyUp(_In_ CoreWindow^ sender, _In_ KeyEventArgs^ args)
	{
		if (april::window == NULL || !april::window->isFocused())
		{
			return;
		}
		april::Key key = (april::Key)args->VirtualKey;
		april::window->queueKeyEvent(april::Window::KEY_UP, key, 0);
		if (key == AK_CONTROL || key == AK_LCONTROL || key == AK_RCONTROL)
		{
			this->scrollHorizontal = false;
		}
		if (key == AK_RETURN)
		{
			april::window->terminateKeyboardHandling();
		}
		args->Handled = true;
	}

	void WinRT_XamlApp::OnCharacterReceived(_In_ CoreWindow^ sender, _In_ CharacterReceivedEventArgs^ args)
	{
		if (april::window == NULL || !april::window->isFocused())
		{
			return;
		}
		april::window->queueKeyEvent(april::Window::KEY_DOWN, AK_NONE, args->KeyCode);
		args->Handled = true;
	}

	gvec2 WinRT_XamlApp::_transformPosition(float x, float y)
	{
		// WinRT is dumb
		return (gvec2(x, y) * april::getSystemInfo().displayDpi / 96.0f);
	}

	void WinRT_XamlApp::_resetTouches()
	{
		for_iter_r (i, this->pointerIds.size(), 0)
		{
			april::window->queueTouchEvent(april::Window::MOUSE_CANCEL, gvec2(), i);
		}
		this->pointerIds.clear();
	}

	void WinRT_XamlApp::_tryRenderSplashTexture()
	{
		if (this->splashTexture != NULL)
		{
			grect storedOrthoProjection = april::rendersys->getOrthoProjection();
			gmat4 storedProjectionMatrix = april::rendersys->getProjectionMatrix();
			grect drawRect(0.0f, 0.0f, 1.0f, 1.0f);
			grect viewport(0.0f, 0.0f, 1.0f, 1.0f);
			float width = (float)april::window->getWidth();
			float height = (float)april::window->getHeight();
			viewport.setSize(width, height);
			april::rendersys->setOrthoProjection(viewport);
			april::rendersys->drawFilledRect(viewport, this->backgroundColor);
#ifndef _WINP8 // for some unknown reason, on WinP8 "ResolutionScale" keeps throwing deprecated warnings and "RawPixelsPerViewPixel" is not available on normal WinRT
			float scale = (float)DisplayInformation::GetForCurrentView()->ResolutionScale * 0.01f;
#else
			float scale = (float)DisplayInformation::GetForCurrentView()->RawPixelsPerViewPixel;
#endif
			float textureWidth = SPLASH_WIDTH * scale;
			float textureHeight = SPLASH_HEIGHT * scale;
			drawRect.set(hroundf(width - textureWidth) * 0.5f, hroundf(height - textureHeight) * 0.5f, textureWidth, textureHeight);
			april::rendersys->setTexture(this->splashTexture);
			april::rendersys->drawTexturedRect(drawRect, grect(0.0f, 0.0f, 1.0f, 1.0f));
			april::rendersys->presentFrame();
			april::rendersys->reset();
			april::rendersys->setOrthoProjection(storedOrthoProjection);
			april::rendersys->setProjectionMatrix(storedProjectionMatrix);
		}
	}

	april::Texture* WinRT_XamlApp::_tryLoadTexture(chstr nodeName, chstr attributeName)
	{
		if (april::rendersys == NULL)
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
		int logoIndex = data.find(attributeName + "=\"");
		if (logoIndex >= 0)
		{
			index = logoIndex + hstr(attributeName + "=\"").size();
			hstr logoFilename = data(index, -1);
			index = logoFilename.find("\"");
			if (index >= 0)
			{
				logoFilename = logoFilename(0, index);
				harray<hstr> filenames;
				index = logoFilename.rfind('.');
				// adding those ".scale-x" things here, because my prayers went unanswered and Microsoft decided to change the format after all
#ifndef _WINP8 // for some unknown reason, on WinP8 "ResolutionScale" keeps throwing deprecated warnings and "RawPixelsPerViewPixel" is not available on normal WinRT
				filenames += logoFilename(0, index) + ".scale-" + hstr((int)DisplayInformation::GetForCurrentView()->ResolutionScale) + logoFilename(index, -1);
#else
				filenames += logoFilename(0, index) + ".scale-" + hstr((int)(DisplayInformation::GetForCurrentView()->RawPixelsPerViewPixel * 100)) + logoFilename(index, -1);
#endif
				
#ifndef _WINP8
				filenames += logoFilename(0, index) + ".scale-180" + logoFilename(index, -1);
#else
				filenames += logoFilename(0, index) + ".scale-240" + logoFilename(index, -1);
#endif
				filenames += logoFilename(0, index) + ".scale-140" + logoFilename(index, -1);
				filenames += logoFilename(0, index) + ".scale-100" + logoFilename(index, -1);
#ifndef _WINP8
				filenames += logoFilename(0, index) + ".scale-80" + logoFilename(index, -1);
#endif
				filenames += logoFilename(0, index);
				filenames.remove_duplicates();
				foreach (hstr, it, filenames)
				{
					// loading the logo file
					april::Texture* texture = april::rendersys->createTextureFromResource((*it), Texture::TYPE_IMMUTABLE, false);
					if (texture != NULL)
					{
						try
						{
							texture->load();
							return texture;
						}
						catch (hltypes::exception&)
						{
							delete texture;
						}
					}
				}
			}
		}
		return NULL;
	}

	void WinRT_XamlApp::_tryLoadSplashTexture()
	{
		if (this->splashTexture == NULL)
		{
			this->splashTexture = this->_tryLoadTexture("SplashScreen", "Image");
		}
		this->_tryLoadBackgroundColor();
	}

	void WinRT_XamlApp::_tryLoadBackgroundColor()
	{
		hstr data;
		int index = 0;
		if (!this->_findVisualElements("VisualElements", data, index))
		{
			return;
		}
		// finding the color entry in XML
		int colorIndex = data.find("BackgroundColor=\"");
		if (colorIndex >= 0)
		{
			index = colorIndex + hstr("BackgroundColor=\"").size();
			hstr colorString = data(index, -1);
			index = colorString.find("\"");
			if (index >= 0)
			{
				// loading the color string
				colorString = colorString(0, index).ltrim('#');
				if (colorString.size() >= 6)
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

	bool WinRT_XamlApp::_findVisualElements(chstr nodeName, hstr& data, int& index)
	{
		if (!hfile::exists(MANIFEST_FILENAME))
		{
			return false;
		}
		data = hfile::hread(MANIFEST_FILENAME); // lets hope Microsoft does not change the format of these
		// locating the right entry in XML
		index = data.find("<Applications>");
		if (index < 0)
		{
			return false;
		}
		data = data(index, -1);
		index = data.find("<Application ");
		if (index < 0)
		{
			return false;
		}
		data = data(index, -1);
		index = data.find("<" NODE_PREFIX + nodeName + " ");
		if (index < 0)
		{
			return false;
		}
		data = data(index, -1);
		return true;
	}

}
#endif
