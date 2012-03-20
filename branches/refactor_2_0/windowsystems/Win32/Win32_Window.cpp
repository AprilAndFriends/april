/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @author  Ivan Vucica
/// @version 2.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _WIN32
#include <windows.h>

#include <hltypes/hltypesUtil.h>
#include <hltypes/hthread.h>

#include "april.h"
#include "RenderSystem.h"
#include "Timer.h"
#include "Win32_Window.h"

namespace april
{
#ifdef _OPENGL
	extern HDC hDC;
#endif
	
/************************************************************************************/
	LRESULT CALLBACK Win32_Window::processCallback(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		return ((Win32_Window*)april::window)->_processEvents(hWnd, message, wParam, lParam);
	}

	Win32_Window::Win32_Window(int width, int height, bool fullscreen, chstr title) : Window()
	{
		if (april::rendersys != NULL)
		{
			april::log("Creating Win32 Windowsystem");
		}
		this->fullscreen = fullscreen;
		this->title = title;
		this->touchEnabled = false;
		this->fpsTitle = " [FPS: 0]";
		this->_touchDown = false;
		this->_doubleTapDown = false;
		this->_nMouseMoveMessages = 0;
		// Win32
		WNDCLASSEXW wc;
		ZeroMemory(&wc, sizeof(WNDCLASSEX));
		
		HINSTANCE hinst = GetModuleHandle(0);
		wc.cbSize = sizeof(WNDCLASSEX);
		wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
		wc.lpfnWndProc = &Win32_Window::processCallback;
		wc.hInstance = hinst;
		wc.hCursor = LoadCursor(0, IDC_ARROW);
		wc.lpszClassName = L"april_win32_window";
		wc.hIcon = (HICON)LoadImage(hinst, MAKEINTRESOURCE(1), IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR);
		wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
		RegisterClassExW(&wc);

		int x = 0;
		int y = 0;
		if (!fullscreen)
		{
			x = (GetSystemMetrics(SM_CXSCREEN) - width) / 2;
			y = (GetSystemMetrics(SM_CYSCREEN) - height) / 2;
		}
		WCHAR wtitle[256] = {0};
		for_iter (i, 0, this->title.size())
		{
			wtitle[i] = this->title[i];
		}
		DWORD style = (this->fullscreen ? WS_EX_TOPMOST | WS_POPUP : WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX);
		april::log(hsprintf("title: %s %d %d %d %d inst: %d", this->title.c_str(), x, y, width, height, hinst));
		this->hWnd = CreateWindowExW(0, L"april_win32_window", wtitle, style, x, y, width, height, NULL, NULL, hinst, NULL);
		
		if (!this->fullscreen)
		{
			RECT rcClient;
			RECT rcWindow;
			POINT ptDiff;
			GetClientRect(hWnd, &rcClient);
			GetWindowRect(hWnd, &rcWindow);
			ptDiff.x = (rcWindow.right - rcWindow.left) - rcClient.right;
			ptDiff.y = (rcWindow.bottom - rcWindow.top) - rcClient.bottom;
			MoveWindow(this->hWnd, rcWindow.left, rcWindow.top, width + ptDiff.x, height + ptDiff.y, TRUE);
		}
 
		// display the window on the screen
		ShowWindow(this->hWnd, 1);
		UpdateWindow(this->hWnd);
		SetCursor(wc.hCursor);
		this->setCursorVisible(true);
	}
	
	Win32_Window::~Win32_Window()
	{
		//log("Destroying Win32 Windowsystem");
	}

