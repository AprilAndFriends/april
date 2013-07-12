/// @file
/// @author  Boris Mikic
/// @version 3.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _WINRT_WINDOW

#include <hltypes/hlog.h>
#include <hltypes/hplatform.h>
#include <hltypes/hresource.h>
#include <hltypes/hstring.h>

#include "april.h"
#include "Platform.h"
#include "Window.h"
#include "WinRT_View.h"
#include "WinRT_Window.h"

using namespace Windows::ApplicationModel;
using namespace Windows::Foundation;
using namespace Windows::UI::ViewManagement;

namespace april
{
	void (*WinRT::Init)(const harray<hstr>&) = NULL;
	void (*WinRT::Destroy)() = NULL;
	harray<hstr> WinRT::Args;
	WinRT_View^ WinRT::View = nullptr;
	
	void WinRT_View::Initialize(_In_ CoreApplicationView^ applicationView)
	{
		applicationView->Activated +=
			ref new TypedEventHandler<CoreApplicationView^, IActivatedEventArgs^>(this, &WinRT_View::OnActivated);
	}
	
	void WinRT_View::Uninitialize()
	{
	}
	
	void WinRT_View::SetWindow(_In_ CoreWindow^ window)
	{
		this->window = window;
		this->window->SizeChanged +=
			ref new TypedEventHandler<CoreWindow^, WindowSizeChangedEventArgs^>(
				this, &WinRT_View::OnWindowSizeChanged);
		CoreApplication::Suspending +=
			ref new EventHandler<SuspendingEventArgs^>(this, &WinRT_View::OnSuspend);
		CoreApplication::Resuming +=
			ref new EventHandler<Platform::Object^>(this, &WinRT_View::OnResume);
		this->window->PointerPressed +=
			ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(
				this, &WinRT_View::OnTouchDown);
		this->window->PointerReleased +=
			ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(
				this, &WinRT_View::OnTouchUp);
		this->window->PointerMoved +=
			ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(
				this, &WinRT_View::OnTouchMove);
		this->window->PointerWheelChanged +=
			ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(
				this, &WinRT_View::OnMouseScroll);
		this->window->KeyDown +=
			ref new TypedEventHandler<CoreWindow^, KeyEventArgs^>(
				this, &WinRT_View::OnKeyDown);
		this->window->KeyUp +=
			ref new TypedEventHandler<CoreWindow^, KeyEventArgs^>(
				this, &WinRT_View::OnKeyUp);
		this->window->CharacterReceived +=
			ref new TypedEventHandler<CoreWindow^, CharacterReceivedEventArgs^>(
				this, &WinRT_View::OnCharacterReceived);
		this->window->Closed +=
			ref new TypedEventHandler<CoreWindow^, CoreWindowEventArgs^>(
				this, &WinRT_View::OnWindowClosed);
		this->setCursorVisible(true);
	}
	
	void WinRT_View::Load(_In_ Platform::String^ entryPoint)
	{
		this->filled = false;
		this->snapped = false;
	}
	
	void WinRT_View::Run()
	{
		this->scrollHorizontal = false;
		this->filled = false;
		this->snapped = false;
		this->mouseMoveMessagesCount = 0;
		hresource::setCwd(normalize_path(hstr::from_unicode(Package::Current->InstalledLocation->Path->Data())));
		hresource::setArchive("");
		WinRT::View = this;
		(*WinRT::Init)(WinRT::Args);
		april::window->enterMainLoop();
		(*WinRT::Destroy)();
		WinRT::View = nullptr;
	}
	
	void WinRT_View::setCursorVisible(bool value)
	{
		this->window->PointerCursor = (value ? ref new CoreCursor(CoreCursorType::Arrow, 0) : nullptr);
	}
	
	void WinRT_View::OnActivated(_In_ CoreApplicationView^ applicationView, _In_ IActivatedEventArgs^ args)
	{
		CoreWindow::GetForCurrentThread()->Activate();
		this->filled = false;
		this->snapped = false;
	}
	
	void WinRT_View::_updateViewState()
	{
		bool newFilled = (ApplicationView::Value == ApplicationViewState::Filled);
		if (!this->filled && newFilled)
		{
			hlog::write(april::logTag, "Handling filled view override...");
		}
		this->filled = newFilled;
		bool newSnapped = (ApplicationView::Value == ApplicationViewState::Snapped);
		if (!this->snapped && newSnapped)
		{
			hlog::write(april::logTag, "Handling snapped view override...");
		}
		this->snapped = newSnapped;
	}
	
	void WinRT_View::OnWindowSizeChanged(_In_ CoreWindow^ sender, _In_ WindowSizeChangedEventArgs^ args)
	{
		this->_updateViewState();
		args->Handled = true;
	}
	
	void WinRT_View::OnVisibilityChanged(_In_ CoreWindow^ sender, _In_ VisibilityChangedEventArgs^ args)
	{
		this->_updateViewState();
		args->Handled = true;
	}
	
