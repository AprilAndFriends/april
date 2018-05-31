/// @file
/// @version 4.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <april/Application.h>
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

#define CUSTOM_WINDOW_CLASS L"AprilCustomWindow"

CustomWindow::CustomWindow() : april::Window()
{
	this->name = "Custom";
	this->hWnd = NULL;
}

CustomWindow::~CustomWindow()
{
	this->destroy();
}

void CustomWindow::_systemCreate(int w, int h, bool fullscreen, chstr title, april::Window::Options options)
{
	if (fullscreen)
	{
		hlog::warnf(LOG_TAG, "Window '%s' does not support fullscreen", this->name.cStr());
		fullscreen = false;
	}
	april::Window::_systemCreate(w, h, fullscreen, title, options);
	this->inputMode = april::InputMode::Mouse;
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
	int x = (screenWidth - w) / 2;
	int y = (screenHeight - h) / 2;
	// setting the necessary styles
	DWORD style = (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX);
	DWORD exstyle = WS_EX_LEFT;
	// takes into account the offset from the border
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
	// create window
	this->hWnd = CreateWindowExW(exstyle, CUSTOM_WINDOW_CLASS, this->title.wStr().c_str(), style, x, y, w, h, NULL, NULL, hinst, NULL);
	// display the window on the screen
	ShowWindow(this->hWnd, SW_SHOWNORMAL);
	UpdateWindow(this->hWnd);
}

void CustomWindow::_systemDestroy()
{
	april::Window::destroy();
	if (this->hWnd != 0)
	{
		DestroyWindow(this->hWnd);
		UnregisterClassW(CUSTOM_WINDOW_CLASS, GetModuleHandle(0));
		this->hWnd = 0;
	}
}

int CustomWindow::getWidth() const
{
	RECT rect;
	GetClientRect(this->hWnd, &rect);
	return (rect.right - rect.left);
}

int CustomWindow::getHeight() const
{
	RECT rect;
	GetClientRect(this->hWnd, &rect);
	return (rect.bottom - rect.top);
}

void* CustomWindow::getBackendId() const
{
	return this->hWnd;
}

bool CustomWindow::update(float timeDelta)
{
	POINT w32_cursorPosition;
	// mouse position
	GetCursorPos(&w32_cursorPosition);
	ScreenToClient(this->hWnd, &w32_cursorPosition);
	this->cursorPosition.set((float)w32_cursorPosition.x, (float)w32_cursorPosition.y);
	// rendering
	return april::Window::update(timeDelta);
}

void CustomWindow::_presentFrame()
{
	CustomRenderSystem* system = dynamic_cast<CustomRenderSystem*>(april::rendersys);
	if (system != NULL)
	{
		SwapBuffers(system->getHDC());
	}
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
				april::window->queueMouseEvent(april::MouseEvent::Type::Up, april::window->getCursorPosition(), april::Key::DoubleTap);
			}
			_touchDown = false;
		}
		else if (wParam == 6) // GID_TWOFINGERTAP
		{
			_doubleTapDown = true;
			april::window->queueMouseEvent(april::MouseEvent::Type::Down, april::window->getCursorPosition(), april::Key::DoubleTap);
		}
		break;
	case 0x011A: // WM_GESTURENOTIFY (win7+ only)
		_touchDown = true;
		april::window->setInputMode(april::InputMode::Touch);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		april::application->finish();
		break;
	case WM_CLOSE:
		april::window->handleQuitRequest(true);
		break;
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
		april::window->queueKeyEvent(april::KeyEvent::Type::Down, april::Key::fromInt(wParam), 0);
		return 0;
	case WM_SYSKEYUP:
		if (wParam == VK_MENU)
		{
			_altKeyDown = false;
		}
		// no break here, because this is still an input message that needs to be processed normally
	case WM_KEYUP:
		april::window->queueKeyEvent(april::KeyEvent::Type::Up, april::Key::fromInt((int)wParam), 0);
		return 0;
	case WM_CHAR:
		april::window->queueKeyEvent(april::KeyEvent::Type::Down, april::Key::None, wParam);
		break;
	case WM_LBUTTONDOWN:
		_touchDown = true;
		_mouseMoveMessagesCount = 0;
		april::window->queueMouseEvent(april::MouseEvent::Type::Down, april::window->getCursorPosition(), april::Key::MouseL);
		if (!april::window->isFullscreen())
		{
			SetCapture((HWND)april::window->getBackendId());
		}
		break;
	case WM_RBUTTONDOWN:
		_touchDown = true;
		_mouseMoveMessagesCount = 0;
		april::window->queueMouseEvent(april::MouseEvent::Type::Down, april::window->getCursorPosition(), april::Key::MouseR);
		if (!april::window->isFullscreen())
		{
			SetCapture((HWND)april::window->getBackendId());
		}
		break;
	case WM_LBUTTONUP:
		_touchDown = false;
		april::window->queueMouseEvent(april::MouseEvent::Type::Up, april::window->getCursorPosition(), april::Key::MouseL);
		if (!april::window->isFullscreen())
		{
			ReleaseCapture();
		}
		break;
	case WM_RBUTTONUP:
		_touchDown = false;
		april::window->queueMouseEvent(april::MouseEvent::Type::Up, april::window->getCursorPosition(), april::Key::MouseR);
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
				april::window->setInputMode(april::InputMode::Mouse);
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
		april::window->queueMouseEvent(april::MouseEvent::Type::Move, april::window->getCursorPosition(), april::Key::None);
		break;
	case WM_MOUSEWHEEL:
		_wheelDelta = (float)GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
		if ((GET_KEYSTATE_WPARAM(wParam) & MK_CONTROL) != MK_CONTROL)
		{
			april::window->queueMouseEvent(april::MouseEvent::Type::Scroll, gvec2f(0.0f, -(float)_wheelDelta), april::Key::None);
		}
		else
		{
			april::window->queueMouseEvent(april::MouseEvent::Type::Scroll, gvec2f(-(float)_wheelDelta, 0.0f), april::Key::None);
		}
		break;
	case WM_MOUSEHWHEEL:
		_wheelDelta = (float)GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
		if ((GET_KEYSTATE_WPARAM(wParam) & MK_CONTROL) != MK_CONTROL)
		{
			april::window->queueMouseEvent(april::MouseEvent::Type::Scroll, gvec2f(-(float)_wheelDelta, 0.0f), april::Key::None);
		}
		else
		{
			april::window->queueMouseEvent(april::MouseEvent::Type::Scroll, gvec2f(0.0f, -(float)_wheelDelta), april::Key::None);
		}
		break;
	case WM_ACTIVATE:
		if (wParam == WA_ACTIVE || wParam == WA_CLICKACTIVE)
		{
			april::window->queueFocusChangeEvent(true);
		}
		else
		{
			april::window->queueFocusChangeEvent(false);
		}
		break;
	}
	return DefWindowProcW(hWnd, message, wParam, lParam);
}
