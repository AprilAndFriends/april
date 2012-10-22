/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @author  Ivan Vucica
/// @version 2.34
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _WIN32
#include <windows.h>
#include <winuser.h>

#include <hltypes/hltypesUtil.h>
#include <hltypes/hthread.h>

#include "april.h"
#include "RenderSystem.h"
#include "Timer.h"
#include "Win32_Window.h"

#define APRIL_WIN32_WINDOW_CLASS L"AprilWin32Window"

namespace april
{
#ifdef _OPENGL
	extern HDC hDC;
#endif
	
	Win32_Window::Win32_Window() : Window()
	{
		this->name = APRIL_WS_WIN32;
		this->touchEnabled = false;
		this->_fpsTitle = " [FPS: 0]";
	}

	Win32_Window::~Win32_Window()
	{
		this->destroy();
	}

	bool Win32_Window::create(int w, int h, bool fullscreen, chstr title)
	{
		if (!Window::create(w, h, fullscreen, title))
		{
			return false;
		}
		this->touchEnabled = false;
		this->_fpsTitle = " [FPS: 0]";
		// Win32
		WNDCLASSEXW wc;
		memset(&wc, 0, sizeof(WNDCLASSEX));
		HINSTANCE hinst = GetModuleHandle(0);
		wc.cbSize = sizeof(WNDCLASSEX);
		wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
		wc.lpfnWndProc = &Win32_Window::processCallback;
		wc.hInstance = hinst;
		wc.hCursor = LoadCursor(0, IDC_ARROW);
		wc.lpszClassName = APRIL_WIN32_WINDOW_CLASS;
		wc.hIcon = (HICON)LoadImage(hinst, MAKEINTRESOURCE(1), IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR);
		wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
		RegisterClassExW(&wc);

		int x = 0;
		int y = 0;
		if (!this->fullscreen)
		{
			x = (GetSystemMetrics(SM_CXSCREEN) - w) / 2;
			y = (GetSystemMetrics(SM_CYSCREEN) - h) / 2;
		}
		DWORD style = (this->fullscreen ? (WS_EX_TOPMOST | WS_POPUP) : (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX));
		this->hWnd = CreateWindowExW(0, APRIL_WIN32_WINDOW_CLASS, this->title.w_str().c_str(), style, x, y, w, h, NULL, NULL, hinst, NULL);
		
		if (!this->fullscreen)
		{
			RECT rcClient;
			RECT rcWindow;
			POINT ptDiff;
			GetClientRect(hWnd, &rcClient);
			GetWindowRect(hWnd, &rcWindow);
			ptDiff.x = (rcWindow.right - rcWindow.left) - rcClient.right;
			ptDiff.y = (rcWindow.bottom - rcWindow.top) - rcClient.bottom;
			MoveWindow(this->hWnd, rcWindow.left, rcWindow.top, ptDiff.x + w, ptDiff.y + h, TRUE);
		}
 
		// display the window on the screen
		ShowWindow(this->hWnd, 1);
		UpdateWindow(this->hWnd);
		SetCursor(wc.hCursor);
		this->setCursorVisible(true);
		return true;
	}
	
	bool Win32_Window::destroy()
	{
		if (!Window::destroy())
		{
			return false;
		}
		if (this->hWnd != 0)
		{
			DestroyWindow(this->hWnd);
			UnregisterClassW(APRIL_WIN32_WINDOW_CLASS, GetModuleHandle(0));
			this->hWnd = 0;
		}
		return true;
	}

	void Win32_Window::setTitle(chstr title)
	{
		this->title = title;
		hstr t = this->title;
#ifdef _DEBUG
		t += this->_fpsTitle;
#endif
		SetWindowTextW(this->hWnd, t.w_str().c_str());
	}
	
	bool Win32_Window::isCursorVisible()
	{
		return (Window::isCursorVisible() || !this->isCursorInside());
	}
	
	void Win32_Window::setCursorVisible(bool value)
	{
		Window::setCursorVisible(value);
		this->isCursorVisible() ? SetCursor(LoadCursor(0, IDC_ARROW)) : SetCursor(0);
	}
	
	int Win32_Window::getWidth()
	{
		RECT rc;
		GetClientRect(hWnd, &rc);
		return (rc.right - rc.left);
	}
	
	int Win32_Window::getHeight()
	{
		RECT rc;
		GetClientRect(hWnd, &rc);
		return (rc.bottom - rc.top);
	}

	void* Win32_Window::getBackendId()
	{
		return this->hWnd;
	}

	void Win32_Window::_setResolution(int w, int h)
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
	
	void Win32_Window::enterMainLoop()
	{
		this->_lastTime = this->globalTimer.getTime();
		this->_fpsTimer = this->_lastTime;
		this->_fps = 0;
		Window::enterMainLoop();
	}

	bool Win32_Window::updateOneFrame()
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
	
	void Win32_Window::presentFrame()
	{
#ifdef _OPENGL
		if (april::rendersys->getName() == APRIL_RS_OPENGL)
		{
			SwapBuffers(hDC);
		}
#endif
	}
	