	void WinRT_View::OnSuspend(_In_ Platform::Object^ sender, _In_ SuspendingEventArgs^ args)
	{
		april::window->handleFocusChangeEvent(false);
	}
	
	void WinRT_View::OnResume(_In_ Platform::Object^ sender, _In_ Platform::Object^ args)
	{
		this->_updateViewState();
		april::window->handleFocusChangeEvent(true);
	}
	
	void WinRT_View::OnWindowClosed(_In_ CoreWindow^ sender, _In_ CoreWindowEventArgs^ args)
	{
		april::window->handleQuitRequest(false);
		args->Handled = true;
	}
	
	void WinRT_View::OnTouchDown(_In_ CoreWindow^ sender, _In_ PointerEventArgs^ args)
	{
		unsigned int id;
		int index;
		gvec2 position(args->CurrentPoint->Position.X, args->CurrentPoint->Position.Y);
		switch (args->CurrentPoint->PointerDevice->PointerDeviceType)
		{
		case Windows::Devices::Input::PointerDeviceType::Mouse:
			april::window->setInputMode(april::Window::MOUSE);
			april::window->handleMouseEvent(april::Window::AMOUSEEVT_DOWN, position, april::AK_LBUTTON);
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
			((WinRT_Window*)april::window)->queueTouchEvent(april::Window::AMOUSEEVT_DOWN, position, index);
			break;
		}
		args->Handled = true;
	}
	
	void WinRT_View::OnTouchUp(_In_ CoreWindow^ sender, _In_ PointerEventArgs^ args)
	{
		unsigned int id;
		int index;
		gvec2 position(args->CurrentPoint->Position.X, args->CurrentPoint->Position.Y);
		switch (args->CurrentPoint->PointerDevice->PointerDeviceType)
		{
		case Windows::Devices::Input::PointerDeviceType::Mouse:
			april::window->setInputMode(april::Window::MOUSE);
			april::window->handleMouseEvent(april::Window::AMOUSEEVT_UP, position, april::AK_LBUTTON);
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
			((WinRT_Window*)april::window)->queueTouchEvent(april::Window::AMOUSEEVT_UP, position, index);
			break;
		}
		args->Handled = true;
	}
	
	void WinRT_View::OnTouchMove(_In_ CoreWindow^ sender, _In_ PointerEventArgs^ args)
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
			april::window->handleMouseEvent(april::Window::AMOUSEEVT_MOVE, position, april::AK_LBUTTON);
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
			((WinRT_Window*)april::window)->queueTouchEvent(april::Window::AMOUSEEVT_MOVE, position, index);
			break;
		}
		args->Handled = true;
	}
	
	void WinRT_View::OnMouseScroll(_In_ CoreWindow^ sender, _In_ PointerEventArgs^ args)
	{
		april::window->setInputMode(april::Window::MOUSE);
		float _wheelDelta = (float)args->CurrentPoint->Properties->MouseWheelDelta / WHEEL_DELTA;
		if (this->scrollHorizontal ^ args->CurrentPoint->Properties->IsHorizontalMouseWheel)
		{
			april::window->handleMouseEvent(april::Window::AMOUSEEVT_SCROLL,
				gvec2(-(float)_wheelDelta, 0.0f), april::AK_NONE);
		}
		else
		{
			april::window->handleMouseEvent(april::Window::AMOUSEEVT_SCROLL,
				gvec2(0.0f, -(float)_wheelDelta), april::AK_NONE);
		}
		args->Handled = true;
	}
	
	void WinRT_View::OnKeyDown(_In_ CoreWindow^ sender, _In_ KeyEventArgs^ args)
	{
		april::Key key = (april::Key)args->VirtualKey;
		april::window->handleKeyOnlyEvent(april::Window::AKEYEVT_DOWN, key);
		if (key == AK_CONTROL || key == AK_LCONTROL || key == AK_RCONTROL)
		{
			this->scrollHorizontal = true;
		}
		args->Handled = true;
	}
	
	void WinRT_View::OnKeyUp(_In_ CoreWindow^ sender, _In_ KeyEventArgs^ args)
	{
		april::Key key = (april::Key)args->VirtualKey;
		april::window->handleKeyOnlyEvent(april::Window::AKEYEVT_DOWN, key);
		if (key == AK_CONTROL || key == AK_LCONTROL || key == AK_RCONTROL)
		{
			this->scrollHorizontal = false;
		}
		args->Handled = true;
	}
	
	void WinRT_View::OnCharacterReceived(_In_ CoreWindow^ sender, _In_ CharacterReceivedEventArgs^ args)
	{
		april::window->handleCharOnlyEvent(args->KeyCode);
		args->Handled = true;
	}
	
	void WinRT_View::checkEvents()
	{
		this->window->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);
	}
	
}
#endif
