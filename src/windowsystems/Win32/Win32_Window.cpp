/// @file
/// @version 4.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifdef _WIN32_WINDOW
#define __HL_INCLUDE_PLATFORM_HEADERS
#include <hltypes/hltypesUtil.h>
#include <hltypes/hlog.h>
#include <hltypes/hplatform.h>
#include <hltypes/hresource.h>
#include <hltypes/hthread.h>

#include "april.h"
#include "Platform.h"
#include "RenderSystem.h"
#include "SystemDelegate.h"
#include "Timer.h"
#include "Win32_Cursor.h"
#include "Win32_Window.h"

#ifdef _OPENGL
#include "OpenGL_RenderSystem.h"
#endif
#ifdef _EGL
#include "egl.h"
#endif

#define MOUSEEVENTF_FROMTOUCH 0xFF515700
#define APRIL_WIN32_WINDOW_CLASS L"AprilWin32Window"
#define STYLE_FULLSCREEN WS_POPUP
#define STYLE_WINDOWED (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX)
#define EXSTYLE_FULLSCREEN WS_EX_TOPMOST
#define EXSTYLE_WINDOWED WS_EX_LEFT

namespace april
{
	// this workaround is required to properly support WinXP
	typedef BOOL(WINAPI *_GetTouchInputInfo)(__in HTOUCHINPUT hTouchInput, __in UINT cInputs, __out_ecount(cInputs) PTOUCHINPUT pInputs, __in int cbSize);
	typedef BOOL(WINAPI *_CloseTouchInputHandle)(__in HTOUCHINPUT hTouchInput);
	typedef BOOL(WINAPI *_RegisterTouchWindow)(__in HWND hwnd, __in ULONG ulFlags);

	static _GetTouchInputInfo _getTouchInputInfo = NULL;
	static _CloseTouchInputHandle _closeTouchInputHandle = NULL;
	static _RegisterTouchWindow _registerTouchWindow = NULL;

	Win32_Window::Win32_Window() : Window()
	{
		this->name = APRIL_WS_WIN32;
		this->hWnd = NULL;
		this->defaultCursor = LoadCursor(0, IDC_ARROW);
		this->cursorExtensions += ".ani";
		this->cursorExtensions += ".cur";
	}

	Win32_Window::~Win32_Window()
	{
		this->destroy();
		DestroyCursor(this->defaultCursor);
	}

	// Considering that Iron Man and Batman's only real superpower is being
	// super rich and smart, Bill Gates turned out to be a real disappointment.
	bool Win32_Window::create(int w, int h, bool fullscreen, chstr title, Window::Options options)
	{
		if (!Window::create(w, h, fullscreen, title, options))
		{
			return false;
		}
		this->inputMode = MOUSE;
		// Win32
		WNDCLASSEXW wc;
		memset(&wc, 0, sizeof(WNDCLASSEX));
		HINSTANCE hinst = GetModuleHandle(0);
		wc.cbSize = sizeof(WNDCLASSEX);
		wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
		wc.lpfnWndProc = &Win32_Window::_mainProcessCallback;
		wc.hInstance = hinst;
		wc.hCursor = this->defaultCursor;
		wc.lpszClassName = APRIL_WIN32_WINDOW_CLASS;
		wc.hIcon = (HICON)LoadImage(hinst, MAKEINTRESOURCE(1), IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR);
		wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
		RegisterClassExW(&wc);
		// determine position
		int screenWidth = GetSystemMetrics(SM_CXSCREEN);
		int screenHeight = GetSystemMetrics(SM_CYSCREEN);
		int x = 0;
		int y = 0;
		if (!this->fullscreen)
		{
			x = (screenWidth - w) / 2;
			y = (screenHeight - h) / 2;
		}
		// setting the necessary styles
		DWORD style = 0;
		DWORD exstyle = 0;
		this->_setupStyles(style, exstyle, this->fullscreen);
		if (!this->fullscreen)
		{
			this->_adjustWindowSizeForClient(x, y, w, h, style, exstyle);
			x = (screenWidth - w) / 2;
			y = (screenHeight - h) / 2;
		}
		// create window
		this->hWnd = CreateWindowExW(exstyle, APRIL_WIN32_WINDOW_CLASS, this->title.wStr().c_str(), style, x, y, w, h, NULL, NULL, hinst, NULL);
		// this workaround is required to properly support WinXP
		if (_getTouchInputInfo == NULL && _closeTouchInputHandle == NULL && _registerTouchWindow == NULL)
		{
			HMODULE user32Dll = LoadLibraryW(L"user32.dll");
			if (user32Dll != NULL)
			{
				_getTouchInputInfo = (_GetTouchInputInfo)GetProcAddress(user32Dll, "GetTouchInputInfo");
				_closeTouchInputHandle = (_CloseTouchInputHandle)GetProcAddress(user32Dll, "CloseTouchInputHandle");
				_registerTouchWindow = (_RegisterTouchWindow)GetProcAddress(user32Dll, "RegisterTouchWindow");
				if (_registerTouchWindow != NULL)
				{
					_registerTouchWindow(this->hWnd, TWF_WANTPALM);
				}
			}
		}
		// display the window on the screen
		ShowWindow(this->hWnd, SW_SHOWNORMAL);
		UpdateWindow(this->hWnd);
		SetCursor(wc.hCursor);
		this->setCursorVisible(true);
		this->fpsCounter = options.fpsCounter;
#ifdef _EGL
		april::egl->create();
#endif
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
#ifdef _EGL
		april::egl->destroy();
#endif
		return true;
	}

