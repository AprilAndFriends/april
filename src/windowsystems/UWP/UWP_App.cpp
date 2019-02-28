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
using namespace Windows::System;
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
		this->defaultCursor = ref new CoreCursor(CoreCursorType::Arrow, 0);
		this->scrollHorizontal = false;
		this->startTime = htickCount();
		this->currentButton = Key::None;
		this->windowTitleRequested = nullptr;
		this->virtualKeyboardCurrentState = false;
		this->virtualKeyboardRequestState = false;
		this->refreshCursorRequested = false;
	}

	void UWP_App::Initialize(Core::CoreApplicationView^ applicationView)
	{
		Windows::Globalization::ApplicationLanguages::PrimaryLanguageOverride = ref new Platform::String(L"");
		applicationView->Activated += ref new TypedEventHandler<CoreApplicationView^, IActivatedEventArgs^>(this, &UWP_App::onActivated);
		CoreApplication::Suspending += ref new EventHandler<SuspendingEventArgs^>(this, &UWP_App::onSuspending);
		CoreApplication::Resuming += ref new EventHandler<Platform::Object^>(this, &UWP_App::onResuming);
		CoreApplication::UnhandledErrorDetected += ref new EventHandler<UnhandledErrorDetectedEventArgs ^>(this, &UWP_App::onUnhandledErrorDetected);
	}

	void UWP_App::SetWindow(CoreWindow^ window)
	{
		// TODOuwp - implement a workaround for when portrait mode is used
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
		window->Dispatcher->AcceleratorKeyActivated += ref new TypedEventHandler<CoreDispatcher^, AcceleratorKeyEventArgs^>(this, &UWP_App::onAcceleratorKeyActivated);
		InputPane^ inputPane = InputPane::GetForCurrentView();
		inputPane->Showing += ref new TypedEventHandler<InputPane^, InputPaneVisibilityEventArgs^>(this, &UWP_App::onVirtualKeyboardShow);
		inputPane->Hiding += ref new TypedEventHandler<InputPane^, InputPaneVisibilityEventArgs^>(this, &UWP_App::onVirtualKeyboardHide);
		DisplayInformation^ displayInformation = DisplayInformation::GetForCurrentView();
		displayInformation->DpiChanged += ref new TypedEventHandler<DisplayInformation^, Object^>(this, &UWP_App::onDpiChanged);
		displayInformation->OrientationChanged += ref new TypedEventHandler<DisplayInformation^, Object^>(this, &UWP_App::onOrientationChanged);
		// disable all pointer visual feedback for better performance when touching.
		PointerVisualizationSettings^ pointerVisualizationSettings = PointerVisualizationSettings::GetForCurrentView();
		pointerVisualizationSettings->IsContactFeedbackEnabled = false;
		pointerVisualizationSettings->IsBarrelButtonFeedbackEnabled = false;
		april::getSystemInfo(); // this call is required to setup the SystemInfo object from the proper thread
		april::application->init();
		april::application->updateInitializing();
	}

	void UWP_App::Load(Platform::String^ entryPoint)
	{
	}

	void UWP_App::Run()
	{
		if (april::application != NULL)
		{
			if (april::window != NULL && april::rendersys != NULL)
			{
				april::application->enterMainLoop();
			}
			april::application->destroy();
		}
		else
		{
			hlog::error(logTag, "Cannot keep running, Application object is NULL! Finishing now...");
		}
	}

	// terminate events do not cause Uninitialize to be called, it will be called if the IFrameworkView class is torn down while the app is in the foreground
	void UWP_App::Uninitialize()
	{
		// TODOuwp - probably needs changing / moving somewhere else
		if (april::application != NULL)
		{
			april::application->finish();
			april::application->finalize();
			april::application->updateFinishing();
			april::application->destroy();
		}
	}

	void UWP_App::updateMainThread()
	{
		if (this->windowTitleRequested != nullptr)
		{
			ApplicationView::GetForCurrentView()->Title = this->windowTitleRequested;
			this->windowTitleRequested = nullptr;
		}
		if (this->refreshCursorRequested)
		{
			if (april::window != NULL)
			{
				// do not change this code due to threading synchronization!
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
				CoreWindow::GetForCurrentThread()->PointerCursor = cursor;
			}
			this->refreshCursorRequested = false;
		}
		if (this->virtualKeyboardCurrentState != this->virtualKeyboardRequestState)
		{
			this->virtualKeyboardCurrentState = this->virtualKeyboardRequestState;
			if (this->virtualKeyboardCurrentState)
			{
				InputPane::GetForCurrentView()->TryShow();
			}
			else
			{
				InputPane::GetForCurrentView()->TryHide();
			}
		}
	}

	void UWP_App::setWindowTitle(Platform::String^ title)
	{
		this->windowTitleRequested = title;
	}

	void UWP_App::refreshCursor()
	{
		this->refreshCursorRequested = true;
	}

	void UWP_App::showVirtualKeyboard()
	{
		this->virtualKeyboardRequestState = true;
	}

	void UWP_App::hideVirtualKeyboard()
	{
		this->virtualKeyboardRequestState = false;
	}

	// Application lifecycle events
	void UWP_App::onUnhandledErrorDetected(Platform::Object^ sender, UnhandledErrorDetectedEventArgs^ args)
	{
		hlog::error("FATAL", _HL_PSTR_TO_HSTR(args->UnhandledError->ToString()));
		args->UnhandledError->Propagate();
	}

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
		this->visible = args->Visible;
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
			april::window->queueTouchInput(TouchEvent::Type::Cancel, i, gvec2f());
		}
		this->pointerIds.clear();
	}

	void UWP_App::onVirtualKeyboardShow(_In_ InputPane^ sender, _In_ InputPaneVisibilityEventArgs^ args)
	{
		if (april::window != NULL)
		{
			this->virtualKeyboardCurrentState = true;
			this->virtualKeyboardRequestState = true;
			april::window->queueVirtualKeyboardChange(true, args->OccludedRect.Height / CoreWindow::GetForCurrentThread()->Bounds.Height);
		}
		this->_resetTouches();
	}

	void UWP_App::onVirtualKeyboardHide(_In_ InputPane^ sender, _In_ InputPaneVisibilityEventArgs^ args)
	{
		if (april::window != NULL)
		{
			this->virtualKeyboardCurrentState = false;
			this->virtualKeyboardRequestState = false;
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
		this->currentButton = Key::MouseL;
		switch (args->CurrentPoint->PointerDevice->PointerDeviceType)
		{
		case Windows::Devices::Input::PointerDeviceType::Mouse:
			april::window->queueInputModeChange(InputMode::Mouse);
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
			april::window->queueInputModeChange(InputMode::Touch);
			id = args->CurrentPoint->PointerId;
			index = this->pointerIds.indexOf(id);
			if (index < 0)
			{
				index = this->pointerIds.size();
				this->pointerIds += id;
			}
			april::window->queueTouchInput(TouchEvent::Type::Down, index, position);
			break;
		}
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
		switch (args->CurrentPoint->PointerDevice->PointerDeviceType)
		{
		case Windows::Devices::Input::PointerDeviceType::Mouse:
			april::window->queueInputModeChange(InputMode::Mouse);
			april::window->queueMouseInput(MouseEvent::Type::Up, position, this->currentButton);
			break;
		case Windows::Devices::Input::PointerDeviceType::Touch:
		case Windows::Devices::Input::PointerDeviceType::Pen:
			april::window->queueInputModeChange(InputMode::Touch);
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
			april::window->queueTouchInput(TouchEvent::Type::Up, index, position);
			break;
		}
		this->currentButton = Key::None;
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
		switch (args->CurrentPoint->PointerDevice->PointerDeviceType)
		{
		case Windows::Devices::Input::PointerDeviceType::Mouse:
			april::window->queueInputModeChange(InputMode::Mouse);
			april::window->queueMouseInput(MouseEvent::Type::Move, position, this->currentButton);
			break;
		case Windows::Devices::Input::PointerDeviceType::Touch:
		case Windows::Devices::Input::PointerDeviceType::Pen:
			april::window->queueInputModeChange(InputMode::Touch);
			id = args->CurrentPoint->PointerId;
			index = this->pointerIds.indexOf(id);
			if (index < 0)
			{
				index = this->pointerIds.size();
			}
			april::window->queueTouchInput(TouchEvent::Type::Move, index, position);
			break;
		}
	}

	void UWP_App::onMouseScroll(_In_ CoreWindow^ sender, _In_ PointerEventArgs^ args)
	{
		args->Handled = true;
		if (april::window == NULL || !april::window->isFocused())
		{
			return;
		}
		april::window->queueInputModeChange(InputMode::Mouse);
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

	void UWP_App::onAcceleratorKeyActivated(_In_ CoreDispatcher^ sender, _In_ AcceleratorKeyEventArgs^ args)
	{
		if (april::window != NULL && args->EventType == CoreAcceleratorKeyEventType::SystemKeyDown &&
			args->VirtualKey == VirtualKey::Enter && args->KeyStatus.IsMenuKeyDown && !args->KeyStatus.WasKeyDown)
		{
			args->Handled = true;
			UWP_WINDOW->_systemToggleHotkeyFullscreen(); // this comes from the main thread so it can directly call the needed function
		}
	}

	gvec2f UWP_App::_transformPosition(float x, float y)
	{
		return (gvec2f(x, y) * UWP::getDpiRatio());
	}

}
#endif
