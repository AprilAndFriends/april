/// @file
/// @author  Boris Mikic
/// @version 3.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _WINRT_WINDOW
#include "pch.h"

#include <hltypes/hltypesUtil.h>
#include <hltypes/hresource.h>
#include <hltypes/hstring.h>

#include "RenderSystem.h"
#include "Window.h"
#include "WinRT_View.h"
#include "WinRT_XamlApp.h"

#ifdef _DEBUG
extern "C" __declspec(dllimport) int __stdcall IsDebuggerPresent();
#endif

//using namespace Concurrency;
//using namespace Platform;
using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::Foundation;
//using namespace Windows::Foundation::Collections;
//using namespace Windows::UI::Core;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
//using namespace Windows::UI::Xaml::Data;
//using namespace Windows::UI::Xaml::Input;
//using namespace Windows::UI::Xaml::Interop;
using namespace Windows::UI::Xaml::Media;
//using namespace Windows::UI::Xaml::Navigation;

#include "Platform.h"

namespace april
{
	WinRT_XamlApp::WinRT_XamlApp() : Application()
	{
		this->running = true;
		this->scrollHorizontal = false;
		//this->filled = false;
		//this->snapped = false;
		this->mouseMoveMessagesCount = 0;
		this->Suspending += ref new SuspendingEventHandler(this, &WinRT_XamlApp::OnSuspend);
		this->Resuming += ref new EventHandler<Object^>(this, &WinRT_XamlApp::OnResume);
#ifdef _DEBUG
		this->UnhandledException += ref new UnhandledExceptionEventHandler(
			[](Object^ sender, UnhandledExceptionEventArgs^ e)
			{
				if (IsDebuggerPresent())
				{
					Platform::String^ errorMessage = e->Message;
					__debugbreak();
				}
			}
		);
#endif
	}

	WinRT_XamlApp::~WinRT_XamlApp()
	{
		CompositionTarget::Rendering::remove(this->renderEventToken);
	}

	void WinRT_XamlApp::setCursorVisible(bool value)
	{
		Windows::UI::Xaml::Window::Current->CoreWindow->PointerCursor = (value ? ref new CoreCursor(CoreCursorType::Arrow, 0) : nullptr);
	}
	
	void WinRT_XamlApp::Connect(int connectionId, Object^ target)
	{
	}

	void WinRT_XamlApp::OnLaunched(LaunchActivatedEventArgs^ args)
	{
		// don't repeat app initialization when already running
		if (args->PreviousExecutionState == ApplicationExecutionState::Running)
		{
			Windows::UI::Xaml::Window::Current->Activate();
			return;
		}
		WinRT::Interface = ref new WinRT_XamlInterface();
		Windows::UI::Xaml::Window::Current->Content = WinRT::Interface;
		Windows::UI::Xaml::Window::Current->Activate();
		// events
		CoreWindow^ window = Windows::UI::Xaml::Window::Current->CoreWindow;
		window->SizeChanged +=
			ref new TypedEventHandler<CoreWindow^, WindowSizeChangedEventArgs^>(
				this, &WinRT_XamlApp::OnWindowSizeChanged);
		window->VisibilityChanged +=
			ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(
				this, &WinRT_XamlApp::OnVisibilityChanged);
		window->Closed +=
			ref new TypedEventHandler<CoreWindow^, CoreWindowEventArgs^>(
				this, &WinRT_XamlApp::OnWindowClosed);
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
		this->setCursorVisible(true);


		
		//WinRT::Keyboard = (TextBox^)XamlReader::Load(APRIL_WINRT_XAML);


		/*
		this->scrollHorizontal = false;
		this->filled = false;
		this->snapped = false;
		this->mouseMoveMessagesCount = 0;
		*/
		hresource::setCwd(normalize_path(hstr::from_unicode(Package::Current->InstalledLocation->Path->Data())));
		hresource::setArchive("");
		(*WinRT::Init)(WinRT::Args);
		this->renderEventToken = CompositionTarget::Rendering::add(ref new EventHandler<Object^>(this, &WinRT_XamlApp::OnRender));
	}

	void WinRT_XamlApp::OnSuspend(_In_ Object^ sender, _In_ SuspendingEventArgs^ args)
	{
		april::window->handleFocusChangeEvent(false);
	}

