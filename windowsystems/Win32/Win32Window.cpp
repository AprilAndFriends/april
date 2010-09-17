/************************************************************************************\
This source file is part of the Awesome Portable Rendering Interface Library         *
For latest info, see http://libapril.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes (kreso@cateia.com)                                  *
Copyright (c) 2010 Ivan Vucica (ivan@vucica.net)                                     *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/

#include "Win32Window.h"
#include <windows.h>
#include "RenderSystem.h"
#include "Timer.h"

#include <hltypes/util.h>

namespace April
{
	static HWND hWnd;
	static gtypes::Vector2 cursorpos;
	static bool cursor_visible=1;
	static bool window_active=1,wnd_fullscreen=0;
	static April::Timer globalTimer;
	static Win32Window* instance;
	
#ifdef _DEBUG
	static char fpstitle[1024]=" [FPS:0]";
#endif
	
	void doWindowEvents()
	{
		MSG msg;
		if (PeekMessage(&msg,hWnd,0,0,PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
/**********************************************************************************************/
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	Win32Window *ws=(Win32Window*) instance;
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
#ifdef _DEBUG //2DO - should be removed completely
		    if (wParam == VK_ESCAPE) { ws->terminateMainLoop(); return 0; }
#endif
			ws->triggerKeyEvent(1,wParam);
			break;
		case WM_KEYUP: 
			ws->triggerKeyEvent(0,wParam);
			break;
		case WM_CHAR:
			ws->triggerCharEvent(wParam);
			break;
		case WM_LBUTTONDOWN:
			ws->triggerMouseDownEvent(AK_LBUTTON);
			if (!wnd_fullscreen) SetCapture(hWnd);
			break;
		case WM_RBUTTONDOWN:
			ws->triggerMouseDownEvent(AK_RBUTTON);
			if (!wnd_fullscreen) SetCapture(hWnd);
			break;
		case WM_LBUTTONUP:
			ws->triggerMouseUpEvent(AK_LBUTTON);
			if (!wnd_fullscreen) ReleaseCapture();
			break;
		case WM_RBUTTONUP:
			ws->triggerMouseUpEvent(AK_RBUTTON);
			if (!wnd_fullscreen) ReleaseCapture();
			break;
		case WM_MOUSEMOVE:
			ws->triggerMouseMoveEvent();break;
		case WM_SETCURSOR:
			return 1;
		case WM_ACTIVATE:
			if (wParam == WA_ACTIVE || wParam == WA_CLICKACTIVE)
			{
				window_active=1;
				rendersys->logMessage("Window activated");
			}
			else
			{
				window_active=0;
				rendersys->logMessage("Window deactivated");
			}
			break;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}