	void Win32_Window::setTitle(chstr title)
	{
		if (this->fpsCounter)
		{
			hstr t = title + hsprintf(" [FPS: %d]", this->fps);
			// optimization to prevent setting title every frame
			if (t == this->fpsTitle) return;
			this->fpsTitle = t;
			SetWindowTextW(this->hWnd, t.wStr().c_str());
		}
		else
		{
			SetWindowTextW(this->hWnd, title.wStr().c_str());
		}
		this->title = title;
	}
	
	bool Win32_Window::isCursorVisible()
	{
		return (Window::isCursorVisible() || !this->isCursorInside());
	}
	
	void Win32_Window::_refreshCursor()
	{
		HCURSOR cursor = NULL;
		if (this->isCursorVisible())
		{
			if (this->cursor != NULL)
			{
				cursor = ((Win32_Cursor*)this->cursor)->getCursor();
			}
			if (cursor == NULL)
			{
				cursor = this->defaultCursor;
			}
		}
		SetCursor(cursor);
	}

	int Win32_Window::getWidth()
	{
		RECT rect;
		GetClientRect(this->hWnd, &rect);
		return (rect.right - rect.left);
	}
	
	int Win32_Window::getHeight()
	{
		RECT rect;
		GetClientRect(this->hWnd, &rect);
		return (rect.bottom - rect.top);
	}

	void* Win32_Window::getBackendId()
	{
		return this->hWnd;
	}

	HCURSOR Win32_Window::getCursorHandle()
	{
		if (this->cursor != NULL)
		{
			return ((Win32_Cursor*)this->cursor)->getCursor();
		}
		return this->defaultCursor;
	}

	void Win32_Window::setResolution(int w, int h, bool fullscreen)
	{
		if (this->fullscreen == fullscreen && this->getWidth() == w && this->getHeight() == h)
		{
			return;
		}
		// do NOT change the order the following function calls or else dragons
		// setting the necessary styles
		DWORD style = 0;
		DWORD exstyle = 0;
		this->_setupStyles(style, exstyle, fullscreen);
		SetWindowLongPtr(this->hWnd, GWL_EXSTYLE, exstyle);
		SetWindowLongPtr(this->hWnd, GWL_STYLE, style);
		// changing display settings
		if (fullscreen)
		{
			DEVMODE deviceMode;
			deviceMode.dmPelsWidth = w;
			deviceMode.dmPelsWidth = h;
			deviceMode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;
			ChangeDisplaySettings(&deviceMode, CDS_FULLSCREEN);
		}
		if (this->fullscreen || fullscreen)
		{
			ChangeDisplaySettings(NULL, CDS_RESET);
		}
		// window positions
		int x = 0;
		int y = 0;
		if (!fullscreen)
		{
			if (this->fullscreen)
			{
				x = (GetSystemMetrics(SM_CXSCREEN) - w) / 2;
				y = (GetSystemMetrics(SM_CYSCREEN) - h) / 2;
			}
			else
			{
				RECT rect;
				GetWindowRect(this->hWnd, &rect);
				x = rect.left;
				y = rect.top;
			}
		}
		// updating/executing all remaining changes
		if (!fullscreen)
		{
			this->_adjustWindowSizeForClient(x, y, w, h, style, exstyle);
		}
		SetWindowPos(this->hWnd, (fullscreen ? HWND_TOPMOST : HWND_NOTOPMOST), x, y, w, h, 0);
		ShowWindow(this->hWnd, SW_SHOW);
		UpdateWindow(this->hWnd);
		this->fullscreen = fullscreen;
		this->_setRenderSystemResolution(this->getWidth(), this->getHeight(), this->fullscreen);
	}
	