	void Win32_Window::checkEvents()
	{
		MSG msg;
		if (PeekMessageW(&msg, this->hWnd, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
	}

	LRESULT CALLBACK Win32_Window::processCallback(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		static bool _touchDown = false;
		static bool _doubleTapDown = false;
		static int _mouseMoveMessagesCount = 0;
		static float _wheelDelta = 0.0f;
		if (!april::window->isCreated()) // don't run callback processing if window was "destroyed"
		{
			return 1;
		}
		switch (message)
		{
		case 0x0119: // WM_GESTURE (Win7+ only)
			if (wParam == 1) // GID_BEGIN
			{
				_touchDown = true;
			}
			else if (wParam == 2) // GID_END
			{
				if (_doubleTapDown)
				{ 
					_doubleTapDown = false;
					april::window->handleMouseEvent(AMOUSEEVT_UP, april::window->getCursorPosition(), AMOUSEBTN_DOUBLETAP);
				}
				_touchDown = false;
			}
			else if (wParam == 6) // GID_TWOFINGERTAP
			{
				_doubleTapDown = true;
				april::window->handleMouseEvent(AMOUSEEVT_DOWN, april::window->getCursorPosition(), AMOUSEBTN_DOUBLETAP);
			}
			break;
		case 0x011A: // WM_GESTURENOTIFY (win7+ only)
			_touchDown = true;
			april::window->handleTouchscreenEnabledEvent(true);
			break;
		case WM_DESTROY:
		case WM_CLOSE:
			if (april::window->handleQuitRequest(true))
			{
				PostQuitMessage(0);
				april::window->terminateMainLoop();
			}
			return 0;
		case WM_SYSKEYDOWN:
		case WM_KEYDOWN:
			april::window->handleKeyOnlyEvent(AKEYEVT_DOWN, (april::KeySym)wParam);
			return 0;
		case WM_SYSKEYUP:
		case WM_KEYUP: 
			april::window->handleKeyOnlyEvent(AKEYEVT_UP, (april::KeySym)wParam);
			return 0;
		case WM_CHAR:
			april::window->handleCharOnlyEvent(wParam);
			break;
		case WM_LBUTTONDOWN:
			_touchDown = true;
			_mouseMoveMessagesCount = 0;
			april::window->handleMouseEvent(AMOUSEEVT_DOWN, april::window->getCursorPosition(), AMOUSEBTN_LEFT);
			if (!april::window->isFullscreen())
			{
				SetCapture((HWND)april::window->getBackendId());
			}
			break;
		case WM_RBUTTONDOWN:
			_touchDown = true;
			_mouseMoveMessagesCount = 0;
			april::window->handleMouseEvent(AMOUSEEVT_DOWN, april::window->getCursorPosition(), AMOUSEBTN_RIGHT);
			if (!april::window->isFullscreen())
			{
				SetCapture((HWND)april::window->getBackendId());
			}
			break;
		case WM_LBUTTONUP:
			_touchDown = false;
			april::window->handleMouseEvent(AMOUSEEVT_UP, april::window->getCursorPosition(), AMOUSEBTN_LEFT);
			if (!april::window->isFullscreen())
			{
				ReleaseCapture();
			}
			break;
		case WM_RBUTTONUP:
			_touchDown = false;
			april::window->handleMouseEvent(AMOUSEEVT_UP, april::window->getCursorPosition(), AMOUSEBTN_RIGHT);
			if (!april::window->isFullscreen())
			{
				ReleaseCapture();
			}
			break;
		case WM_MOUSEMOVE:
			if (!_touchDown)
			{
				if (_mouseMoveMessagesCount >= 10)
				{
					april::window->handleTouchscreenEnabledEvent(false);
				}
				else
				{
					_mouseMoveMessagesCount++;
				}
			}
			else
			{
				_mouseMoveMessagesCount = 0;
			}
			april::window->handleMouseEvent(AMOUSEEVT_MOVE, april::window->getCursorPosition(), AMOUSEBTN_NONE);
			break;
		case WM_MOUSEWHEEL:
			_wheelDelta = (float)GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
			if ((GET_KEYSTATE_WPARAM(wParam) & MK_CONTROL) != MK_CONTROL)
			{
				april::window->handleMouseEvent(AMOUSEEVT_SCROLL, gvec2(0.0f, -(float)_wheelDelta), AMOUSEBTN_NONE);
			}
			else
			{
				april::window->handleMouseEvent(AMOUSEEVT_SCROLL, gvec2(-(float)_wheelDelta, 0.0f), AMOUSEBTN_NONE);
			}
			break;
		case WM_MOUSEHWHEEL:
			_wheelDelta = (float)GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
			if ((GET_KEYSTATE_WPARAM(wParam) & MK_CONTROL) != MK_CONTROL)
			{
				april::window->handleMouseEvent(AMOUSEEVT_SCROLL, gvec2(-(float)_wheelDelta, 0.0f), AMOUSEBTN_NONE);
			}
			else
			{
				april::window->handleMouseEvent(AMOUSEEVT_SCROLL, gvec2(0.0f, -(float)_wheelDelta), AMOUSEBTN_NONE);
			}
			break;
		case WM_SETCURSOR:
			april::window->isCursorVisible() ? SetCursor(LoadCursor(0, IDC_ARROW)) : SetCursor(0);
			return 1;
		case WM_ACTIVATE:
			if (wParam == WA_ACTIVE || wParam == WA_CLICKACTIVE)
			{
				april::log("Window activated");
				april::window->handleFocusChangeEvent(true);
			}
			else
			{
				april::log("Window deactivated");
				april::window->handleFocusChangeEvent(false);
			}
			break;
		}
		return DefWindowProcW(hWnd, message, wParam, lParam);
	}

}
#endif