	void WinRT_XamlApp::OnResume(_In_ Object^ sender, _In_ Object^ args)
	{
		april::window->handleFocusChangeEvent(true);
	}

	void WinRT_XamlApp::OnRender(_In_ Object^ sender, _In_ Object^ args)
	{
		if (!this->running)
		{
			return;
		}
		this->running = april::window->updateOneFrame();
		april::rendersys->presentFrame();
		if (!this->running)
		{
			if (this->renderEventToken.Value != 0)
			{
				CompositionTarget::Rendering::remove(this->renderEventToken);
				this->renderEventToken.Value = 0;
			}
			(*WinRT::Destroy)();
		}
	}

	void WinRT_XamlApp::OnWindowSizeChanged(_In_ CoreWindow^ sender, _In_ WindowSizeChangedEventArgs^ args)
	{
		args->Handled = true;
	}
	
	void WinRT_XamlApp::OnVisibilityChanged(_In_ CoreWindow^ sender, _In_ VisibilityChangedEventArgs^ args)
	{
		args->Handled = true;
	}
	
	void WinRT_XamlApp::OnWindowClosed(_In_ CoreWindow^ sender, _In_ CoreWindowEventArgs^ args)
	{
		april::window->handleQuitRequest(false);
		args->Handled = true;
	}
	
	void WinRT_XamlApp::OnTouchDown(_In_ CoreWindow^ sender, _In_ PointerEventArgs^ args)
	{
		unsigned int id;
		int index;
		gvec2 position(args->CurrentPoint->Position.X, args->CurrentPoint->Position.Y);
		switch (args->CurrentPoint->PointerDevice->PointerDeviceType)
		{
		case Windows::Devices::Input::PointerDeviceType::Mouse:
			april::window->setInputMode(april::Window::MOUSE);
			april::window->queueMouseEvent(april::Window::AMOUSEEVT_DOWN, position, april::AK_LBUTTON);
			break;
		case Windows::Devices::Input::PointerDeviceType::Touch:
		case Windows::Devices::Input::PointerDeviceType::Pen:
			this->mouseMoveMessagesCount = 0;
			april::window->setInputMode(april::Window::TOUCH);
			id = args->CurrentPoint->PointerId;
			index = this->pointerIds.index_of(id);
			if (index < 0)
			{
				index = this->pointerIds.size();
				this->pointerIds += id;
			}
			april::window->queueTouchEvent(april::Window::AMOUSEEVT_DOWN, position, index);
			break;
		}
		args->Handled = true;
	}
	
	void WinRT_XamlApp::OnTouchUp(_In_ CoreWindow^ sender, _In_ PointerEventArgs^ args)
	{
		unsigned int id;
		int index;
		gvec2 position(args->CurrentPoint->Position.X, args->CurrentPoint->Position.Y);
		switch (args->CurrentPoint->PointerDevice->PointerDeviceType)
		{
		case Windows::Devices::Input::PointerDeviceType::Mouse:
			april::window->setInputMode(april::Window::MOUSE);
			april::window->queueMouseEvent(april::Window::AMOUSEEVT_UP, position, april::AK_LBUTTON);
			break;
		case Windows::Devices::Input::PointerDeviceType::Touch:
		case Windows::Devices::Input::PointerDeviceType::Pen:
			this->mouseMoveMessagesCount = 0;
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
			april::window->queueTouchEvent(april::Window::AMOUSEEVT_UP, position, index);
			break;
		}
		args->Handled = true;
	}
	
	void WinRT_XamlApp::OnTouchMove(_In_ CoreWindow^ sender, _In_ PointerEventArgs^ args)
	{
		unsigned int id;
		int index;
		gvec2 position(args->CurrentPoint->Position.X, args->CurrentPoint->Position.Y);
		switch (args->CurrentPoint->PointerDevice->PointerDeviceType)
		{
		case Windows::Devices::Input::PointerDeviceType::Mouse:
			this->mouseMoveMessagesCount++;
			if (this->mouseMoveMessagesCount >= 10)
			{
				april::window->setInputMode(april::Window::MOUSE);
			}
			april::window->queueMouseEvent(april::Window::AMOUSEEVT_MOVE, position, april::AK_LBUTTON);
			break;
		case Windows::Devices::Input::PointerDeviceType::Touch:
		case Windows::Devices::Input::PointerDeviceType::Pen:
			this->mouseMoveMessagesCount = 0;
			april::window->setInputMode(april::Window::TOUCH);
			id = args->CurrentPoint->PointerId;
			index = this->pointerIds.index_of(id);
			if (index < 0)
			{
				index = this->pointerIds.size();
			}
			april::window->queueTouchEvent(april::Window::AMOUSEEVT_MOVE, position, index);
			break;
		}
		args->Handled = true;
	}
	
