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

#include <hltypes/hltypesUtil.h>
#include <hltypes/hthread.h>

#include "april.h"
#include "RenderSystem.h"
#include "Timer.h"
#include "WinRT_View.h"
#include "WinRT_ViewSource.h"
#include "WinRT_Window.h"

using namespace Windows::ApplicationModel::Core;

namespace april
{
	WinRT_Window::WinRT_Window() : Window()
	{
		this->name = APRIL_WS_WINRT;
		this->width = 0;
		this->height = 0;
		this->touchEnabled = false;
	}

	WinRT_Window::~WinRT_Window()
	{
		this->destroy();
	}

	bool WinRT_Window::create(int w, int h, bool fullscreen, chstr title)
	{
		if (!Window::create(w, h, fullscreen, title))
		{
			return false;
		}
		this->width = w;
		this->height = h;
		this->setCursorVisible(true);
		return true;
	}
	
	bool WinRT_Window::destroy()
	{
		if (!Window::destroy())
		{
			return false;
		}
		return true;
	}

	/*
	void WinRT_Window::setTitle(chstr title)
	{
		this->title = title;
		hstr t = this->title;
#ifdef _DEBUG
		t += this->_fpsTitle;
#endif
		SetWindowTextW(this->hWnd, t.w_str().c_str());
	}
	
	bool WinRT_Window::isCursorVisible()
	{
		return (Window::isCursorVisible() || !this->isCursorInside());
	}
	
	void WinRT_Window::setCursorVisible(bool value)
	{
		Window::setCursorVisible(value);
		this->isCursorVisible() ? SetCursor(LoadCursor(0, IDC_ARROW)) : SetCursor(0);
	}
	*/
	
	void* WinRT_Window::getBackendId()
	{
		// TODO
		return 0;
	}

	/*
	void WinRT_Window::_setResolution(int w, int h)
	{
		int x = 0;
		int y = 0;
		DWORD style = WS_EX_TOPMOST | WS_POPUP;
		if (!this->fullscreen)
		{
			x = (GetSystemMetrics(SM_CXSCREEN) - w) / 2,
			y = (GetSystemMetrics(SM_CYSCREEN) - h) / 2;
			style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
		}
		if (!this->fullscreen)
		{
			RECT rcClient;
			RECT rcWindow;
			POINT ptDiff;
			GetClientRect(this->hWnd, &rcClient);
			GetWindowRect(this->hWnd, &rcWindow);
			ptDiff.x = (rcWindow.right - rcWindow.left) - rcClient.right;
			ptDiff.y = (rcWindow.bottom - rcWindow.top) - rcClient.bottom;
			MoveWindow(this->hWnd, rcWindow.left, rcWindow.top, ptDiff.x + w, ptDiff.y + h, TRUE);
		}
		// display the window on the screen
		ShowWindow(this->hWnd, 1);
		UpdateWindow(this->hWnd);
	}
	*/
	
	void WinRT_Window::enterMainLoop()
	{
		CoreApplication::Run(ref new WinRT_ViewSource());
	}

	/*
	bool WinRT_Window::updateOneFrame()
	{
		static bool result = true;
		static float t = 0.0f;
		static float k = 0.0f;
		static POINT w32_cursorPosition;
		// mouse position
		GetCursorPos(&w32_cursorPosition);
		ScreenToClient(this->hWnd, &w32_cursorPosition);
		this->cursorPosition.set((float)w32_cursorPosition.x, (float)w32_cursorPosition.y);
		this->checkEvents();
		t = this->globalTimer.getTime();
		if (t == this->_lastTime)
		{
			return true; // don't redraw frames which won't change
		}
		k = (t - this->_lastTime) / 1000.0f;
		if (k > 0.5f)
		{
			k = 0.05f; // prevent jumps. from eg, waiting on device reset or super low framerate
		}

		this->_lastTime = t;
		if (!this->focused)
		{
			k = 0.0f;
			for_iter (i, 0, 5)
			{
				this->checkEvents();
				hthread::sleep(40.0f);
			}
		}
		// rendering
		result = this->performUpdate(k);
#ifndef _DEBUG
		this->setTitle(this->title);
#else
		if (this->_lastTime - this->_fpsTimer > 1000)
		{
			this->_fpsTitle = hsprintf(" [FPS: %d]", this->_fps);
			this->setTitle(this->title);
			this->_fps = 0;
			this->_fpsTimer = this->_lastTime;
		}
		else
		{
			this->_fps++;
		}
#endif			
		april::rendersys->presentFrame();
		return (result && Window::updateOneFrame());
	}
	*/
	
	void WinRT_Window::presentFrame()
	{
	}
	
	void WinRT_Window::checkEvents()
	{
		// events are handled by WinRT_View
	}

}
#endif
#endif