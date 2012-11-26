/// @file
/// @author  Boris Mikic
/// @version 2.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _WIN32
#include <hltypes/hplatform.h>
#if _HL_WINRT

#include <windows.h>

#include <hltypes/hlog.h>
#include <hltypes/hresource.h>
#include <hltypes/hstring.h>

#include "Window.h"
#include "WinRT_View.h"
#include "WinRT_Window.h"

using namespace Windows::Foundation;
using namespace Windows::ApplicationModel;

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
        this->window->PointerCursor = ref new CoreCursor(CoreCursorType::Arrow, 0);
		this->window->SizeChanged +=
            ref new TypedEventHandler<CoreWindow^, WindowSizeChangedEventArgs^>(
                this, &WinRT_View::OnWindowSizeChanged);
		this->window->VisibilityChanged +=
			ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(
                this, &WinRT_View::OnVisibilityChanged);
		this->window->PointerPressed +=
			ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(
				this, &WinRT_View::OnMouseDown);
		this->window->PointerReleased +=
			ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(
				this, &WinRT_View::OnMouseUp);
		this->window->PointerMoved +=
			ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(
				this, &WinRT_View::OnMouseMove);
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
	}
	
	void WinRT_View::Load(_In_ Platform::String^ entryPoint)
	{
	}

	void WinRT_View::Run()
	{
		hresource::setCwd(normalize_path(unicode_to_utf8(Package::Current->InstalledLocation->Path->Data())));
		hresource::setArchive("");
		WinRT::View = this;
		(*WinRT::Init)(WinRT::Args);
		april::window->enterMainLoop();
		(*WinRT::Destroy)();
	}

	void WinRT_View::OnActivated(_In_ CoreApplicationView^ applicationView, _In_ IActivatedEventArgs^ args)
	{
        CoreWindow::GetForCurrentThread()->Activate();
	}
	
	void WinRT_View::OnWindowSizeChanged(_In_ CoreWindow^ sender, _In_ WindowSizeChangedEventArgs^ args)
	{
        // TODO
		//args->Handled = true;
	}

	void WinRT_View::OnVisibilityChanged(_In_ CoreWindow^ sender, _In_ VisibilityChangedEventArgs^ args)
	{
		april::window->handleFocusChangeEvent(args->Visible);
		args->Handled = true;
	}

	void WinRT_View::OnMouseDown(_In_ CoreWindow^ sender, _In_ PointerEventArgs^ args)
	{
		((WinRT_Window*)april::window)->handleTouchEvent(april::Window::AMOUSEEVT_DOWN,
			gvec2(args->CurrentPoint->Position.X, args->CurrentPoint->Position.Y),
			args->CurrentPoint->PointerId - 1);
		args->Handled = true;
	}

	void WinRT_View::OnMouseUp(_In_ CoreWindow^ sender, _In_ PointerEventArgs^ args)
	{
		((WinRT_Window*)april::window)->handleTouchEvent(april::Window::AMOUSEEVT_UP,
			gvec2(args->CurrentPoint->Position.X, args->CurrentPoint->Position.Y),
			args->CurrentPoint->PointerId - 1);
		args->Handled = true;
	}

	void WinRT_View::OnMouseMove(_In_ CoreWindow^ sender, _In_ PointerEventArgs^ args)
	{
		april::window->handleMouseEvent(april::Window::AMOUSEEVT_MOVE, gvec2(args->CurrentPoint->Position.X,
			args->CurrentPoint->Position.Y), april::Window::AMOUSEBTN_NONE);
		args->Handled = true;
	}

	void WinRT_View::OnMouseScroll(_In_ CoreWindow^ sender, _In_ PointerEventArgs^ args)
	{
		float delta = (float)args->CurrentPoint->Properties->MouseWheelDelta;
		// TODO
		//args->CurrentPoint->
		//april::window->handleMouseEvent(april::Window::AMOUSEEVT_SCROLL, gvec2(args->CurrentPoint->Position.X,
		//	args->CurrentPoint->Position.Y), april::Window::AMOUSEBTN_NONE);
		args->Handled = true;
	}

	void WinRT_View::OnKeyDown(_In_ CoreWindow^ sender, _In_ KeyEventArgs^ args)
	{
		april::window->handleKeyOnlyEvent(april::Window::AKEYEVT_DOWN, (april::KeySym)args->VirtualKey);
		args->Handled = true;
	}

	void WinRT_View::OnKeyUp(_In_ CoreWindow^ sender, _In_ KeyEventArgs^ args)
	{
		april::window->handleKeyOnlyEvent(april::Window::AKEYEVT_DOWN, (april::KeySym)args->VirtualKey);
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
#endif