/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @author  Ivan Vucica
/// @version 1.33
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _WIN32
#include <windows.h>

#include <hltypes/hltypesUtil.h>
#include <hltypes/hthread.h>

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

#ifdef _OPENGL
	extern HDC hDC;
#endif
	
#ifdef _DEBUG
	hstr fpsTitle = " [FPS: 0]";
#endif
/************************************************************************************/
	LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		static bool touchDown = false;
		static bool doubleTapDown = false;
		static int nMouseMoveMessages = 0;
		Win32Window *ws = (Win32Window*) instance;
		switch (message)
		{
		case 0x0119: // WM_GESTURE (win7+ only)
			if (wParam == 1) // GID_BEGIN
			{
				touchDown = true;
			}
			else if (wParam == 2) // GID_END
			{
				if (doubleTapDown)
				{ 
					doubleTapDown = false;
					ws->triggerMouseUpEvent(AK_DOUBLETAP);
				}
				touchDown = false;
			}
			else if (wParam == 6) // GID_TWOFINGERTAP
			{
				doubleTapDown = true;
				ws->triggerMouseDownEvent(AK_DOUBLETAP);
			}
			break;
		case 0x011A: // WM_GESTURENOTIFY (win7+ only)
			touchDown = true;
			ws->triggerTouchscreenCallback(true);
			break;
		case WM_DESTROY:
		case WM_CLOSE:
			if (ws->triggerQuitEvent())
			{
				PostQuitMessage(0);
				ws->terminateMainLoop();
			}
			return 0;
			break;
		case WM_KEYDOWN:
			ws->triggerKeyEvent(true, wParam);
			break;
		case WM_KEYUP: 
			ws->triggerKeyEvent(false, wParam);
			break;
		case WM_CHAR:
			ws->triggerCharEvent(wParam);
			break;
		case WM_LBUTTONDOWN:
			touchDown = true;
			nMouseMoveMessages = 0;
			ws->triggerMouseDownEvent(AK_LBUTTON);
			if (!instance->isFullscreen())
			{
				SetCapture(hWnd);
			}
			break;
		case WM_RBUTTONDOWN:
			touchDown = true;
			nMouseMoveMessages = 0;
			ws->triggerMouseDownEvent(AK_RBUTTON);
			if (!instance->isFullscreen())
			{
				SetCapture(hWnd);
			}
			break;
		case WM_LBUTTONUP:
			touchDown = false;
			ws->triggerMouseUpEvent(AK_LBUTTON);
			if (!instance->isFullscreen())
			{
				ReleaseCapture();
			}
			break;
		case WM_RBUTTONUP:
			touchDown = false;
			ws->triggerMouseUpEvent(AK_RBUTTON);
			if (!instance->isFullscreen())
			{
				ReleaseCapture();
			}
			break;
		case WM_MOUSEMOVE:
			if (!touchDown)
			{
				if (nMouseMoveMessages >= 10)
				{
					ws->triggerTouchscreenCallback(false);
				}
				else
				{
					nMouseMoveMessages++;
				}
			}
			else
			{
				nMouseMoveMessages = 0;
			}
			ws->triggerMouseMoveEvent();
			break;
		case WM_SETCURSOR:
			if (!cursorVisible)
			{
				if (is_between(cursorPosition.x, 0.0f, (float)ws->getWidth()) && is_between(cursorPosition.y, 0.0f, (float)ws->getHeight()))
				{
					SetCursor(0);
				}
				else
				{
					SetCursor(LoadCursor(0, IDC_ARROW));
				}
			}
			return 1;
		case WM_ACTIVATE:
			if (wParam == WA_ACTIVE || wParam == WA_CLICKACTIVE)
			{
				instance->_setActive(true);
				ws->triggerFocusCallback(true);
				april::log("Window activated");
			}
			else
			{
				instance->_setActive(false);
				ws->triggerFocusCallback(false);
				april::log("Window deactivated");
			}
			break;
		}
		return DefWindowProcW(hWnd, message, wParam, lParam);
	}