	void WinRT_XamlApp::OnMouseScroll(_In_ CoreWindow^ sender, _In_ PointerEventArgs^ args)
	{
		april::window->setInputMode(april::Window::MOUSE);
		float _wheelDelta = (float)args->CurrentPoint->Properties->MouseWheelDelta / WHEEL_DELTA;
		if (this->scrollHorizontal ^ args->CurrentPoint->Properties->IsHorizontalMouseWheel)
		{
			april::window->queueMouseEvent(april::Window::AMOUSEEVT_SCROLL,
				gvec2(-(float)_wheelDelta, 0.0f), april::AK_NONE);
		}
		else
		{
			april::window->queueMouseEvent(april::Window::AMOUSEEVT_SCROLL,
				gvec2(0.0f, -(float)_wheelDelta), april::AK_NONE);
		}
		args->Handled = true;
	}
	
	void WinRT_XamlApp::OnKeyDown(_In_ CoreWindow^ sender, _In_ KeyEventArgs^ args)
	{
		april::Key key = (april::Key)args->VirtualKey;
		april::window->queueKeyEvent(april::Window::AKEYEVT_DOWN, key, 0);
		if (key == AK_CONTROL || key == AK_LCONTROL || key == AK_RCONTROL)
		{
			this->scrollHorizontal = true;
		}
		args->Handled = true;
	}
	
	void WinRT_XamlApp::OnKeyUp(_In_ CoreWindow^ sender, _In_ KeyEventArgs^ args)
	{
		april::Key key = (april::Key)args->VirtualKey;
		april::window->queueKeyEvent(april::Window::AKEYEVT_UP, key, 0);
		if (key == AK_CONTROL || key == AK_LCONTROL || key == AK_RCONTROL)
		{
			this->scrollHorizontal = false;
		}
		args->Handled = true;
	}
	
	void WinRT_XamlApp::OnCharacterReceived(_In_ CoreWindow^ sender, _In_ CharacterReceivedEventArgs^ args)
	{
		april::window->queueKeyEvent(april::Window::AKEYEVT_DOWN, AK_NONE, args->KeyCode);
		args->Handled = true;
	}
	
