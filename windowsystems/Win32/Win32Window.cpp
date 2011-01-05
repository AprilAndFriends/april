/************************************************************************************\
This source file is part of the Awesome Portable Rendering Interface Library         *
For latest info, see http://libapril.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes, Ivan Vucica                                        *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#include <windows.h>

#include <hltypes/util.h>

#include "RenderSystem.h"
#include "Timer.h"
#include "Win32Window.h"

namespace april
{
	static HWND hWnd;
	static gvec2 cursorPosition;
	static bool cursorVisible = true;
	static april::Timer globalTimer;
	static Win32Window* instance;
	
#ifdef _DEBUG
	static char fpstitle[1024] = " [FPS:0]";
#endif
/**********************************************************************************************/
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	Win32Window *ws = (Win32Window*) instance;
    switch(message)
    {
	case WM_DESTROY:
	case WM_CLOSE:
		if(ws->triggerQuitEvent())
		{
			PostQuitMessage(0);
			ws->terminateMainLoop();
		}
		return 0;
		break;
	case WM_KEYDOWN:
		ws->triggerKeyEvent(1, wParam);
		break;
	case WM_KEYUP: 
		ws->triggerKeyEvent(0, wParam);
		break;
	case WM_CHAR:
		ws->triggerCharEvent(wParam);
		break;
	case WM_LBUTTONDOWN:
		ws->triggerMouseDownEvent(AK_LBUTTON);
		if (!instance->isFullscreen())
		{
			SetCapture(hWnd);
		}
		break;
	case WM_RBUTTONDOWN:
		ws->triggerMouseDownEvent(AK_RBUTTON);
		if (!instance->isFullscreen())
		{
			SetCapture(hWnd);
		}
		break;
	case WM_LBUTTONUP:
		ws->triggerMouseUpEvent(AK_LBUTTON);
		if (!instance->isFullscreen())
		{
			ReleaseCapture();
		}
		break;
	case WM_RBUTTONUP:
		ws->triggerMouseUpEvent(AK_RBUTTON);
		if (!instance->isFullscreen())
		{
			ReleaseCapture();
		}
		break;
	case WM_MOUSEMOVE:
		ws->triggerMouseMoveEvent();
		break;
	case WM_SETCURSOR:
		if (!cursorVisible)
		{
			if (cursorPosition.x >= 0 && cursorPosition.y >= 0 && cursorPosition.x <= ws->getWidth() && cursorPosition.y <= ws->getHeight())
			{
				SetCursor(0);
			}
			else
			{
				SetCursor(LoadCursor(0,IDC_ARROW));
			}
		}
		return 1;
	case WM_ACTIVATE:
		if (wParam == WA_ACTIVE || wParam == WA_CLICKACTIVE)
		{
			instance->_setActive(1);
			ws->triggerFocusCallback(true);
			april::log("Window activated");
		}
		else
		{
			instance->_setActive(0);
			ws->triggerFocusCallback(false);
			april::log("Window deactivated");
		}
		break;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}