	void Win32_Window::destroyWindow()
	{
		if (this->hWnd != 0)
		{
			DestroyWindow(this->hWnd);
			UnregisterClassW(L"april_win32_window", GetModuleHandle(0));
			this->hWnd = 0;
		}
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

	void Win32_Window::setTitle(chstr title)
	{
		this->title = title;
#ifdef _DEBUG
		hstr t = this->title + this->fpsTitle;
#else
		chstr t = this->title;
#endif
		WCHAR wtitle[256] = {0};
		for_iter (i, 0, t.size())
		{
			wtitle[i] = t[i];
		}
		SetWindowTextW(hWnd, wtitle);
	}
	
	void Win32_Window::_setResolution(int width, int height)
	{
		int x = 0;
		int y = 0;
		DWORD style = WS_EX_TOPMOST | WS_POPUP;
		if (!this->fullscreen)
		{
			x = (GetSystemMetrics(SM_CXSCREEN) - width) / 2,
			y = (GetSystemMetrics(SM_CYSCREEN) - height) / 2;
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
			MoveWindow(this->hWnd, rcWindow.left, rcWindow.top, width + ptDiff.x, height + ptDiff.y, TRUE);
		}
		// display the window on the screen
		ShowWindow(this->hWnd, 1);
		UpdateWindow(this->hWnd);
	}
	
	void Win32_Window::showSystemCursor(bool b)
	{
		b ? SetCursor(LoadCursor(0, IDC_ARROW)) : SetCursor(0);
		cursorVisible = b;
	}
	
	bool Win32_Window::isSystemCursorShown()
	{
		if (cursorVisible)
		{
			return true;
		}
		return !(is_in_range(cursorPosition.x, 0.0f, (float)getWidth()) && is_in_range(cursorPosition.y, 0.0f, (float)getHeight()));
	}
	
	void Win32_Window::doEvents()
	{
		MSG msg;
		if (PeekMessageW(&msg, hWnd, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
	}

	void Win32_Window::presentFrame()
	{
#ifdef _OPENGL
		SwapBuffers(hDC);
#endif
	}
	
	void Win32_Window::terminateMainLoop()
	{
		this->running = false;
	}
	
	void Win32_Window::enterMainLoop()
	{
		this->_lastTime = this->globalTimer.getTime();
		this->_fpsTimer = this->_lastTime;
		this->_fps = 0;
		while (this->running)
		{
			this->running = this->updateOneFrame();
		}
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
		this->doEvents();
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
				this->doEvents();
				hthread::sleep(40.0f);
			}
		}
		// rendering
		result = this->performUpdate(k);
#ifndef _DEBUG
		this->setWindowTitle(this->title);
#else
		if (this->_lastTime - this->_fpsTimer > 1000)
		{
			this->fpsTitle = hsprintf(" [FPS: %d]", this->_fps);
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
		return result;
	}
	
	void* Win32_Window::getIDFromBackend()
	{
		return this->hWnd;
	}

	LRESULT Win32_Window::_processEvents(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		switch (message)
		{
		case 0x0119: // WM_GESTURE (Win7+ only)
			if (wParam == 1) // GID_BEGIN
			{
				this->_touchDown = true;
			}
			else if (wParam == 2) // GID_END
			{
				if (this->_doubleTapDown)
				{ 
					this->_doubleTapDown = false;
					this->handleMouseEvent(AMOUSEEVT_UP, this->cursorPosition, AMOUSEBTN_DOUBLETAP);
				}
				this->_touchDown = false;
			}
			else if (wParam == 6) // GID_TWOFINGERTAP
			{
				this->_doubleTapDown = true;
				this->handleMouseEvent(AMOUSEEVT_DOWN, this->cursorPosition, AMOUSEBTN_DOUBLETAP);
			}
			break;
		case 0x011A: // WM_GESTURENOTIFY (win7+ only)
			this->_touchDown = true;
			this->handleTouchscreenEnabledEvent(true);
			break;
		case WM_DESTROY:
		case WM_CLOSE:
			if (this->handleQuitRequest(true))
			{
				PostQuitMessage(0);
				this->terminateMainLoop();
			}
			return 0;
			break;
		case WM_KEYDOWN:
			this->_handleKeyOnlyEvent(AKEYEVT_DOWN, (april::KeySym)wParam);
			break;
		case WM_KEYUP: 
			this->_handleKeyOnlyEvent(AKEYEVT_UP, (april::KeySym)wParam);
			break;
		case WM_CHAR:
			this->_handleCharOnlyEvent(wParam);
			break;
		case WM_LBUTTONDOWN:
			this->_touchDown = true;
			this->_nMouseMoveMessages = 0;
			this->handleMouseEvent(AMOUSEEVT_DOWN, this->cursorPosition, AMOUSEBTN_LEFT);
			if (!april::window->isFullscreen())
			{
				SetCapture(this->hWnd);
			}
			break;
		case WM_RBUTTONDOWN:
			this->_touchDown = true;
			this->_nMouseMoveMessages = 0;
			this->handleMouseEvent(AMOUSEEVT_DOWN, this->cursorPosition, AMOUSEBTN_RIGHT);
			if (!april::window->isFullscreen())
			{
				SetCapture(this->hWnd);
			}
			break;
		case WM_LBUTTONUP:
			this->_touchDown = false;
			this->handleMouseEvent(AMOUSEEVT_UP, this->cursorPosition, AMOUSEBTN_LEFT);
			if (!april::window->isFullscreen())
			{
				ReleaseCapture();
			}
			break;
		case WM_RBUTTONUP:
			this->_touchDown = false;
			this->handleMouseEvent(AMOUSEEVT_UP, this->cursorPosition, AMOUSEBTN_RIGHT);
			if (!april::window->isFullscreen())
			{
				ReleaseCapture();
			}
			break;
		case WM_MOUSEMOVE:
			if (!this->_touchDown)
			{
				if (this->_nMouseMoveMessages >= 10)
				{
					this->handleTouchscreenEnabledEvent(false);
				}
				else
				{
					this->_nMouseMoveMessages++;
				}
			}
			else
			{
				this->_nMouseMoveMessages = 0;
			}
			this->handleMouseEvent(AMOUSEEVT_MOVE, this->cursorPosition, AMOUSEBTN_NONE);
			break;
		case WM_SETCURSOR:
			this->setCursorVisible(this->isCursorInside());
			return 1;
		case WM_ACTIVATE:
			if (wParam == WA_ACTIVE || wParam == WA_CLICKACTIVE)
			{
				this->handleFocusChangeEvent(true);
				april::log("Window activated");
			}
			else
			{
				this->handleFocusChangeEvent(false);
				april::log("Window deactivated");
			}
			break;
		}
		return DefWindowProcW(hWnd, message, wParam, lParam);
	}

}
#endif