		/*
	void WinRT_XamlApp::OnWindowActivationChanged(
		_In_ Object^ sender,
		_In_ Windows::UI::Core::WindowActivatedEventArgs^ args
		)
	{
		if (args->WindowActivationState == CoreWindowActivationState::Deactivated)
		{
			m_haveFocus = false;

			switch (m_updateState)
			{
			case UpdateEngineState::Dynamics:
				// From Dynamic mode, when coming out of Deactivated rather than going directly back into game play
				// go to the paused state waiting for user input to continue
				m_updateStateNext = UpdateEngineState::WaitingForPress;
				m_pressResult = PressResultState::ContinueLevel;
				SetGameInfoOverlay(GameInfoOverlayState::Pause);
				ShowGameInfoOverlay();
				m_game->PauseGame();
				m_updateState = UpdateEngineState::Deactivated;
				SetAction(GameInfoOverlayCommand::None);
				m_renderNeeded = true;
				break;

			case UpdateEngineState::WaitingForResources:
			case UpdateEngineState::WaitingForPress:
				m_updateStateNext = m_updateState;
				m_updateState = UpdateEngineState::Deactivated;
				SetAction(GameInfoOverlayCommand::None);
				ShowGameInfoOverlay();
				m_renderNeeded = true;
				break;
			}
			// Don't have focus so shutdown input processing
			m_controller->Active(false);
		}
		else if (args->WindowActivationState == CoreWindowActivationState::CodeActivated
			|| args->WindowActivationState == CoreWindowActivationState::PointerActivated)
		{
			m_haveFocus = true;

			if (m_updateState == UpdateEngineState::Deactivated)
			{
				m_updateState = m_updateStateNext;

				if (m_updateState == UpdateEngineState::WaitingForPress)
				{
					SetAction(GameInfoOverlayCommand::TapToContinue);
					m_controller->WaitForPress();
				}
				else if (m_updateStateNext == UpdateEngineState::WaitingForResources)
				{
					SetAction(GameInfoOverlayCommand::PleaseWait);
				}

				// App is now "active" so set up the event handler to do game processing and rendering
				if (this->renderEventToken.Value == 0)
				{
					this->renderEventToken = CompositionTarget::Rendering::add(ref new EventHandler<Object^>(this, &WinRT_XamlApp::OnRendering));
				}
			}
		}
	}

	//--------------------------------------------------------------------------------------

	void WinRT_XamlApp::UpdateViewState()
	{
		if (ApplicationView::Value == ApplicationViewState::Snapped)
		{
			switch (m_updateState)
			{
			case UpdateEngineState::Dynamics:
				// From Dynamic mode, when coming out of SNAPPED layout rather than going directly back into game play
				// go to the paused state waiting for user input to continue
				m_updateStateNext = UpdateEngineState::WaitingForPress;
				m_pressResult = PressResultState::ContinueLevel;
				SetGameInfoOverlay(GameInfoOverlayState::Pause);
				SetAction(GameInfoOverlayCommand::TapToContinue);
				m_game->PauseGame();
				break;

			case UpdateEngineState::WaitingForResources:
			case UpdateEngineState::WaitingForPress:
				// Avoid corrupting the m_updateStateNext on a transition from Snapped -> Snapped.
				// Otherwise just cache the current state and return to it when leaving SNAPPED layout

				m_updateStateNext = m_updateState;
				break;

			default:
				break;
			}

			m_updateState = UpdateEngineState::Snapped;
			m_controller->Active(false);
			HideGameInfoOverlay();
			SetSnapped();
			m_renderNeeded = true;
		}
		else if (ApplicationView::Value == ApplicationViewState::Filled ||
			ApplicationView::Value == ApplicationViewState::FullScreenLandscape ||
			ApplicationView::Value == ApplicationViewState::FullScreenPortrait)
		{
			if (m_updateState == UpdateEngineState::Snapped)
			{
				HideSnapped();
				ShowGameInfoOverlay();
				m_renderNeeded = true;

				if (m_haveFocus)
				{
					if (m_updateStateNext == UpdateEngineState::WaitingForPress)
					{
						SetAction(GameInfoOverlayCommand::TapToContinue);
						m_controller->WaitForPress();
					}
					else if (m_updateStateNext == UpdateEngineState::WaitingForResources)
					{
						SetAction(GameInfoOverlayCommand::PleaseWait);
					}

					m_updateState = m_updateStateNext;
				}
				else
				{
					m_updateState = UpdateEngineState::Deactivated;
					SetAction(GameInfoOverlayCommand::None);
				}
			}
			else if (m_updateState == UpdateEngineState::Dynamics)
			{
				// In Dynamic mode, when a resize event happens, go to the paused state waiting for user input to continue.
				m_pressResult = PressResultState::ContinueLevel;
				SetGameInfoOverlay(GameInfoOverlayState::Pause);
				m_game->PauseGame();
				if (m_haveFocus)
				{
					m_updateState = UpdateEngineState::WaitingForPress;
					SetAction(GameInfoOverlayCommand::TapToContinue);
					m_controller->WaitForPress();
				}
				else
				{
					m_updateState = UpdateEngineState::Deactivated;
					SetAction(GameInfoOverlayCommand::None);
				}
				ShowGameInfoOverlay();
				m_renderNeeded = true;
			}

			if (m_haveFocus && this->renderEventToken.Value == 0)
			{
				// App is now "active" so set up the event handler to do game processing and rendering
				this->renderEventToken = CompositionTarget::Rendering::add(ref new EventHandler<Object^>(this, &WinRT_XamlApp::OnRendering));
			}
		}
	}


	*/
}
#endif
