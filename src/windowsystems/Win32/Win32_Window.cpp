/// @file
/// @version 4.5
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

#include <WindowsX.h>

#include "Application.h"
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

#define ONE_BY_255 0.003921569f
#define ONE_BY_32767 0.00003051851f
#define ONE_BY_32768 0.00003051757f

#define MOUSEEVENTF_FROMTOUCH 0xFF515700
#define APRIL_WIN32_WINDOW_CLASS L"AprilWin32Window"
#define STYLE_FULLSCREEN WS_POPUP
#define STYLE_WINDOWED (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX)
#define EXSTYLE_FULLSCREEN WS_EX_TOPMOST
#define EXSTYLE_WINDOWED WS_EX_LEFT

#define WIN32_WINDOW ((Win32_Window*)april::window)

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
		this->name = april::WindowType::Win32.getName();
		this->hWnd = NULL;
		this->defaultCursor = LoadCursor(0, IDC_ARROW);
		this->cursorExtensions += ".ani";
		this->cursorExtensions += ".cur";
#ifdef _WIN32_XINPUT
		memset(&this->xinputStates, 0, sizeof(XINPUT_STATE) * XUSER_MAX_COUNT);
		memset(&this->connectedControllers, 0, sizeof(bool) * XUSER_MAX_COUNT);