/**********************************************************************************************/
	Win32Window::Win32Window(int w,int h,bool fullscreen,chstr title) //:
		/*mTexCoordsEnabled(0), mColorEnabled(0), RenderSystem()*/
	{
		if(rendersys) rendersys->logMessage("Creating Win32 Windowsystem");
		
		instance = this;
		
		mRunning=true;
		wnd_fullscreen=fullscreen;
		// WINDOW
		mTitle=title;
		WNDCLASSEX wc;
		ZeroMemory(&wc, sizeof(WNDCLASSEX));
		
		HINSTANCE hinst=GetModuleHandle(0);
		wc.cbSize = sizeof(WNDCLASSEX);
		wc.style = CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc = WindowProc;
		wc.hInstance = hinst;
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
		wc.lpszClassName = "april_win32_window";
		wc.hIcon=(HICON) LoadImage(0,"game.ico",IMAGE_ICON,0,0,LR_LOADFROMFILE);
		wc.hbrBackground=(HBRUSH)GetStockObject(BLACK_BRUSH);
		
		RegisterClassEx(&wc);
		int x=(fullscreen) ? 0 : (GetSystemMetrics(SM_CXSCREEN)-w)/2,
			y=(fullscreen) ? 0 : (GetSystemMetrics(SM_CYSCREEN)-h)/2;
		
		DWORD style=(fullscreen) ? WS_EX_TOPMOST|WS_POPUP : WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX;
		printf("title: %s %d %d %d %d inst: %d\n", title.c_str(), x, y, w, h, hinst);
		hWnd = CreateWindowEx(0,"april_win32_window",title.c_str(),style,x,y,w,h,NULL,NULL,hinst,NULL);
		
		if (!fullscreen)
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
		
		SetCursor(LoadCursor(0,IDC_ARROW));
	}
	
	Win32Window::~Win32Window()
	{
		//logMessage("Destroying Win32 Windowsystem");
		UnregisterClass("april_win32_window",GetModuleHandle(0));
	}

	/*hstr Win32Window::getName()
	{
		return "DirectX9";
	}*/
	
	
	
	int Win32Window::getWindowWidth()
	{
		RECT rc;
		GetClientRect(hWnd, &rc);
		return rc.right-rc.left;
	}
	
	int Win32Window::getWindowHeight()
	{
		RECT rc;
		GetClientRect(hWnd, &rc);
		return rc.bottom-rc.top;
	}


	void Win32Window::setWindowTitle(chstr title)
	{
		mTitle=title;
#ifdef _DEBUG
		SetWindowText(hWnd,(title+fpstitle).c_str());
#else
		SetWindowText(hWnd,title.c_str());
#endif
	}
	
	gtypes::Vector2 Win32Window::getCursorPos()
	{
		
		return gtypes::Vector2(cursorpos.x,cursorpos.y);
	}

	void Win32Window::showSystemCursor(bool b)
	{
		if (b) SetCursor(LoadCursor(0,IDC_ARROW));
		else   SetCursor(0);
		cursor_visible=b;
	}
	
	bool Win32Window::isSystemCursorShown()
	{
		return cursor_visible;
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
		if(mQuitCallback)
		{
			return mQuitCallback(true);
		}
		return true;
	}
	
	void Win32Window::triggerKeyEvent(bool down,unsigned int keycode)
	{
		if (down)
		{ if (mKeyDownCallback) mKeyDownCallback(keycode); }
		else
		{ if (mKeyUpCallback) mKeyUpCallback(keycode); }
	}
	
	void Win32Window::triggerCharEvent(unsigned int chr)
	{
		if (mCharCallback) mCharCallback(chr);
	}
	
	void Win32Window::triggerMouseUpEvent(int button)
	{
		if (mMouseUpCallback) mMouseUpCallback(cursorpos.x,cursorpos.y,button);
	}
	
	void Win32Window::triggerMouseDownEvent(int button)
	{
		if (mMouseDownCallback) mMouseDownCallback(cursorpos.x,cursorpos.y,button);
	}
	
	void Win32Window::triggerMouseMoveEvent()
	{
		if (mMouseMoveCallback) mMouseMoveCallback(cursorpos.x,cursorpos.y);
	}
	
	void Win32Window::enterMainLoop()
	{
		float time=globalTimer.getTime(),t;
		bool cvisible=cursor_visible;
#ifdef _DEBUG
		static float fpsTimer=globalTimer.getTime(),fps=0;
#endif
		POINT w32_cursorpos;
		float k;
		while (mRunning)
		{
			// mouse position
			GetCursorPos(&w32_cursorpos);
			ScreenToClient(hWnd,&w32_cursorpos);
			cursorpos.set(w32_cursorpos.x,w32_cursorpos.y);
			if (!cursor_visible && !wnd_fullscreen)
			{
				if (cursorpos.y < 0)
				{
					if (!cvisible) { cvisible=1; SetCursor(LoadCursor(0,IDC_ARROW)); }
				}
				else
				{
					if (cvisible) { cvisible=0; SetCursor(0); }
				}
			}
			doWindowEvents();
			t=globalTimer.getTime();

			if (t == time) continue; // don't redraw frames which won't change
			k=(t-time)/1000.0f;
			if (k > 0.5f) k=0.05f; // prevent jumps. from eg, waiting on device reset or super low framerate
			time=t;
			if (!window_active)
			{
				k=0;
				for (int i=0;i<5;i++)
				{
					doWindowEvents();
					Sleep(40);
				}
			}
			// rendering
			//d3dDevice->BeginScene();
			if (mUpdateCallback) mUpdateCallback(k);
			
#ifdef _DEBUG
			if (time-fpsTimer > 1000)
			{
				sprintf(fpstitle," [FPS: %d]",fps);
				setWindowTitle(mTitle);
				fps=0;
				fpsTimer=time;
			}
			else fps++;
#endif			
			//d3dDevice->EndScene();


			rendersys->presentFrame();
		}
	}



/*

	gtypes::Vector2 getDesktopResolution()
	{
		return gtypes::Vector2(GetSystemMetrics(SM_CXSCREEN),GetSystemMetrics(SM_CYSCREEN));
	}
	*/
	void* Win32Window::getIDFromBackend()
	{
		return hWnd;
	}

}
