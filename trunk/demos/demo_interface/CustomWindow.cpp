/// @file
/// @version 3.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <april/april.h>
#include <april/Keys.h>
#include <april/Platform.h>
#include <april/SystemDelegate.h>
#include <april/Timer.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hlog.h>
#include <hltypes/hplatform.h>
#include <hltypes/hresource.h>
#include <hltypes/hthread.h>

#include "CustomRenderSystem.h"
#include "CustomWindow.h"

#ifdef _OPENGL
#include "OpenGL_RenderSystem.h"
#endif

#define CUSTOM_WINDOW_CLASS L"AprilCustomWindow"
#define STYLE_FULLSCREEN WS_POPUP
#define STYLE_WINDOWED (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX)
#define EXSTYLE_FULLSCREEN WS_EX_TOPMOST
#define EXSTYLE_WINDOWED WS_EX_LEFT

CustomWindow::CustomWindow() : april::Window()
{
	this->name = "Custom";
	this->hWnd = NULL;
	this->cursorExtensions += ".ani";
	this->cursorExtensions += ".cur";
}

CustomWindow::~CustomWindow()
{
	this->destroy();
}

bool CustomWindow::create(int w, int h, bool fullscreen, chstr title, april::Window::Options options)
{
	if (!april::Window::create(w, h, fullscreen, title, options))
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
	wc.lpfnWndProc = &CustomWindow::_processCallback;
	wc.hInstance = hinst;
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.lpszClassName = CUSTOM_WINDOW_CLASS;
	wc.hIcon = 0;
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
	DWORD style = STYLE_WINDOWED;
	if (this->fullscreen)
	{
		style = STYLE_FULLSCREEN;
	}
	else if (this->options.resizable)
	{
		style |= WS_SIZEBOX | WS_MAXIMIZEBOX;
	}
	DWORD exstyle = (this->fullscreen ? EXSTYLE_FULLSCREEN : EXSTYLE_WINDOWED);
	if (!this->fullscreen)
	{
		// takes into account the offset
		RECT rect;
		rect.left = x;
		rect.top = y;
		rect.right = x + w;
		rect.bottom = y + h;
		AdjustWindowRectEx(&rect, style, FALSE, exstyle);
		w = rect.right - rect.left;
		h = rect.bottom - rect.top;
		// centers window
		x = (screenWidth - w) / 2;
		y = (screenHeight - h) / 2;
	}
	// create window
	this->hWnd = CreateWindowExW(exstyle, CUSTOM_WINDOW_CLASS, this->title.w_str().c_str(), style, x, y, w, h, NULL, NULL, hinst, NULL);
	// display the window on the screen
	ShowWindow(this->hWnd, SW_SHOWNORMAL);
	UpdateWindow(this->hWnd);
	SetCursor(wc.hCursor);
	this->setCursorVisible(true);
	return true;
}

bool CustomWindow::destroy()
{
	if (!april::Window::destroy())
	{
		return false;
	}
	if (this->hWnd != 0)
	{
		DestroyWindow(this->hWnd);
		UnregisterClassW(CUSTOM_WINDOW_CLASS, GetModuleHandle(0));
		this->hWnd = 0;
	}
	return true;
}

int CustomWindow::getWidth()
{
	RECT rect;
	GetClientRect(this->hWnd, &rect);
	return (rect.right - rect.left);
}

int CustomWindow::getHeight()
{
	RECT rect;
	GetClientRect(this->hWnd, &rect);
	return (rect.bottom - rect.top);
}

void* CustomWindow::getBackendId()
{
	return this->hWnd;
}

bool CustomWindow::updateOneFrame()
{
	POINT w32_cursorPosition;
	// mouse position
	GetCursorPos(&w32_cursorPosition);
	ScreenToClient(this->hWnd, &w32_cursorPosition);
	this->cursorPosition.set((float)w32_cursorPosition.x, (float)w32_cursorPosition.y);
	this->checkEvents();
	// rendering
	return april::Window::updateOneFrame();
}

void CustomWindow::presentFrame()
{
#ifdef _OPENGL
	harray<hstr> renderSystems;
	renderSystems += APRIL_RS_OPENGL1;
	renderSystems += APRIL_RS_OPENGLES1;
	renderSystems += APRIL_RS_OPENGLES2;
	if (renderSystems.contains(april::rendersys->getName()))
	{
		SwapBuffers(((OpenGL_RenderSystem*)april::rendersys)->getHDC());
	}
#endif
}

void CustomWindow::checkEvents()
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
	april::Window::checkEvents();
}

april::Cursor* CustomWindow::_createCursor()
{
	return NULL; // not supporting cursors in this implementation
}