#endif
	}

	Win32_Window::~Win32_Window()
	{
		this->destroy();
		DestroyCursor(this->defaultCursor);
	}

	// Considering that Iron Man and Batman's only real superpower is being
	// super rich and smart, Bill Gates turned out to be a real disappointment.
	void Win32_Window::_systemCreate(int w, int h, bool fullscreen, chstr title, Window::Options options)
	{
		Window::_systemCreate(w, h, fullscreen, title, options);
		this->inputMode = InputMode::Mouse;
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
	}
	
	void Win32_Window::_systemDestroy()
	{
		Window::_systemDestroy();
		if (this->hWnd != 0)
		{
			DestroyWindow(this->hWnd);
			UnregisterClassW(APRIL_WIN32_WINDOW_CLASS, GetModuleHandle(0));
			this->hWnd = 0;
		}
#ifdef _EGL
		april::egl->destroy();
#endif
	}

	void Win32_Window::setTitle(chstr title)
	{
		if (this->fpsCounter)
		{
			hstr t = title + hsprintf(" [FPS: %d]", april::application->getFps());
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
	
	bool Win32_Window::isCursorVisible() const
	{
		return (Window::isCursorVisible() || this->inputMode == InputMode::Mouse && !this->isCursorInside());
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

	void Win32_Window::_updateCursorPosition()
	{
		// Win32 mouse position
		static POINT _systemCursorPosition;
		GetCursorPos(&_systemCursorPosition);
		ScreenToClient(this->hWnd, &_systemCursorPosition);
		this->cursorPosition.set((float)_systemCursorPosition.x, (float)_systemCursorPosition.y);
	}

	int Win32_Window::getWidth() const
	{
		RECT rect;
		GetClientRect(this->hWnd, &rect);
		return (rect.right - rect.left);
	}
	
	int Win32_Window::getHeight() const
	{
		RECT rect;
		GetClientRect(this->hWnd, &rect);
		return (rect.bottom - rect.top);
	}

	void* Win32_Window::getBackendId() const
	{
		return this->hWnd;
	}

	HCURSOR Win32_Window::getCursorHandle() const
	{
		return (this->cursor != NULL ? ((Win32_Cursor*)this->cursor)->getCursor() : this->defaultCursor);
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
		this->handleSizeChange(this->getWidth(), this->getHeight(), this->fullscreen);
	}
	
	bool Win32_Window::update(float timeDelta)
	{
		if (this->inputMode == InputMode::Mouse)
		{
			this->_updateCursorPosition();
		}
		// rendering
		bool result = Window::update(timeDelta);
		if (this->fpsCounter)
		{
			this->setTitle(this->title); // has to come after Window::updateOneFrame(), otherwise FPS value in title would be late one frame
		}
		return result;
	}
	
	void Win32_Window::_presentFrame()
	{
#ifdef _OPENGL
		harray<hstr> renderSystems;
		renderSystems += april::RenderSystemType::OpenGL1.getName();
		renderSystems += april::RenderSystemType::OpenGLES1.getName();
		renderSystems += april::RenderSystemType::OpenGLES2.getName();
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
#ifdef _WIN32_XINPUT
		this->_checkXInputControllerStates();
#endif
		Window::checkEvents();
	}

	Cursor* Win32_Window::_createCursor(bool fromResource)
	{
		return new Win32_Cursor(fromResource);
	}

#ifdef _WIN32_XINPUT
	void Win32_Window::_checkXInputControllerStates()
	{
		XINPUT_STATE states[XUSER_MAX_COUNT];
		memset(&states, 0, sizeof(XINPUT_STATE) * XUSER_MAX_COUNT);
		DWORD result = 0;
		harray<int> disconnectedControllers;
		// first the connection state
		for_iter (i, 0, XUSER_MAX_COUNT)
		{
			result = XInputGetState(i, &states[i]);
			if (result != ERROR_DEVICE_NOT_CONNECTED)
			{
				if (!this->connectedControllers[i])
				{
					this->connectedControllers[i] = true;
					this->queueControllerEvent(ControllerEvent::Type::Connected, i, Button::None, 0.0f);
				}
			}
			else if (this->connectedControllers[i])
			{
				this->connectedControllers[i] = false;
				disconnectedControllers += i; // disconnection events go after releasing all buttons and resetting all axises
				memset(&states[i], 0, sizeof(XINPUT_STATE));
			}
		}
		int newButtonState = 0;
		bool oldTriggerState = false;
		bool newTriggerState = false;
		float oldAxisX = 0.0f;
		float oldAxisY = 0.0f;
		float newAxisX = 0.0f;
		float newAxisY = 0.0f;
		bool oldAxisState = false;
		bool newAxisState = false;
		// now check the input (disconnected controllers release all buttons and reset all axises)
		for_iter (i, 0, XUSER_MAX_COUNT)
		{
			if (this->connectedControllers[i] || disconnectedControllers.has(i))
			{
				// basic buttons
				newButtonState = states[i].Gamepad.wButtons & XINPUT_GAMEPAD_START;
				if (newButtonState != (this->xinputStates[i].Gamepad.wButtons & XINPUT_GAMEPAD_START))
				{
					this->queueControllerEvent((newButtonState != 0 ? ControllerEvent::Type::Down : ControllerEvent::Type::Up), i, Button::Start, 0.0f);
				}
				newButtonState = states[i].Gamepad.wButtons & XINPUT_GAMEPAD_BACK;
				if (newButtonState != (this->xinputStates[i].Gamepad.wButtons & XINPUT_GAMEPAD_BACK))
				{
					this->queueControllerEvent((newButtonState != 0 ? ControllerEvent::Type::Down : ControllerEvent::Type::Up), i, Button::Select, 0.0f);
				}
				newButtonState = states[i].Gamepad.wButtons & XINPUT_GAMEPAD_A;
				if (newButtonState != (this->xinputStates[i].Gamepad.wButtons & XINPUT_GAMEPAD_A))
				{
					this->queueControllerEvent((newButtonState != 0 ? ControllerEvent::Type::Down : ControllerEvent::Type::Up), i, Button::A, 0.0f);
				}
				newButtonState = states[i].Gamepad.wButtons & XINPUT_GAMEPAD_B;
				if (newButtonState != (this->xinputStates[i].Gamepad.wButtons & XINPUT_GAMEPAD_B))
				{
					this->queueControllerEvent((newButtonState != 0 ? ControllerEvent::Type::Down : ControllerEvent::Type::Up), i, Button::B, 0.0f);
				}
				newButtonState = states[i].Gamepad.wButtons & XINPUT_GAMEPAD_X;
				if (newButtonState != (this->xinputStates[i].Gamepad.wButtons & XINPUT_GAMEPAD_X))
				{
					this->queueControllerEvent((newButtonState != 0 ? ControllerEvent::Type::Down : ControllerEvent::Type::Up), i, Button::X, 0.0f);
				}
				newButtonState = states[i].Gamepad.wButtons & XINPUT_GAMEPAD_Y;
				if (newButtonState != (this->xinputStates[i].Gamepad.wButtons & XINPUT_GAMEPAD_Y))
				{
					this->queueControllerEvent((newButtonState != 0 ? ControllerEvent::Type::Down : ControllerEvent::Type::Up), i, Button::Y, 0.0f);
				}
				// special buttons
				newButtonState = states[i].Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER;
				if (newButtonState != (this->xinputStates[i].Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER))
				{
					this->queueControllerEvent((newButtonState != 0 ? ControllerEvent::Type::Down : ControllerEvent::Type::Up), i, Button::L1, 0.0f);
				}
				newButtonState = states[i].Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER;
				if (newButtonState != (this->xinputStates[i].Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER))
				{
					this->queueControllerEvent((newButtonState != 0 ? ControllerEvent::Type::Down : ControllerEvent::Type::Up), i, Button::R1, 0.0f);
				}
				newButtonState = states[i].Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB;
				if (newButtonState != (this->xinputStates[i].Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB))
				{
					this->queueControllerEvent((newButtonState != 0 ? ControllerEvent::Type::Down : ControllerEvent::Type::Up), i, Button::LS, 0.0f);
				}
				newButtonState = states[i].Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB;
				if (newButtonState != (this->xinputStates[i].Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB))
				{
					this->queueControllerEvent((newButtonState != 0 ? ControllerEvent::Type::Down : ControllerEvent::Type::Up), i, Button::RS, 0.0f);
				}
				// D-Pad
				newButtonState = states[i].Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP;
				if (newButtonState != (this->xinputStates[i].Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP))
				{
					this->queueControllerEvent((newButtonState != 0 ? ControllerEvent::Type::Down : ControllerEvent::Type::Up), i, Button::DPadUp, 0.0f);
				}
				newButtonState = states[i].Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN;
				if (newButtonState != (this->xinputStates[i].Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN))
				{
					this->queueControllerEvent((newButtonState != 0 ? ControllerEvent::Type::Down : ControllerEvent::Type::Up), i, Button::DPadDown, 0.0f);
				}
				newButtonState = states[i].Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT;
				if (newButtonState != (this->xinputStates[i].Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT))
				{
					this->queueControllerEvent((newButtonState != 0 ? ControllerEvent::Type::Down : ControllerEvent::Type::Up), i, Button::DPadLeft, 0.0f);
				}
				newButtonState = states[i].Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT;
				if (newButtonState != (this->xinputStates[i].Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT))
				{
					this->queueControllerEvent((newButtonState != 0 ? ControllerEvent::Type::Down : ControllerEvent::Type::Up), i, Button::DPadRight, 0.0f);
				}
				// triggers
				oldTriggerState = (this->xinputStates[i].Gamepad.bLeftTrigger >= XINPUT_GAMEPAD_TRIGGER_THRESHOLD);
				newTriggerState = (states[i].Gamepad.bLeftTrigger >= XINPUT_GAMEPAD_TRIGGER_THRESHOLD);
				if (newTriggerState != oldTriggerState || newTriggerState && oldTriggerState && states[i].Gamepad.bLeftTrigger != this->xinputStates[i].Gamepad.bLeftTrigger)
				{
					if (newTriggerState)
					{
						if (!oldTriggerState)
						{
							this->queueControllerEvent(ControllerEvent::Type::Down, i, Button::L2, 0.0f);
						}
						this->queueControllerEvent(ControllerEvent::Type::Axis, i, Button::TriggerL, (float)states[i].Gamepad.bLeftTrigger * ONE_BY_255);
					}
					else
					{
						if (oldTriggerState)
						{
							this->queueControllerEvent(ControllerEvent::Type::Up, i, Button::L2, 0.0f);
						}
						this->queueControllerEvent(ControllerEvent::Type::Axis, i, Button::TriggerL, 0.0f);
					}
				}
				oldTriggerState = (this->xinputStates[i].Gamepad.bRightTrigger >= XINPUT_GAMEPAD_TRIGGER_THRESHOLD);
				newTriggerState = (states[i].Gamepad.bRightTrigger >= XINPUT_GAMEPAD_TRIGGER_THRESHOLD);
				if (newTriggerState != oldTriggerState || newTriggerState && oldTriggerState && states[i].Gamepad.bRightTrigger != this->xinputStates[i].Gamepad.bRightTrigger)
				{
					if (newTriggerState)
					{
						if (!oldTriggerState)
						{
							this->queueControllerEvent(ControllerEvent::Type::Down, i, Button::R2, 0.0f);
						}
						this->queueControllerEvent(ControllerEvent::Type::Axis, i, Button::TriggerR, (float)states[i].Gamepad.bRightTrigger  * 0.003921569f);
					}
					else
					{
						if (oldTriggerState)
						{
							this->queueControllerEvent(ControllerEvent::Type::Up, i, Button::R2, 0.0f);
						}
						this->queueControllerEvent(ControllerEvent::Type::Axis, i, Button::TriggerR, 0.0f);
					}
				}
				// axises
				oldAxisX = (float)this->xinputStates[i].Gamepad.sThumbLX;
				oldAxisY = (float)this->xinputStates[i].Gamepad.sThumbLY;
				newAxisX = (float)states[i].Gamepad.sThumbLX;
				newAxisY = (float)states[i].Gamepad.sThumbLY;
				oldAxisState = ((int)hsqrt(oldAxisX * oldAxisX + oldAxisY * oldAxisY) >= XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
				newAxisState = ((int)hsqrt(newAxisX * newAxisX + newAxisY * newAxisY) >= XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
				if (!newAxisState && oldAxisState)
				{
					this->queueControllerEvent(ControllerEvent::Type::Axis, i, Button::AxisLX, 0.0f);
					this->queueControllerEvent(ControllerEvent::Type::Axis, i, Button::AxisLY, 0.0f);
				}
				else if (newAxisState)
				{
					if (!oldAxisState || states[i].Gamepad.sThumbLX != this->xinputStates[i].Gamepad.sThumbLX)
					{
						this->queueControllerEvent(ControllerEvent::Type::Axis, i, Button::AxisLX, (states[i].Gamepad.sThumbLX > 0 ? newAxisX * ONE_BY_32767 : newAxisX * ONE_BY_32768));
					}
					if (!oldAxisState || states[i].Gamepad.sThumbLY != this->xinputStates[i].Gamepad.sThumbLY)
					{
						// negative, because XBox uses physics logic rather than screen logic for Y
						this->queueControllerEvent(ControllerEvent::Type::Axis, i, Button::AxisLY, -(states[i].Gamepad.sThumbLY > 0 ? newAxisY * ONE_BY_32767 : newAxisY * ONE_BY_32768));
					}
				}
				oldAxisX = (float)this->xinputStates[i].Gamepad.sThumbRX;
				oldAxisY = (float)this->xinputStates[i].Gamepad.sThumbRY;
				newAxisX = (float)states[i].Gamepad.sThumbRX;
				newAxisY = (float)states[i].Gamepad.sThumbRY;
				oldAxisState = ((int)hsqrt(oldAxisX * oldAxisX + oldAxisY * oldAxisY) >= XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
				newAxisState = ((int)hsqrt(newAxisX * newAxisX + newAxisY * newAxisY) >= XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
				if (!newAxisState && oldAxisState)
				{
					this->queueControllerEvent(ControllerEvent::Type::Axis, i, Button::AxisRX, 0.0f);
					this->queueControllerEvent(ControllerEvent::Type::Axis, i, Button::AxisRY, 0.0f);
				}
				else if (newAxisState)
				{
					if (!oldAxisState || states[i].Gamepad.sThumbRX != this->xinputStates[i].Gamepad.sThumbRX)
					{
						this->queueControllerEvent(ControllerEvent::Type::Axis, i, Button::AxisRX, (states[i].Gamepad.sThumbRX > 0 ? newAxisX * ONE_BY_32767 : newAxisX * ONE_BY_32768));
					}
					if (!oldAxisState || states[i].Gamepad.sThumbRY != this->xinputStates[i].Gamepad.sThumbRY)
					{
						// negative, because XBox uses physics logic rather than screen logic for Y
						this->queueControllerEvent(ControllerEvent::Type::Axis, i, Button::AxisRY, -(states[i].Gamepad.sThumbRY > 0 ? newAxisY * ONE_BY_32767 : newAxisY * ONE_BY_32768));
					}
				}
			}
		}
		memcpy(&this->xinputStates, &states, sizeof(XINPUT_STATE) * XUSER_MAX_COUNT);
		// disconnection events go after releasing all buttons and resetting all axises
		foreach (int, it, disconnectedControllers)
		{
			this->queueControllerEvent(ControllerEvent::Type::Disconnected, (*it), Button::None, 0.0f);
		}
	}
#endif

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

	static int _mouseMessages = 0;

	void Win32_Window::handleSizeChange(int width, int height, bool fullscreen)
	{
		this->_setRenderSystemResolution(width, height, fullscreen);
		UpdateWindow(this->hWnd);
		Window::handleSizeChange(width, height, fullscreen);
		this->performUpdate(0.0f);
		april::rendersys->presentFrame();
	}

	void Win32_Window::queueControllerEvent(const ControllerEvent::Type& type, int controllerIndex, const Button& buttonCode, float axisValue)
	{
		if (type != ControllerEvent::Type::Connected && type != ControllerEvent::Type::Disconnected)
		{
			_mouseMessages = 5;
			this->queueInputModeChangeEvent(InputMode::Controller);
		}
		Window::queueControllerEvent(type, controllerIndex, buttonCode, axisValue);
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
				april::window->queueSizeChangeEvent(april::window->getWidth(), april::window->getHeight(), april::window->isFullscreen());
				april::rendersys->update(0.0f);
				// TODOx - remove this?
				//april::window->performUpdate(0.0f);
				//april::rendersys->presentFrame();
			}
			_initialSize = false;
			break;
		}
		return Win32_Window::childProcessCallback(hWnd, message, wParam, lParam);
	}

	static inline april::Key _getKeyFromMessage(UINT message)
	{
		if (message == WM_LBUTTONDOWN || message == WM_LBUTTONUP)
		{
			return Key::MouseL;
		}
		if (message == WM_RBUTTONDOWN || message == WM_RBUTTONUP)
		{
			return Key::MouseR;
		}
		if (message == WM_MBUTTONDOWN || message == WM_MBUTTONUP)
		{
			return Key::MouseM;
		}
		return Key::None;
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
		static TOUCHINPUT touches[1000];
		static POINT _systemCursorPosition;
		switch (message)
		{
		case WM_TOUCH: // (Win7+ only)
			if (wParam > 0 && _getTouchInputInfo != NULL && _closeTouchInputHandle != NULL)
			{
				april::window->queueInputModeChangeEvent(InputMode::Touch);
				_mouseMessages = 5;
				if (_getTouchInputInfo((HTOUCHINPUT)lParam, wParam, touches, sizeof(TOUCHINPUT)))
				{
					_systemCursorPosition.x = TOUCH_COORD_TO_PIXEL(touches[0].x);
					_systemCursorPosition.y = TOUCH_COORD_TO_PIXEL(touches[0].y);
					ScreenToClient(hWnd, &_systemCursorPosition);
					WIN32_WINDOW->cursorPosition.set((float)_systemCursorPosition.x, (float)_systemCursorPosition.y);
					if ((touches[0].dwFlags & TOUCHEVENTF_DOWN) == TOUCHEVENTF_DOWN)
					{
						if (!april::window->isFullscreen())
						{
							SetCapture((HWND)april::window->getBackendId());
						}
						april::window->queueMouseEvent(MouseEvent::Type::Down, april::window->getCursorPosition(), Key::MouseL);
					}
					else if ((touches[0].dwFlags & TOUCHEVENTF_UP) == TOUCHEVENTF_UP)
					{
						if (!april::window->isFullscreen())
						{
							ReleaseCapture();
						}
						april::window->queueMouseEvent(MouseEvent::Type::Up, april::window->getCursorPosition(), Key::MouseL);
					}
					else if ((touches[0].dwFlags & TOUCHEVENTF_MOVE) == TOUCHEVENTF_MOVE)
					{
						april::window->queueMouseEvent(MouseEvent::Type::Move, april::window->getCursorPosition(), Key::None);
					}
				}
				_closeTouchInputHandle((HTOUCHINPUT)lParam);
			}
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			april::application->finish();
			break;
		case WM_CLOSE:
			april::window->queueQuitRequestEvent(true);
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
					april::window->queueQuitRequestEvent(true);
					return 0;
				}
				if (wParam == VK_RETURN)
				{
					april::window->toggleHotkeyFullscreen();
					return 0;
				}
			}
			april::window->queueKeyEvent(KeyEvent::Type::Down, april::Key::fromInt((int)wParam), 0);
			return 0;
		case WM_SYSKEYUP:
			if (wParam == VK_MENU || wParam == VK_RETURN || wParam == VK_F4)
			{
				_altKeyDown = false;
			}
			// no break here, because this is still an input message that needs to be processed normally
		case WM_KEYUP:
			april::window->queueKeyEvent(KeyEvent::Type::Up, april::Key::fromInt((int)wParam), 0);
			return 0;
		case WM_CHAR:
			april::window->queueKeyEvent(KeyEvent::Type::Down, Key::None, wParam);
			break;
			// oh no, it's LMR!
		case WM_LBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_RBUTTONDOWN:
			if ((GetMessageExtraInfo() & MOUSEEVENTF_FROMTOUCH) != MOUSEEVENTF_FROMTOUCH)
			{
				_mouseMessages = 0;
				if (!april::window->isFullscreen())
				{
					SetCapture((HWND)april::window->getBackendId());
				}
				april::window->queueInputModeChangeEvent(InputMode::Mouse);
				// some sort of touch simulation going on, update cursor position
				if (april::window->getInputMode() == InputMode::Touch)
				{
					WIN32_WINDOW->_updateCursorPosition();
				}
				april::window->queueMouseEvent(MouseEvent::Type::Down, gvec2((float)GET_X_LPARAM(lParam), (float)GET_Y_LPARAM(lParam)), _getKeyFromMessage(message));
			}
			break;
		case WM_LBUTTONUP:
		case WM_MBUTTONUP:
		case WM_RBUTTONUP:
			if ((GetMessageExtraInfo() & MOUSEEVENTF_FROMTOUCH) != MOUSEEVENTF_FROMTOUCH)
			{
				_mouseMessages = 0;
				if (!april::window->isFullscreen())
				{
					ReleaseCapture();
				}
				april::window->queueInputModeChangeEvent(InputMode::Mouse);
				// some sort of touch simulation going on, update cursor position
				if (april::window->getInputMode() == InputMode::Touch)
				{
					WIN32_WINDOW->_updateCursorPosition();
				}
				april::window->queueMouseEvent(MouseEvent::Type::Up, gvec2((float)GET_X_LPARAM(lParam), (float)GET_Y_LPARAM(lParam)), _getKeyFromMessage(message));
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
					april::window->queueInputModeChangeEvent(InputMode::Mouse);
					// some sort of touch simulation going on, update cursor position
					if (april::window->getInputMode() == InputMode::Mouse)
					{
						WIN32_WINDOW->_updateCursorPosition();
					}
					april::window->queueMouseEvent(MouseEvent::Type::Move, gvec2((float)GET_X_LPARAM(lParam), (float)GET_Y_LPARAM(lParam)), Key::None);
				}
			}
			break;
		case WM_MOUSEWHEEL:
			if ((GetMessageExtraInfo() & MOUSEEVENTF_FROMTOUCH) != MOUSEEVENTF_FROMTOUCH)
			{
				_wheelDelta = (float)GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
				if ((GET_KEYSTATE_WPARAM(wParam) & MK_CONTROL) != MK_CONTROL)
				{
					april::window->queueMouseEvent(MouseEvent::Type::Scroll, gvec2(0.0f, -(float)_wheelDelta), Key::None);
				}
				else
				{
					april::window->queueMouseEvent(MouseEvent::Type::Scroll, gvec2(-(float)_wheelDelta, 0.0f), Key::None);
				}
			}
			break;
		case WM_MOUSEHWHEEL:
			if ((GetMessageExtraInfo() & MOUSEEVENTF_FROMTOUCH) != MOUSEEVENTF_FROMTOUCH)
			{
				_wheelDelta = (float)GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
				if ((GET_KEYSTATE_WPARAM(wParam) & MK_CONTROL) != MK_CONTROL)
				{
					april::window->queueMouseEvent(MouseEvent::Type::Scroll, gvec2(-(float)_wheelDelta, 0.0f), Key::None);
				}
				else
				{
					april::window->queueMouseEvent(MouseEvent::Type::Scroll, gvec2(0.0f, -(float)_wheelDelta), Key::None);
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
				HCURSOR cursor = WIN32_WINDOW->getCursorHandle();
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
					april::window->queueFocusChangeEvent(true);
				}
			}
			else
			{
				_altKeyDown = false; // because ALT keyup isn't processed when alt-tab-ing for some reason.
				april::window->queueFocusChangeEvent(false);
			}
			break;
		}
		return DefWindowProcW(hWnd, message, wParam, lParam);
	}

}
#endif
