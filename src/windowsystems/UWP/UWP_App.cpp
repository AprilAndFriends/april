/// @file
/// @version 5.2
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifdef _UWP_WINDOW
#include "pch.h"

#include <hltypes/hfile.h>
#include <hltypes/hlog.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hstring.h>
#include <hltypes/hthread.h>

#include "Application.h"
#include "april.h"
#include "Platform.h"
#include "RenderSystem.h"
#include "UpdateDelegate.h"
#include "Window.h"
#include "UWP.h"
#include "UWP_Cursor.h"
#include "UWP_Window.h"
#include "UWP_App.h"

using namespace concurrency;
using namespace Windows::ApplicationModel;
using namespace Windows::Foundation;
using namespace Windows::UI::Core;
using namespace Windows::UI::Input;
using namespace Windows::UI::ViewManagement;

#define UWP_WINDOW ((UWP_Window*)april::window)

namespace april
{
	extern void _updateSystemInfo();

	static inline hstr _CoreWindowActivationStateName(CoreWindowActivationState state)
	{
		if (state == CoreWindowActivationState::CodeActivated)		return "CodeActivated";
		if (state == CoreWindowActivationState::PointerActivated)	return "PointerActivated";
		if (state == CoreWindowActivationState::Deactivated)		return "Deactivated";
		return "Unknown";
	}

	UWP_App::UWP_App() :
		running(true),
		visible(true)
	{
#ifndef _WINP8
		this->defaultCursor = ref new CoreCursor(CoreCursorType::Arrow, 0);
#else
		this->defaultCursor = nullptr;
#endif
		this->scrollHorizontal = false;
		this->startTime = htickCount();
		this->currentButton = Key::None;
	}

	void UWP_App::Initialize(Core::CoreApplicationView^ applicationView)
	{
		Windows::Globalization::ApplicationLanguages::PrimaryLanguageOverride = ref new Platform::String(L"");
		// Register event handlers for app lifecycle. This example includes Activated, so that we
		// can make the CoreWindow active and start rendering on the window.
		applicationView->Activated += ref new TypedEventHandler<CoreApplicationView^, IActivatedEventArgs^>(this, &UWP_App::onActivated);
		CoreApplication::Suspending += ref new EventHandler<SuspendingEventArgs^>(this, &UWP_App::onSuspending);
		CoreApplication::Resuming += ref new EventHandler<Platform::Object^>(this, &UWP_App::onResuming);
	}