void CustomWindow::_setupStyles(DWORD& style, DWORD& exstyle, bool fullscreen)
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

void CustomWindow::_adjustWindowSizeForClient(int x, int y, int& w, int& h, DWORD style, DWORD exstyle)
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

LRESULT CALLBACK CustomWindow::_processCallback(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (!april::window->isCreated()) // don't run callback processing if window was "destroyed"
	{
		return 1;
	}
	static bool _touchDown = false;
	static bool _doubleTapDown = false;
	static int _mouseMoveMessagesCount = 0;
	static float _wheelDelta = 0.0f;
	static bool _altKeyDown = false;
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
				april::window->queueMouseEvent(MOUSE_UP, april::window->getCursorPosition(), april::AK_DOUBLETAP);
			}
			_touchDown = false;
		}
		else if (wParam == 6) // GID_TWOFINGERTAP
		{
			_doubleTapDown = true;
			april::window->queueMouseEvent(MOUSE_DOWN, april::window->getCursorPosition(), april::AK_DOUBLETAP);
		}
		break;
	case 0x011A: // WM_GESTURENOTIFY (win7+ only)
		_touchDown = true;
		april::window->setInputMode(april::Window::TOUCH);
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
		if (wParam == VK_MENU)
		{
			_altKeyDown = false;
		}
		// no break here, because this is still an input message that needs to be processed normally
	case WM_KEYUP:
		april::window->queueKeyEvent(KEY_UP, (april::Key)wParam, 0);
		return 0;
	case WM_CHAR:
		april::window->queueKeyEvent(KEY_DOWN, april::AK_NONE, wParam);
		break;
	case WM_LBUTTONDOWN:
		_touchDown = true;
		_mouseMoveMessagesCount = 0;
		april::window->queueMouseEvent(MOUSE_DOWN, april::window->getCursorPosition(), april::AK_LBUTTON);
		if (!april::window->isFullscreen())
		{
			SetCapture((HWND)april::window->getBackendId());
		}
		break;
	case WM_RBUTTONDOWN:
		_touchDown = true;
		_mouseMoveMessagesCount = 0;
		april::window->queueMouseEvent(MOUSE_DOWN, april::window->getCursorPosition(), april::AK_RBUTTON);
		if (!april::window->isFullscreen())
		{
			SetCapture((HWND)april::window->getBackendId());
		}
		break;
	case WM_LBUTTONUP:
		_touchDown = false;
		april::window->queueMouseEvent(MOUSE_UP, april::window->getCursorPosition(), april::AK_LBUTTON);
		if (!april::window->isFullscreen())
		{
			ReleaseCapture();
		}
		break;
	case WM_RBUTTONUP:
		_touchDown = false;
		april::window->queueMouseEvent(MOUSE_UP, april::window->getCursorPosition(), april::AK_RBUTTON);
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
				april::window->setInputMode(april::Window::MOUSE);
			}
			else
			{
				++_mouseMoveMessagesCount;
			}
		}
		else
		{
			_mouseMoveMessagesCount = 0;
		}
		april::window->queueMouseEvent(MOUSE_MOVE, april::window->getCursorPosition(), april::AK_NONE);
		break;
	case WM_MOUSEWHEEL:
		_wheelDelta = (float)GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
		if ((GET_KEYSTATE_WPARAM(wParam) & MK_CONTROL) != MK_CONTROL)
		{
			april::window->queueMouseEvent(MOUSE_SCROLL, gvec2(0.0f, -(float)_wheelDelta), april::AK_NONE);
		}
		else
		{
			april::window->queueMouseEvent(MOUSE_SCROLL, gvec2(-(float)_wheelDelta, 0.0f), april::AK_NONE);
		}
		break;
	case WM_MOUSEHWHEEL:
		_wheelDelta = (float)GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
		if ((GET_KEYSTATE_WPARAM(wParam) & MK_CONTROL) != MK_CONTROL)
		{
			april::window->queueMouseEvent(MOUSE_SCROLL, gvec2(-(float)_wheelDelta, 0.0f), april::AK_NONE);
		}
		else
		{
			april::window->queueMouseEvent(MOUSE_SCROLL, gvec2(0.0f, -(float)_wheelDelta), april::AK_NONE);
		}
		break;
	case WM_ACTIVATE:
		if (wParam == WA_ACTIVE || wParam == WA_CLICKACTIVE)
		{
			april::window->handleFocusChangeEvent(true);
		}
		else
		{
			april::window->handleFocusChangeEvent(false);
		}
		break;
	}
	return DefWindowProcW(hWnd, message, wParam, lParam);
}