/**********************************************************************************************/
	Win32Window::Win32Window(int w, int h, bool fullscreen, chstr title) //:
		/*mTexCoordsEnabled(0), mColorEnabled(0), RenderSystem()*/
	{
		if (april::rendersys)
		{
			april::log("Creating Win32 Windowsystem");
		}
		instance = this;
		
		mRunning = mActive = true;
		mFullscreen = fullscreen;
		// WINDOW
		mTitle = title;
		WNDCLASSEX wc;
		ZeroMemory(&wc, sizeof(WNDCLASSEX));
		
		HINSTANCE hinst = GetModuleHandle(0);
		wc.cbSize = sizeof(WNDCLASSEX);
		wc.style = CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc = WindowProc;
		wc.hInstance = hinst;
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
		wc.lpszClassName = "april_win32_window";
		wc.hIcon = (HICON)LoadImage(hinst, MAKEINTRESOURCE(1), IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR);
		wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
		
		RegisterClassEx(&wc);
		int x = (fullscreen ? 0 : (GetSystemMetrics(SM_CXSCREEN) - w) / 2);
		int y = (fullscreen ? 0 : (GetSystemMetrics(SM_CYSCREEN) - h) / 2);
		
		DWORD style = (fullscreen ? WS_EX_TOPMOST | WS_POPUP : WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX);
		//april::logf("title: %s %d %d %d %d inst: %d", title.c_str(), x, y, w, h, hinst);
		april::log(hsprintf("title: %s %d %d %d %d inst: %d", title.c_str(), x, y, w, h, hinst));
		hWnd = CreateWindowEx(0, "april_win32_window", title.c_str(), style, x, y, w, h, NULL, NULL, hinst, NULL);
		
		if (!fullscreen)
		{
			RECT rcClient, rcWindow;
			POINT ptDiff;
			GetClientRect(hWnd, &rcClient);
			GetWindowRect(hWnd, &rcWindow);
			ptDiff.x = (rcWindow.right - rcWindow.left) - rcClient.right;
			ptDiff.y = (rcWindow.bottom - rcWindow.top) - rcClient.bottom;
			MoveWindow(hWnd, rcWindow.left, rcWindow.top, w + ptDiff.x, h + ptDiff.y, TRUE);
		}
 
		// display the window on the screen
		ShowWindow(hWnd, 1);
		UpdateWindow(hWnd);
		SetCursor(LoadCursor(0, IDC_ARROW));
	}
	
	Win32Window::~Win32Window()
	{
		//log("Destroying Win32 Windowsystem");
		UnregisterClass("april_win32_window", GetModuleHandle(0));
	}

	int Win32Window::getWidth()
	{
		RECT rc;
		GetClientRect(hWnd, &rc);
		return (rc.right - rc.left);
	}
	
	int Win32Window::getHeight()
	{
		RECT rc;
		GetClientRect(hWnd, &rc);
		return (rc.bottom - rc.top);
	}

	void Win32Window::setWindowTitle(chstr title)
	{
		mTitle  =title;
#ifdef _DEBUG
		SetWindowText(hWnd, (title + fpstitle).c_str());
#else
		SetWindowText(hWnd, title.c_str());
#endif
	}
	
	gvec2 Win32Window::getCursorPosition()
	{
		return cursorPosition;
	}

	void Win32Window::showSystemCursor(bool b)
	{
		b ? SetCursor(LoadCursor(0, IDC_ARROW)) : SetCursor(0);
		cursorVisible = b;
	}
	
	bool Win32Window::isSystemCursorShown()
	{
		if (cursorVisible)
		{
			return true;
		}
		return !(cursorPosition.x >= 0 && cursorPosition.y >= 0 && cursorPosition.x < getWidth() && cursorPosition.y < getHeight());
	}
	
	void Win32Window::doEvents()
	{
		MSG msg;
		if (PeekMessage(&msg, hWnd, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	void Win32Window::presentFrame()
	{
		// TODO will do opengl's swapbuffer here
	}
	
	void Win32Window::terminateMainLoop()
	{
		mRunning = false;
	}
	
	bool Win32Window::triggerQuitEvent()
	{
		if (mQuitCallback)
		{
			return mQuitCallback(true);
		}
		return true;
	}
	
	void Win32Window::triggerKeyEvent(bool down, unsigned int keycode)
	{
		if (down)
		{
			if (mKeyDownCallback)
			{
				mKeyDownCallback(keycode);
			}
		}
		else
		{
			if (mKeyUpCallback)
			{
				mKeyUpCallback(keycode);
			}
		}
	}
	
	void Win32Window::triggerCharEvent(unsigned int chr)
	{
		if (mCharCallback)
		{
			mCharCallback(chr);
		}
	}
	
	void Win32Window::triggerMouseUpEvent(int button)
	{
		if (mMouseUpCallback)
		{
			mMouseUpCallback(cursorPosition.x, cursorPosition.y, button);
		}
	}
	
	void Win32Window::triggerMouseDownEvent(int button)
	{
		if (mMouseDownCallback)
		{
			mMouseDownCallback(cursorPosition.x, cursorPosition.y, button);
		}
	}
	
	void Win32Window::triggerMouseMoveEvent()
	{
		if (mMouseMoveCallback)
		{
			mMouseMoveCallback(cursorPosition.x, cursorPosition.y);
		}
	}
	
	void Win32Window::triggerFocusCallback(bool focused)
	{
		if (mFocusCallback)
		{
			mFocusCallback(focused);
		}
	}
	
	void Win32Window::enterMainLoop()
	{
		float time = globalTimer.getTime();
		float t;
		bool cvisible = cursorVisible;
#ifdef _DEBUG
		static float fpsTimer = globalTimer.getTime();
		static float fps = 0;
#endif
		POINT w32_cursorPosition;
		float k;
		while (mRunning)
		{
			// mouse position
			GetCursorPos(&w32_cursorPosition);
			ScreenToClient(hWnd, &w32_cursorPosition);
			cursorPosition.set(w32_cursorPosition.x, w32_cursorPosition.y);
			doEvents();
			t = globalTimer.getTime();

			if (t == time)
			{
				continue; // don't redraw frames which won't change
			}
			k = (t - time) / 1000.0f;
			if (k > 0.5f)
			{
				k = 0.05f; // prevent jumps. from eg, waiting on device reset or super low framerate
			}
			time = t;
			if (!mActive)
			{
				k = 0;
				for (int i = 0; i < 5; i++)
				{
					doEvents();
					Sleep(40);
				}
			}
			// rendering
			//d3dDevice->BeginScene();
			if (mUpdateCallback)
			{
				mUpdateCallback(k);
			}
#ifdef _DEBUG
			if (time - fpsTimer > 1000)
			{
				sprintf(fpstitle, " [FPS: %d]", fps);
				setWindowTitle(mTitle);
				fps = 0;
				fpsTimer = time;
			}
			else
			{
				fps++;
			}
#endif			
			//d3dDevice->EndScene();
			rendersys->presentFrame();
		}
	}
	
	void* Win32Window::getIDFromBackend()
	{
		return hWnd;
	}

}