	void UWP_App::SetWindow(CoreWindow^ window)
	{
		DisplayInformation::AutoRotationPreferences = (DisplayOrientations::Landscape | DisplayOrientations::LandscapeFlipped);
		window->Activated += ref new TypedEventHandler<CoreWindow^, WindowActivatedEventArgs^>(this, &UWP_App::onWindowFocusChanged);
		window->SizeChanged += ref new TypedEventHandler<CoreWindow^, WindowSizeChangedEventArgs^>(this, &UWP_App::onWindowSizeChanged);
		window->VisibilityChanged += ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(this, &UWP_App::onVisibilityChanged);
		window->Closed += ref new TypedEventHandler<CoreWindow^, CoreWindowEventArgs^>(this, &UWP_App::onWindowClosed);
		window->PointerPressed += ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &UWP_App::onTouchDown);
		window->PointerReleased += ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &UWP_App::onTouchUp);
		window->PointerMoved += ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &UWP_App::onTouchMove);
		window->PointerWheelChanged += ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &UWP_App::onMouseScroll);
		window->KeyDown += ref new TypedEventHandler<CoreWindow^, KeyEventArgs^>(this, &UWP_App::onKeyDown);
		window->KeyUp += ref new TypedEventHandler<CoreWindow^, KeyEventArgs^>(this, &UWP_App::onKeyUp);
		window->CharacterReceived += ref new TypedEventHandler<CoreWindow^, CharacterReceivedEventArgs^>(this, &UWP_App::onCharacterReceived);
		InputPane^ inputPane = InputPane::GetForCurrentView();
		inputPane->Showing += ref new TypedEventHandler<InputPane^, InputPaneVisibilityEventArgs^>(this, &UWP_App::onVirtualKeyboardShow);
		inputPane->Hiding += ref new TypedEventHandler<InputPane^, InputPaneVisibilityEventArgs^>(this, &UWP_App::onVirtualKeyboardHide);
		DisplayInformation^ displayInformation = DisplayInformation::GetForCurrentView();
		displayInformation->DpiChanged += ref new TypedEventHandler<DisplayInformation^, Object^>(this, &UWP_App::onDpiChanged);
		displayInformation->OrientationChanged += ref new TypedEventHandler<DisplayInformation^, Object^>(this, &UWP_App::onOrientationChanged);
		DisplayInformation::DisplayContentsInvalidated += ref new TypedEventHandler<DisplayInformation^, Object^>(this, &UWP_App::onDisplayContentsInvalidated);
		// disable all pointer visual feedback for better performance when touching.
		PointerVisualizationSettings^ pointerVisualizationSettings = PointerVisualizationSettings::GetForCurrentView();
		pointerVisualizationSettings->IsContactFeedbackEnabled = false;
		pointerVisualizationSettings->IsBarrelButtonFeedbackEnabled = false;
		getSystemInfo(); // this call is required to setup the SystemInfo object from the proper thread
		april::application->init();
	}

	void UWP_App::Load(Platform::String^ entryPoint)
	{
	}

	void UWP_App::Run()
	{
		if (april::application != NULL)
		{
			april::application->updateInitializing();
			april::application->enterMainLoop();
		}
		else
		{
			hlog::error(logTag, "Cannot keep running, Application object is NULL! Finishing now...");
		}
	}

	// Required for IFrameworkView.
	// Terminate events do not cause Uninitialize to be called. It will be called if your IFrameworkView
	// class is torn down while the app is in the foreground.
	void UWP_App::Uninitialize()
	{
		// TODOuwp - probably needs changing / moving somewhere else
		if (april::application != NULL)
		{
			april::application->updateFinishing();
			april::application->destroy();
		}
	}

	// Application lifecycle events
	void UWP_App::onActivated(CoreApplicationView^ applicationView, IActivatedEventArgs^ args)
	{
		CoreWindow::GetForCurrentThread()->Activate();
	}

	void UWP_App::onSuspending(Platform::Object^ sender, SuspendingEventArgs^ args)
	{
		hlog::write(logTag, "UWP suspend triggered.");
		// using deferral here, because it's the suggested way to handle a suspend event
		SuspendingDeferral^ deferral = args->SuspendingOperation->GetDeferral();
		create_task([this, deferral]()
		{
			hlog::write(logTag, "UWP suspending...");
			this->_processWindowFocusChange(true);
			if (april::rendersys != NULL)
			{
				april::rendersys->suspend();
			}
			if (april::application != NULL)
			{
				april::application->suspend();
			}
			deferral->Complete();
		});
	}

	void UWP_App::onResuming(Platform::Object^ sender, Platform::Object^ args)
	{
		hlog::write(logTag, "UWP resuming...");
		this->_processWindowFocusChange(true);
		if (april::application != NULL)
		{
			april::application->resume();
		}
	}

	// CoreWindow events
	void UWP_App::onWindowSizeChanged(CoreWindow^ sender, WindowSizeChangedEventArgs^ args)
	{
		args->Handled = true;
		this->_processWindowSizeChange(sender->Bounds.Width, sender->Bounds.Height);
	}

	void UWP_App::onWindowFocusChanged(CoreWindow^ window, WindowActivatedEventArgs^ args)
	{
		hlog::write(logTag, "UWP activation change: " + _CoreWindowActivationStateName(args->WindowActivationState));
		args->Handled = true;
		this->_processWindowFocusChange(args->WindowActivationState != CoreWindowActivationState::Deactivated);
	}

	void UWP_App::onVisibilityChanged(CoreWindow^ sender, VisibilityChangedEventArgs^ args)
	{
		hlog::write(logTag, "UWP visibility change: " + hstr(args->Visible ? "true" : "false"));
		args->Handled = true;
		// TODOuwp - likely not needed
		/*
		if (this->visible != args->Visible)
		{
			this->visible = args->Visible;
			this->_processWindowFocusChange(args->Visible);
		}
		*/
		this->_resetTouches();
	}

	void UWP_App::onWindowClosed(CoreWindow^ sender, CoreWindowEventArgs^ args)
	{
		args->Handled = true;
		if (april::window != NULL)
		{
			april::window->queueQuitRequest(false);
		}
	}

	// DisplayInformation events
	void UWP_App::onDpiChanged(DisplayInformation^ sender, Platform::Object^ args)
	{
		CoreWindow^ window = CoreWindow::GetForCurrentThread();
		this->_processWindowSizeChange(window->Bounds.Width, window->Bounds.Height);
	}

	void UWP_App::onOrientationChanged(DisplayInformation^ sender, Platform::Object^ args)
	{
		CoreWindow^ window = CoreWindow::GetForCurrentThread();
		this->_processWindowSizeChange(window->Bounds.Width, window->Bounds.Height);
	}

	void UWP_App::onDisplayContentsInvalidated(DisplayInformation^ sender, Platform::Object^ args)
	{
		// TODOuwp - probably needs implementing
		//GetDeviceResources()->ValidateDevice();
	}

	void UWP_App::_processWindowSizeChange(float width, float height)
	{
		_updateSystemInfo(); // internal platform method to update system information from the proper thread
		this->_resetTouches();
		if (april::window != NULL)
		{
			int windowWidth = (int)width;
			int windowHeight = (int)height;
			bool fullscreen = ApplicationView::GetForCurrentView()->IsFullScreenMode;
			UWP_WINDOW->_systemSetResolution(windowWidth, windowHeight, fullscreen);
			april::window->queueSizeChange(april::window->getWidth(), april::window->getHeight(), april::window->isFullscreen());
		}
	}

	void UWP_App::_processWindowFocusChange(bool focused)
	{
		this->_resetTouches();
		if (april::window != NULL)
		{
			april::window->queueFocusChange(focused);
		}
	}

	void UWP_App::_resetTouches()
	{
		for_iter_r (i, this->pointerIds.size(), 0)
		{
			april::window->queueTouchInput(MouseEvent::Type::Cancel, gvec2f(), i);
		}
		this->pointerIds.clear();
	}

	void UWP_App::refreshCursor()
	{
		// TODOuwp - should be queued into main thread
		if (april::window != NULL)
		{
			CoreCursor^ cursor = nullptr;
			if (april::window->isCursorVisible())
			{
				Cursor* windowCursor = april::window->getCursor();
				if (windowCursor != NULL)
				{
					cursor = ((UWP_Cursor*)windowCursor)->getCursor();
				}
				if (cursor == nullptr)
				{
					cursor = this->defaultCursor;
				}
			}
			// TODOuwp - implement this, can't be called from secondary thread
			//CoreWindow::GetForCurrentThread()->PointerCursor = cursor;
		}
	}

	void UWP_App::onVirtualKeyboardShow(_In_ InputPane^ sender, _In_ InputPaneVisibilityEventArgs^ args)
	{
		if (april::window != NULL)
		{
			april::window->queueVirtualKeyboardChange(true, args->OccludedRect.Height / CoreWindow::GetForCurrentThread()->Bounds.Height);
		}
		this->_resetTouches();
	}

	void UWP_App::onVirtualKeyboardHide(_In_ InputPane^ sender, _In_ InputPaneVisibilityEventArgs^ args)
	{
		if (april::window != NULL)
		{
			april::window->queueVirtualKeyboardChange(false, 0.0f);
		}
		this->_resetTouches();
	}

	void UWP_App::onTouchDown(_In_ CoreWindow^ sender, _In_ PointerEventArgs^ args)
	{
		args->Handled = true;
		if (april::window == NULL || !april::window->isFocused())
		{
			return;
		}
		unsigned int id;
		int index;
		gvec2f position = this->_transformPosition(args->CurrentPoint->Position.X, args->CurrentPoint->Position.Y);
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
			april::window->queueMouseInput(MouseEvent::Type::Down, position, this->currentButton);
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
			april::window->queueTouchInput(MouseEvent::Type::Down, position, index);
#ifndef _WINP8
			break;
		}
