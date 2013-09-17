/// @file
/// @author  Boris Mikic
/// @version 3.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#if defined(_WINRT_WINDOW) && defined(_WINP8)

#include <hltypes/hlog.h>
#include <hltypes/hplatform.h>
#include <hltypes/hresource.h>
#include <hltypes/hstring.h>

#include "april.h"
#include "DirectX11_RenderSystem.h"
#include "Platform.h"
#include "SystemDelegate.h"
#include "Window.h"
#include "WinRT.h"
#include "WinRT_View.h"
#include "WinRT_Window.h"

using namespace Windows::ApplicationModel;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Display;
using namespace Windows::UI::ViewManagement;

#define DX11_RENDERSYS ((DirectX11_RenderSystem*)april::rendersys)

namespace april
{
	void WinRT_View::Initialize(_In_ CoreApplicationView^ applicationView)
	{
		DisplayProperties::AutoRotationPreferences = (DisplayOrientations::Landscape | DisplayOrientations::LandscapeFlipped);
		applicationView->Activated +=
			ref new TypedEventHandler<CoreApplicationView^, IActivatedEventArgs^>(this, &WinRT_View::OnActivated);
	}
	
	void WinRT_View::Uninitialize()
	{
	}
	
	void WinRT_View::SetWindow(_In_ CoreWindow^ window)
	{
		this->window = window;
		CoreApplication::Suspending +=
			ref new EventHandler<SuspendingEventArgs^>(this, &WinRT_View::OnSuspend);
		CoreApplication::Resuming +=
			ref new EventHandler<Platform::Object^>(this, &WinRT_View::OnResume);
		DisplayProperties::OrientationChanged +=
			ref new DisplayPropertiesEventHandler(
				this, &WinRT_View::OnOrientationChanged);
		DisplayProperties::LogicalDpiChanged +=
			ref new DisplayPropertiesEventHandler(
				this, &WinRT_View::OnLogicalDpiChanged);
		this->window->SizeChanged +=
			ref new TypedEventHandler<CoreWindow^, WindowSizeChangedEventArgs^>(
				this, &WinRT_View::OnWindowSizeChanged);
		this->window->VisibilityChanged +=
			ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(
				this, &WinRT_View::OnVisibilityChanged);
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
	}
	
	void WinRT_View::Run()
	{
		this->scrollHorizontal = false;
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
		// not used on WinP8
	}
	
	void WinRT_View::OnActivated(_In_ CoreApplicationView^ applicationView, _In_ IActivatedEventArgs^ args)
	{
		CoreWindow::GetForCurrentThread()->Activate();
	}
	
	void WinRT_View::OnWindowSizeChanged(_In_ CoreWindow^ sender, _In_ WindowSizeChangedEventArgs^ args)
	{
		april::SystemDelegate* systemDelegate = april::window->getSystemDelegate();
		if (systemDelegate != NULL)
		{
			systemDelegate->onWindowSizeChanged((int)args->Size.Width, (int)args->Size.Height, true);
		}
		args->Handled = true;
	}
	
	void WinRT_View::OnVisibilityChanged(_In_ CoreWindow^ sender, _In_ VisibilityChangedEventArgs^ args)
	{
		args->Handled = true;
	}
	
	void WinRT_View::OnOrientationChanged(_In_ Platform::Object^ sender)
	{
		DX11_RENDERSYS->updateOrientation();
	}
	
	void WinRT_View::OnLogicalDpiChanged(_In_ Platform::Object^ sender)
	{
		DX11_RENDERSYS->updateOrientation();
	}
	
	void WinRT_View::OnSuspend(_In_ Platform::Object^ sender, _In_ SuspendingEventArgs^ args)
	{
		april::window->handleFocusChangeEvent(false);
	}
	
	void WinRT_View::OnResume(_In_ Platform::Object^ sender, _In_ Platform::Object^ args)
	{
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
		gvec2 position = this->_translatePosition(args->CurrentPoint->Position.X, args->CurrentPoint->Position.Y);
#ifndef _WINP8
		switch (args->CurrentPoint->PointerDevice->PointerDeviceType)
		{
		case Windows::Devices::Input::PointerDeviceType::Mouse:
			april::window->setInputMode(april::Window::MOUSE);
			april::window->queueMouseEvent(april::Window::AMOUSEEVT_DOWN, position, april::AK_LBUTTON);
			break;
		case Windows::Devices::Input::PointerDeviceType::Touch:
		case Windows::Devices::Input::PointerDeviceType::Pen:
			this->mouseMoveMessagesCount = 0;
#endif
			april::window->setInputMode(april::Window::TOUCH);
			id = args->CurrentPoint->PointerId;
			index = this->pointerIds.index_of(id);
			if (index < 0)
			{
				index = this->pointerIds.size();
				this->pointerIds += id;
			}
			april::window->queueTouchEvent(april::Window::AMOUSEEVT_DOWN, position, index);
#ifndef _WINP8
			break;
		}
#endif
		args->Handled = true;
	}
	