/************************************************************************************/
	Win32Window::Win32Window(int w, int h, bool fullscreen, chstr title) : Window()
	{
		if (april::rendersys != NULL)
		{
			april::log("Creating Win32 Windowsystem");
		}
		instance = this;
		
		mRunning = mActive = true;
		mFullscreen = fullscreen;
		mTouchEnabled = false;
		// WINDOW
		mTitle = title;
		WNDCLASSEXW wc;
		ZeroMemory(&wc, sizeof(WNDCLASSEX));
		
		HINSTANCE hinst = GetModuleHandle(0);
		wc.cbSize = sizeof(WNDCLASSEX);
		wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
		wc.lpfnWndProc = WindowProc;
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
			x = (GetSystemMetrics(SM_CXSCREEN) - w) / 2;
			y = (GetSystemMetrics(SM_CYSCREEN) - h) / 2;
		}
		
		WCHAR wtitle[256] = {0};
		for (int i = 0; i < title.size(); i++) wtitle[i] = title[i];

		DWORD style = (fullscreen ? WS_EX_TOPMOST | WS_POPUP : WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX);
		april::log(hsprintf("title: %s %d %d %d %d inst: %d", title.c_str(), x, y, w, h, hinst));
		hWnd = CreateWindowExW(0, L"april_win32_window", wtitle, style, x, y, w, h, NULL, NULL, hinst, NULL);
		
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
		SetCursor(wc.hCursor);
	}
	
	Win32Window::~Win32Window()
	{
		//log("Destroying Win32 Windowsystem");
	}

	void Win32Window::destroyWindow()
	{
		if (hWnd != 0)
		{
			DestroyWindow(hWnd);
			UnregisterClassW(L"april_win32_window", GetModuleHandle(0));
			hWnd = 0;
		}
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
		mTitle = title;
#ifdef _DEBUG
		hstr t = title + fpsTitle;
#else
		chstr t = title;
#endif
		WCHAR wtitle[256] = {0};
		for (int i = 0; i < t.size(); i++)
		{
			wtitle[i] = t[i];
		}

		SetWindowTextW(hWnd, wtitle);
	}
	
	void Win32Window::_setResolution(int w, int h)
	{
		int x = 0;
		int y = 0;
		DWORD style = WS_EX_TOPMOST | WS_POPUP;
		if (!mFullscreen)
		{
			x = (GetSystemMetrics(SM_CXSCREEN) - w) / 2,
			y = (GetSystemMetrics(SM_CYSCREEN) - h) / 2;
			style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
		}
		
		if (!mFullscreen)
		{
			RECT rcClient, rcWindow;
			POINT ptDiff;
			GetClientRect(hWnd, &rcClient);
			GetWindowRect(hWnd, &rcWindow);
			ptDiff.x = (rcWindow.right - rcWindow.left) - rcClient.right;
			ptDiff.y = (rcWindow.bottom - rcWindow.top) - rcClient.bottom;
			MoveWindow(hWnd,rcWindow.left, rcWindow.top, w + ptDiff.x, h + ptDiff.y, TRUE);
		}

		// display the window on the screen
		ShowWindow(hWnd, 1);
		UpdateWindow(hWnd);
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
		return !(is_in_range(cursorPosition.x, 0.0f, (float)getWidth()) && is_in_range(cursorPosition.y, 0.0f, (float)getHeight()));
	}
	
	void Win32Window::doEvents()
	{
		MSG msg;
		if (PeekMessageW(&msg, hWnd, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
	}

	void Win32Window::presentFrame()
	{
#ifdef _OPENGL
		SwapBuffers(hDC);
#endif
	}
	
	void Win32Window::terminateMainLoop()
	{
		mRunning = false;
	}
	
	bool Win32Window::triggerQuitEvent()
	{
		if (mQuitCallback != NULL)
		{
			return (*mQuitCallback)(true);
		}
		return true;
	}
	
	void Win32Window::triggerKeyEvent(bool down, unsigned int keycode)
	{
		if (down)
		{
			if (mKeyDownCallback != NULL)
			{
				(*mKeyDownCallback)(keycode);
			}
		}
		else
		{
			if (mKeyUpCallback != NULL)
			{
				(*mKeyUpCallback)(keycode);
			}
		}
	}
	
	void Win32Window::triggerCharEvent(unsigned int chr)
	{
		if (mCharCallback != NULL)
		{
			(*mCharCallback)(chr);
		}
	}
	
	void Win32Window::triggerMouseUpEvent(int button)
	{
		if (mMouseUpCallback != NULL)
		{
			(*mMouseUpCallback)(cursorPosition.x, cursorPosition.y, button);
		}
	}
	
	void Win32Window::triggerMouseDownEvent(int button)
	{
		if (mMouseDownCallback != NULL)
		{
			(*mMouseDownCallback)(cursorPosition.x, cursorPosition.y, button);
		}
	}
	
	void Win32Window::triggerMouseMoveEvent()
	{
		if (mMouseMoveCallback != NULL)
		{
			(*mMouseMoveCallback)(cursorPosition.x, cursorPosition.y);
		}
	}
	
	void Win32Window::triggerFocusCallback(bool focused)
	{
		if (mFocusCallback != NULL)
		{
			(*mFocusCallback)(focused);
		}
	}
	
	void Win32Window::triggerTouchscreenCallback(bool enabled)
	{
		if (mTouchEnabledCallback != NULL && mTouchEnabled != enabled)
		{
			mTouchEnabled = enabled;
			(*mTouchEnabledCallback)(enabled);
		}
	}
	
	void Win32Window::enterMainLoop()
	{
		float time = globalTimer.getTime();
		float t;
		bool cVisible = cursorVisible;
#ifdef _DEBUG
		static float fpsTimer = globalTimer.getTime();
		static int fps = 0;
#endif
		POINT w32_cursorPosition;
		float k;
		while (mRunning)
		{
			// mouse position
			GetCursorPos(&w32_cursorPosition);
			ScreenToClient(hWnd, &w32_cursorPosition);
			cursorPosition.set((float)w32_cursorPosition.x, (float)w32_cursorPosition.y);
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
					hthread::sleep(40.0f);
				}
			}
			// rendering
			//d3dDevice->BeginScene();
			if (mUpdateCallback != NULL)
			{
				(*mUpdateCallback)(k);
			}
#ifndef _DEBUG
			setWindowTitle(mTitle);
#else
			if (time - fpsTimer > 1000)
			{
				fpsTitle = hsprintf(" [FPS: %d]", fps);
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
			april::rendersys->presentFrame();
		}
	}

	void Win32Window::updateOneFrame()
	{
		// TODO
	}
	
	void* Win32Window::getIDFromBackend()
	{
		return hWnd;
	}

	Window::DeviceType Win32Window::getDeviceType()
	{
		return Window::DEVICE_WINDOWS_PC;
	}

	SystemInfo& getSystemInfo()
	{
		static SystemInfo info;
		if (info.locale == "")
		{
			info.ram = 1024;
			info.locale = "en";
		}
		return info;
	}

}
#endif