#endif
	}

	void UWP_App::onTouchUp(_In_ CoreWindow^ sender, _In_ PointerEventArgs^ args)
	{
		args->Handled = true;
		if (april::window == NULL || !april::window->isFocused())
		{
			return;
		}
		unsigned int id;
		int index;
		gvec2f position = this->_transformPosition(args->CurrentPoint->Position.X, args->CurrentPoint->Position.Y);
#ifndef _WINP8
		switch (args->CurrentPoint->PointerDevice->PointerDeviceType)
		{
		case Windows::Devices::Input::PointerDeviceType::Mouse:
			april::window->setInputMode(InputMode::Mouse);
			april::window->queueMouseInput(MouseEvent::Type::Up, position, this->currentButton);
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
			april::window->queueTouchInput(MouseEvent::Type::Up, position, index);
#ifndef _WINP8
			break;
		}
		this->currentButton = Key::None;
#endif
	}

	void UWP_App::onTouchMove(_In_ CoreWindow^ sender, _In_ PointerEventArgs^ args)
	{
		args->Handled = true;
		if (april::window == NULL || !april::window->isFocused())
		{
			return;
		}
		unsigned int id;
		int index;
		gvec2f position = this->_transformPosition(args->CurrentPoint->Position.X, args->CurrentPoint->Position.Y);
#ifndef _WINP8
		switch (args->CurrentPoint->PointerDevice->PointerDeviceType)
		{
		case Windows::Devices::Input::PointerDeviceType::Mouse:
			april::window->setInputMode(InputMode::Mouse);
			april::window->queueMouseInput(MouseEvent::Type::Move, position, this->currentButton);
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
			april::window->queueTouchInput(MouseEvent::Type::Move, position, index);
#ifndef _WINP8
			break;
		}
#endif
	}

	void UWP_App::onMouseScroll(_In_ CoreWindow^ sender, _In_ PointerEventArgs^ args)
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
			april::window->queueMouseInput(MouseEvent::Type::Scroll, gvec2f(-(float)_wheelDelta, 0.0f), Key::None);
		}
		else
		{
			april::window->queueMouseInput(MouseEvent::Type::Scroll, gvec2f(0.0f, -(float)_wheelDelta), Key::None);
		}
	}

	void UWP_App::onKeyDown(_In_ CoreWindow^ sender, _In_ KeyEventArgs^ args)
	{
		args->Handled = true;
		if (april::window == NULL || !april::window->isFocused())
		{
			return;
		}
		Key key = Key::fromInt((int)args->VirtualKey);
		april::window->queueKeyInput(KeyEvent::Type::Down, key, 0);
		if (key == Key::Control || key == Key::ControlL || key == Key::ControlR)
		{
			this->scrollHorizontal = true;
		}
	}

	void UWP_App::onKeyUp(_In_ CoreWindow^ sender, _In_ KeyEventArgs^ args)
	{
		args->Handled = true;
		if (april::window == NULL || !april::window->isFocused())
		{
			return;
		}
		Key key = Key::fromInt((int)args->VirtualKey);
		april::window->queueKeyInput(KeyEvent::Type::Up, key, 0);
		if (key == Key::Control || key == Key::ControlL || key == Key::ControlR)
		{
			this->scrollHorizontal = false;
		}
		else if (key == Key::Return)
		{
			april::window->hideVirtualKeyboard();
		}
	}

	void UWP_App::onCharacterReceived(_In_ CoreWindow^ sender, _In_ CharacterReceivedEventArgs^ args)
	{
		args->Handled = true;
		if (april::window == NULL || !april::window->isFocused())
		{
			return;
		}
		april::window->queueKeyInput(KeyEvent::Type::Down, Key::None, args->KeyCode);
	}

	gvec2f UWP_App::_transformPosition(float x, float y)
	{
		// UWP is dumb
		return (gvec2f(x, y) * UWP::getDpiRatio());
	}

}
#endif