	void WinRT_View::OnTouchUp(_In_ CoreWindow^ sender, _In_ PointerEventArgs^ args)
	{
		unsigned int id;
		int index;
		gvec2 position = this->_translatePosition(args->CurrentPoint->Position.X, args->CurrentPoint->Position.Y);
#ifndef _WINP8
		switch (args->CurrentPoint->PointerDevice->PointerDeviceType)
		{
		case Windows::Devices::Input::PointerDeviceType::Mouse:
			april::window->setInputMode(april::Window::MOUSE);
			april::window->queueMouseEvent(april::Window::AMOUSEEVT_UP, position, april::AK_LBUTTON);
			break;
		case Windows::Devices::Input::PointerDeviceType::Touch:
		case Windows::Devices::Input::PointerDeviceType::Pen:
			this->mouseMoveMessagesCount = 0;
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
			april::window->queueTouchEvent(april::Window::AMOUSEEVT_UP, position, index);
#ifndef _WINP8
			break;
		}
#endif
		args->Handled = true;
	}
	
	void WinRT_View::OnTouchMove(_In_ CoreWindow^ sender, _In_ PointerEventArgs^ args)
	{
		unsigned int id;
		int index;
		gvec2 position = this->_translatePosition(args->CurrentPoint->Position.X, args->CurrentPoint->Position.Y);
#ifndef _WINP8
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
#endif
			april::window->setInputMode(april::Window::TOUCH);
			id = args->CurrentPoint->PointerId;
			index = this->pointerIds.index_of(id);
			if (index < 0)
			{
				index = this->pointerIds.size();
			}
			april::window->queueTouchEvent(april::Window::AMOUSEEVT_MOVE, position, index);
#ifndef _WINP8
			break;
		}
#endif
		args->Handled = true;
	}
	
	void WinRT_View::OnMouseScroll(_In_ CoreWindow^ sender, _In_ PointerEventArgs^ args)
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
	
	void WinRT_View::OnKeyDown(_In_ CoreWindow^ sender, _In_ KeyEventArgs^ args)
	{
		april::Key key = (april::Key)args->VirtualKey;
		april::window->queueKeyEvent(april::Window::AKEYEVT_DOWN, key, 0);
		if (key == AK_CONTROL || key == AK_LCONTROL || key == AK_RCONTROL)
		{
			this->scrollHorizontal = true;
		}
		args->Handled = true;
	}
	
	void WinRT_View::OnKeyUp(_In_ CoreWindow^ sender, _In_ KeyEventArgs^ args)
	{
		april::Key key = (april::Key)args->VirtualKey;
		april::window->queueKeyEvent(april::Window::AKEYEVT_UP, key, 0);
		if (key == AK_CONTROL || key == AK_LCONTROL || key == AK_RCONTROL)
		{
			this->scrollHorizontal = false;
		}
		args->Handled = true;
	}
	
	void WinRT_View::OnCharacterReceived(_In_ CoreWindow^ sender, _In_ CharacterReceivedEventArgs^ args)
	{
		april::window->queueKeyEvent(april::Window::AKEYEVT_DOWN, AK_NONE, args->KeyCode);
		args->Handled = true;
	}
	
	void WinRT_View::checkEvents()
	{
		this->window->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);
	}
	
	gvec2 WinRT_View::_translatePosition(float x, float y)
	{
		static int w = 0;
		static int h = 0;
		if (w == 0 || h == 0)
		{
			gvec2 resolution = april::getSystemInfo().displayResolution;
			w = hround(resolution.x);
			h = hround(resolution.y);
		}
#ifdef _WINP8
		int angle = WinRT::getScreenRotation();
		if (angle == 90)
		{
			hswap(x, y);
			y = h - y;
		}
		if (angle == 180)
		{
			x = w - x;
			y = h - y;
		}
		if (angle == 270)
		{
			hswap(x, y);
			x = w - x;
		}
#endif
		int width = april::window->getWidth();
		int height = april::window->getHeight();
		if (w == width && h == height)
		{
			return gvec2(x, y);
		}
		return gvec2((float)(int)(x * width / w), (float)(int)(y * height / h));
	}

}
#endif