	bool Win32_Window::updateOneFrame()
	{
		if (this->inputMode != InputMode::TOUCH)
		{
			// Win32 mouse position
			static POINT w32_cursorPosition;
			GetCursorPos(&w32_cursorPosition);
			ScreenToClient(this->hWnd, &w32_cursorPosition);
			this->cursorPosition.set((float)w32_cursorPosition.x, (float)w32_cursorPosition.y);
		}
		this->checkEvents();
		// rendering
		bool result = Window::updateOneFrame();
		if (this->fpsCounter)
		{
			this->setTitle(this->title); // has to come after Window::updateOneFrame(), otherwise FPS value in title would be late one frame
		}
		return result;
	}
	
	void Win32_Window::presentFrame()
	{
#ifdef _OPENGL
		harray<hstr> renderSystems;
		renderSystems += APRIL_RS_OPENGL1;
		renderSystems += APRIL_RS_OPENGLES1;
		renderSystems += APRIL_RS_OPENGLES2;
		if (renderSystems.has(april::rendersys->getName()))
		{
			SwapBuffers(((OpenGL_RenderSystem*)april::rendersys)->getHDC());
		}
#endif
	}
	
	void Win32_Window::checkEvents()
	{
		MSG msg;
		// limiting to 100 events per frame just to be safe
		for_iter (i, 0, 100)
		{
			if (!PeekMessageW(&msg, this->hWnd, 0, 0, PM_REMOVE))
			{
				break;
			}
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
		Window::checkEvents();
	}

	Cursor* Win32_Window::_createCursor()
	{
		return new Win32_Cursor();
	}

	void Win32_Window::_setupStyles(DWORD& style, DWORD& exstyle, bool fullscreen)
	{
		style = STYLE_WINDOWED;
		if (fullscreen)
		{
			style = STYLE_FULLSCREEN;
		}
		else if (this->options.resizable)
		{
			style |= WS_SIZEBOX | WS_MAXIMIZEBOX;
		}
		exstyle = (fullscreen ? EXSTYLE_FULLSCREEN : EXSTYLE_WINDOWED);
	}

	void Win32_Window::_adjustWindowSizeForClient(int x, int y, int& w, int& h, DWORD style, DWORD exstyle)
	{
		RECT rect;
		rect.left = x;
		rect.top = y;
		rect.right = x + w;
		rect.bottom = y + h;
		AdjustWindowRectEx(&rect, style, FALSE, exstyle);
		w = rect.right - rect.left;
		h = rect.bottom - rect.top;
	}

	void Win32_Window::beginKeyboardHandling()
	{
#ifdef _DEBUG
		//this->handleVirtualKeyboardChangeEvent(true, 0.5f); // usually only used for testing
#endif
	}

	void Win32_Window::terminateKeyboardHandling()
	{
#ifdef _DEBUG
		//this->handleVirtualKeyboardChangeEvent(false, 0.0f); // usually only used for testing
#endif
	}

	LRESULT CALLBACK Win32_Window::_mainProcessCallback(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		if (!april::window->isCreated()) // don't run callback processing if window was "destroyed"
		{
			return 1;
		}
		static bool _sizeChanging = false;
		static bool _initialSize = true;
		switch (message)
		{
		case WM_ENTERSIZEMOVE:
			_sizeChanging = true;
			break;
		case WM_EXITSIZEMOVE:
			_sizeChanging = false;
			break;
		case WM_SIZE:
			if (!april::window->isFullscreen() &&
				(_sizeChanging || wParam == SIZE_MAXIMIZED || wParam == SIZE_RESTORED && !_initialSize))
			{
				((Win32_Window*)april::window)->_setRenderSystemResolution();
				UpdateWindow(hWnd);
				april::window->performUpdate(0.0f);
				april::rendersys->presentFrame();
			}
			_initialSize = false;
			break;
		}
		return Win32_Window::childProcessCallback(hWnd, message, wParam, lParam);
	}

	LRESULT CALLBACK Win32_Window::childProcessCallback(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		if (!april::window->isCreated()) // don't run callback processing if window was "destroyed"
		{
			return 1;
		}
		static float _wheelDelta = 0.0f;
		static bool _altKeyDown = false;
		static int lastWidth = april::window->getWidth();
		static int lastHeight = april::window->getHeight();
		static TOUCHINPUT touches[100];
		static POINT w32_cursorPosition;
		static int _mouseMessages = 0;
		switch (message)
		{
		case WM_TOUCH: // (Win7+ only)
			if (wParam > 0 && _getTouchInputInfo != NULL && _closeTouchInputHandle != NULL)
			{
				april::window->setInputMode(april::Window::TOUCH);
				_mouseMessages = 5;
				if (_getTouchInputInfo((HTOUCHINPUT)lParam, wParam, touches, sizeof(TOUCHINPUT)))
				{
					w32_cursorPosition.x = TOUCH_COORD_TO_PIXEL(touches[0].x);
					w32_cursorPosition.y = TOUCH_COORD_TO_PIXEL(touches[0].y);
					ScreenToClient(hWnd, &w32_cursorPosition);
					((Win32_Window*)april::window)->cursorPosition.set((float)w32_cursorPosition.x, (float)w32_cursorPosition.y);
					if ((touches[0].dwFlags & TOUCHEVENTF_DOWN) == TOUCHEVENTF_DOWN)
					{
						if (!april::window->isFullscreen())
						{
							SetCapture((HWND)april::window->getBackendId());
						}
						april::window->queueMouseEvent(MOUSE_DOWN, april::window->getCursorPosition(), AK_LBUTTON);
					}
					else if ((touches[0].dwFlags & TOUCHEVENTF_UP) == TOUCHEVENTF_UP)
					{
						if (!april::window->isFullscreen())
						{
							ReleaseCapture();
						}
						april::window->queueMouseEvent(MOUSE_UP, april::window->getCursorPosition(), AK_LBUTTON);
					}
					else if ((touches[0].dwFlags & TOUCHEVENTF_MOVE) == TOUCHEVENTF_MOVE)
					{
						april::window->queueMouseEvent(MOUSE_MOVE, april::window->getCursorPosition(), AK_NONE);
					}
				}
				_closeTouchInputHandle((HTOUCHINPUT)lParam);
			}
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			april::window->terminateMainLoop();
			break;
		case WM_CLOSE:
			if (april::window->handleQuitRequest(true))
			{
				PostQuitMessage(0);
				april::window->terminateMainLoop();
			}
			return 0;
		case WM_SYSKEYDOWN:
			if (wParam == VK_MENU)
			{
				_altKeyDown = true;
			}
			// no break here, because this is still an input message that needs to be processed normally
		case WM_KEYDOWN:
			if (_altKeyDown)
			{
				if (wParam == VK_F4)
				{
					if (april::window->handleQuitRequest(true))
					{
						PostQuitMessage(0);
						april::window->terminateMainLoop();
					}
					return 0;
				}
				if (wParam == VK_RETURN)
				{
					april::window->toggleHotkeyFullscreen();
					return 0;
				}
			}
			april::window->queueKeyEvent(KEY_DOWN, (april::Key)wParam, 0);
			return 0;
		case WM_SYSKEYUP:
			if (wParam == VK_MENU || wParam == VK_RETURN || wParam == VK_F4)
			{
				_altKeyDown = false;
			}
			// no break here, because this is still an input message that needs to be processed normally
		case WM_KEYUP:
			april::window->queueKeyEvent(KEY_UP, (april::Key)wParam, 0);
			return 0;
		case WM_CHAR:
			april::window->queueKeyEvent(KEY_DOWN, AK_NONE, wParam);
			break;
		case WM_LBUTTONDOWN:
			if ((GetMessageExtraInfo() & MOUSEEVENTF_FROMTOUCH) != MOUSEEVENTF_FROMTOUCH)
			{
				_mouseMessages = 0;
				if (!april::window->isFullscreen())
				{
					SetCapture((HWND)april::window->getBackendId());
				}
				april::window->setInputMode(april::Window::MOUSE);
				april::window->queueMouseEvent(MOUSE_DOWN, april::window->getCursorPosition(), AK_LBUTTON);
			}
			break;
		case WM_RBUTTONDOWN:
			if ((GetMessageExtraInfo() & MOUSEEVENTF_FROMTOUCH) != MOUSEEVENTF_FROMTOUCH)
			{
				_mouseMessages = 0;
				if (!april::window->isFullscreen())
				{
					SetCapture((HWND)april::window->getBackendId());
				}
				april::window->setInputMode(april::Window::MOUSE);
				april::window->queueMouseEvent(MOUSE_DOWN, april::window->getCursorPosition(), AK_RBUTTON);
			}
			break;
		case WM_LBUTTONUP:
			if ((GetMessageExtraInfo() & MOUSEEVENTF_FROMTOUCH) != MOUSEEVENTF_FROMTOUCH)
			{
				_mouseMessages = 0;
				if (!april::window->isFullscreen())
				{
					ReleaseCapture();
				}
				april::window->setInputMode(april::Window::MOUSE);
				april::window->queueMouseEvent(MOUSE_UP, april::window->getCursorPosition(), AK_LBUTTON);
			}
			break;
		case WM_RBUTTONUP:
			if ((GetMessageExtraInfo() & MOUSEEVENTF_FROMTOUCH) != MOUSEEVENTF_FROMTOUCH)
			{
				_mouseMessages = 0;
				if (!april::window->isFullscreen())
				{
					ReleaseCapture();
				}
				april::window->setInputMode(april::Window::MOUSE);
				april::window->queueMouseEvent(MOUSE_UP, april::window->getCursorPosition(), AK_RBUTTON);
			}
			break;
		case WM_MOUSEMOVE:
			if ((GetMessageExtraInfo() & MOUSEEVENTF_FROMTOUCH) != MOUSEEVENTF_FROMTOUCH)
			{
				if (_mouseMessages > 0)
				{
					--_mouseMessages;
				}
				if (_mouseMessages == 0)
				{
					april::window->setInputMode(april::Window::MOUSE);
					april::window->queueMouseEvent(MOUSE_MOVE, april::window->getCursorPosition(), AK_NONE);
				}
			}
			break;
		case WM_MOUSEWHEEL:
			if ((GetMessageExtraInfo() & MOUSEEVENTF_FROMTOUCH) != MOUSEEVENTF_FROMTOUCH)
			{
				_wheelDelta = (float)GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
				if ((GET_KEYSTATE_WPARAM(wParam) & MK_CONTROL) != MK_CONTROL)
				{
					april::window->queueMouseEvent(MOUSE_SCROLL, gvec2(0.0f, -(float)_wheelDelta), AK_NONE);
				}
				else
				{
					april::window->queueMouseEvent(MOUSE_SCROLL, gvec2(-(float)_wheelDelta, 0.0f), AK_NONE);
				}
			}
			break;
		case WM_MOUSEHWHEEL:
			if ((GetMessageExtraInfo() & MOUSEEVENTF_FROMTOUCH) != MOUSEEVENTF_FROMTOUCH)
			{
				_wheelDelta = (float)GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
				if ((GET_KEYSTATE_WPARAM(wParam) & MK_CONTROL) != MK_CONTROL)
				{
					april::window->queueMouseEvent(MOUSE_SCROLL, gvec2(-(float)_wheelDelta, 0.0f), AK_NONE);
				}
				else
				{
					april::window->queueMouseEvent(MOUSE_SCROLL, gvec2(0.0f, -(float)_wheelDelta), AK_NONE);
				}
			}
			break;
		case WM_SETCURSOR:
			if (!april::window->isCursorVisible())
			{
				SetCursor(NULL);
				return 1;
			}
			if (april::window->isCursorInside())
			{
				HCURSOR cursor = ((Win32_Window*)april::window)->getCursorHandle();
				if (cursor != GetCursor())
				{
					SetCursor(cursor);
				}
				if (cursor != NULL)
				{
					return 1;
				}
			}
			break;
		case WM_ACTIVATE:
			if ((wParam & 0xFFFF) != 0)
			{
				if ((wParam & 0xFFFF0000) == 0) // only respond to activation if window is not minimized
				{
					april::window->handleFocusChangeEvent(true);
				}
			}
			else
			{
				_altKeyDown = false; // because ALT keyup isn't processed when alt-tab-ing for some reason.
				april::window->handleFocusChangeEvent(false);
			}
			break;
		}
		return DefWindowProcW(hWnd, message, wParam, lParam);
	}

}
#endif
