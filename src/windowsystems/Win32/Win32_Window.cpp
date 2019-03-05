/// @file
/// @version 5.2
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
	bool _touchEnabled = true;

	Win32_Window::Win32_Window() :
		Window()
	{
		this->name = april::WindowType::Win32.getName();
		this->hWnd = NULL;
		this->width = 0;
		this->height = 0;
		this->defaultCursor = LoadCursor(0, IDC_ARROW);
		this->refreshCursorRequested = false;
		this->cursorExtensions += ".ani";
		this->cursorExtensions += ".cur";
#ifdef _WIN32_XINPUT
		memset(&this->xinputStates, 0, sizeof(XINPUT_STATE) * XUSER_MAX_COUNT);
		memset(&this->connectedControllers, 0, sizeof(bool) * XUSER_MAX_COUNT);
#endif
		this->_mouseMessages = 0;
	}

	Win32_Window::~Win32_Window()
	{
		this->destroy();
		DestroyCursor(this->defaultCursor);
	}

	// Considering that Iron Man and Batman's only real superpower is being
	// super rich and smart, Bill Gates turned out to be a real disappointment.
	void Win32_Window::_systemCreate(int width, int height, bool fullscreen, chstr title, Window::Options options)
	{
		Window::_systemCreate(width, height, fullscreen, title, options);
		this->inputMode = InputMode::Mouse;
		// Win32
		WNDCLASSEXW windowClass;
		memset(&windowClass, 0, sizeof(WNDCLASSEX));
		HINSTANCE hinst = GetModuleHandle(0);
		windowClass.cbSize = sizeof(WNDCLASSEX);
		windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
		windowClass.lpfnWndProc = &Win32_Window::_mainProcessCallback;
		windowClass.hInstance = hinst;
		windowClass.hCursor = this->defaultCursor;
		windowClass.lpszClassName = APRIL_WIN32_WINDOW_CLASS;
		windowClass.hIcon = (HICON)LoadImage(hinst, MAKEINTRESOURCE(1), IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR);
		windowClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
		RegisterClassExW(&windowClass);
		// determine position
		int screenWidth = GetSystemMetrics(SM_CXSCREEN);
		int screenHeight = GetSystemMetrics(SM_CYSCREEN);
		int x = 0;
		int y = 0;
		if (!this->fullscreen)
		{
			x = (screenWidth - width) / 2;
			y = (screenHeight - height) / 2;
		}
		// setting the necessary styles
		DWORD style = 0;
		DWORD exstyle = 0;
		this->_setupStyles(style, exstyle, this->fullscreen);
		if (!this->fullscreen)
		{
			this->_adjustWindowSizeForClient(x, y, width, height, style, exstyle);
			x = (screenWidth - width) / 2;
			y = (screenHeight - height) / 2;
		}
		// create window
		this->hWnd = CreateWindowExW(exstyle, APRIL_WIN32_WINDOW_CLASS, this->title.wStr().c_str(), style, x, y, width, height, NULL, NULL, hinst, NULL);
		if (options.enableTouchInput)
		{
			// this workaround is required to properly support everything before Win7
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
		}
		else
		{
			_touchEnabled = false;
		}
		RECT clientRect;
		GetClientRect(this->hWnd, &clientRect);
		this->width = clientRect.right - clientRect.left;
		this->height = clientRect.bottom - clientRect.top;
		// display the window on the screen
		ShowWindow(this->hWnd, !this->options.minimized ? SW_SHOWNORMAL : SW_SHOWMINIMIZED);
		UpdateWindow(this->hWnd);
		SetCursor(windowClass.hCursor);
		this->setCursorVisible(true);
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
		if (this->options.fpsCounter)
		{
			hstr newTitle = title + hsprintf(" [FPS: %d]", april::application->getFps());
			// optimization to prevent setting title every frame
			if (newTitle == this->fpsTitle)
			{
				return;
			}
			this->fpsTitle = newTitle;
			SetWindowTextW(this->hWnd, newTitle.wStr().c_str());
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
		this->refreshCursorRequested = true;
	}

	void Win32_Window::_updateCursorPosition()
	{
		// Win32 mouse position
		static POINT _systemCursorPosition;
		GetCursorPos(&_systemCursorPosition);
		ScreenToClient(this->hWnd, &_systemCursorPosition);
		this->cursorPosition.set((float)_systemCursorPosition.x, (float)_systemCursorPosition.y);
	}

	void* Win32_Window::getBackendId() const
	{
		return this->hWnd;
	}

	HCURSOR Win32_Window::getCursorHandle() const
	{
		return (this->cursor != NULL ? ((Win32_Cursor*)this->cursor)->getCursor() : this->defaultCursor);
	}

	bool Win32_Window::update(float timeDelta)
	{
		if (this->inputMode == InputMode::Mouse)
		{
			this->_updateCursorPosition();
		}
		// rendering
		bool result = Window::update(timeDelta);
		if (this->options.fpsCounter)
		{
			this->setTitle(this->title); // has to come after Window::updateOneFrame(), otherwise FPS value in title would be late one frame
		}
		return result;
	}

	void Win32_Window::_systemSetResolution(int width, int height, bool fullscreen)
	{
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
			deviceMode.dmPelsWidth = width;
			deviceMode.dmPelsWidth = height;
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
				x = (GetSystemMetrics(SM_CXSCREEN) - width) / 2;
				y = (GetSystemMetrics(SM_CYSCREEN) - height) / 2;
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
			this->_adjustWindowSizeForClient(x, y, width, height, style, exstyle);
		}
		// needs to be set here already, because some calls below may call system event callbacks
		this->fullscreen = fullscreen;
		SetWindowPos(this->hWnd, (fullscreen ? HWND_TOPMOST : HWND_NOTOPMOST), x, y, width, height, 0);
		// the requested size might not match the one the OS is willing to actually allow
		RECT clientRect;
		GetClientRect(this->hWnd, &clientRect);
		this->width = clientRect.right - clientRect.left;
		this->height = clientRect.bottom - clientRect.top;
		// tell render system to resize back buffers and finish things up
		this->_setRenderSystemResolution(this->width, this->height, this->fullscreen);
		ShowWindow(this->hWnd, SW_SHOW);
		UpdateWindow(this->hWnd);
	}
	
	void Win32_Window::_presentFrame(bool systemEnabled)
	{
		Window::_presentFrame(systemEnabled);
#ifdef _OPENGL
		harray<hstr> renderSystems;
		renderSystems += april::RenderSystemType::OpenGL1.getName();
		renderSystems += april::RenderSystemType::OpenGLES2.getName();
		if (renderSystems.has(april::rendersys->getName()))
		{
			SwapBuffers(((OpenGL_RenderSystem*)april::rendersys)->getHDC());
		}
#endif
	}
	
	void Win32_Window::checkEvents()
	{
		// this is called from the main thread so this code is put here
		if (this->refreshCursorRequested)
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
			this->refreshCursorRequested = false;
		}
		MSG message;
		// limiting to 100 events per frame just to be safe
		for_iter (i, 0, 100)
		{
			if (!PeekMessageW(&message, this->hWnd, 0, 0, PM_REMOVE))
			{
				break;
			}
			TranslateMessage(&message);
			DispatchMessageW(&message);
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
					this->queueControllerInput(ControllerEvent::Type::Connected, i, Button::None, 0.0f);
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
					this->queueControllerInput((newButtonState != 0 ? ControllerEvent::Type::Down : ControllerEvent::Type::Up), i, Button::Start, 0.0f);
				}
				newButtonState = states[i].Gamepad.wButtons & XINPUT_GAMEPAD_BACK;
				if (newButtonState != (this->xinputStates[i].Gamepad.wButtons & XINPUT_GAMEPAD_BACK))
				{
					this->queueControllerInput((newButtonState != 0 ? ControllerEvent::Type::Down : ControllerEvent::Type::Up), i, Button::Select, 0.0f);
				}
				newButtonState = states[i].Gamepad.wButtons & XINPUT_GAMEPAD_A;
				if (newButtonState != (this->xinputStates[i].Gamepad.wButtons & XINPUT_GAMEPAD_A))
				{
					this->queueControllerInput((newButtonState != 0 ? ControllerEvent::Type::Down : ControllerEvent::Type::Up), i, Button::A, 0.0f);
				}
				newButtonState = states[i].Gamepad.wButtons & XINPUT_GAMEPAD_B;
				if (newButtonState != (this->xinputStates[i].Gamepad.wButtons & XINPUT_GAMEPAD_B))
				{
					this->queueControllerInput((newButtonState != 0 ? ControllerEvent::Type::Down : ControllerEvent::Type::Up), i, Button::B, 0.0f);
				}
				newButtonState = states[i].Gamepad.wButtons & XINPUT_GAMEPAD_X;
				if (newButtonState != (this->xinputStates[i].Gamepad.wButtons & XINPUT_GAMEPAD_X))
				{
					this->queueControllerInput((newButtonState != 0 ? ControllerEvent::Type::Down : ControllerEvent::Type::Up), i, Button::X, 0.0f);
				}
				newButtonState = states[i].Gamepad.wButtons & XINPUT_GAMEPAD_Y;
				if (newButtonState != (this->xinputStates[i].Gamepad.wButtons & XINPUT_GAMEPAD_Y))
				{
					this->queueControllerInput((newButtonState != 0 ? ControllerEvent::Type::Down : ControllerEvent::Type::Up), i, Button::Y, 0.0f);
				}
				// special buttons
				newButtonState = states[i].Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER;
				if (newButtonState != (this->xinputStates[i].Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER))
				{
					this->queueControllerInput((newButtonState != 0 ? ControllerEvent::Type::Down : ControllerEvent::Type::Up), i, Button::L1, 0.0f);
				}
				newButtonState = states[i].Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER;
				if (newButtonState != (this->xinputStates[i].Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER))
				{
					this->queueControllerInput((newButtonState != 0 ? ControllerEvent::Type::Down : ControllerEvent::Type::Up), i, Button::R1, 0.0f);
				}
				newButtonState = states[i].Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB;
				if (newButtonState != (this->xinputStates[i].Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB))
				{
					this->queueControllerInput((newButtonState != 0 ? ControllerEvent::Type::Down : ControllerEvent::Type::Up), i, Button::LS, 0.0f);
				}
				newButtonState = states[i].Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB;
				if (newButtonState != (this->xinputStates[i].Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB))
				{
					this->queueControllerInput((newButtonState != 0 ? ControllerEvent::Type::Down : ControllerEvent::Type::Up), i, Button::RS, 0.0f);
				}
				// D-Pad
				newButtonState = states[i].Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP;
				if (newButtonState != (this->xinputStates[i].Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP))
				{
					this->queueControllerInput((newButtonState != 0 ? ControllerEvent::Type::Down : ControllerEvent::Type::Up), i, Button::DPadUp, 0.0f);
				}
				newButtonState = states[i].Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN;
				if (newButtonState != (this->xinputStates[i].Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN))
				{
					this->queueControllerInput((newButtonState != 0 ? ControllerEvent::Type::Down : ControllerEvent::Type::Up), i, Button::DPadDown, 0.0f);
				}
				newButtonState = states[i].Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT;
				if (newButtonState != (this->xinputStates[i].Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT))
				{
					this->queueControllerInput((newButtonState != 0 ? ControllerEvent::Type::Down : ControllerEvent::Type::Up), i, Button::DPadLeft, 0.0f);
				}
				newButtonState = states[i].Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT;
				if (newButtonState != (this->xinputStates[i].Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT))
				{
					this->queueControllerInput((newButtonState != 0 ? ControllerEvent::Type::Down : ControllerEvent::Type::Up), i, Button::DPadRight, 0.0f);
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
							this->queueControllerInput(ControllerEvent::Type::Down, i, Button::L2, 0.0f);
						}
						this->queueControllerInput(ControllerEvent::Type::Axis, i, Button::TriggerL, (float)states[i].Gamepad.bLeftTrigger * ONE_BY_255);
					}
					else
					{
						if (oldTriggerState)
						{
							this->queueControllerInput(ControllerEvent::Type::Up, i, Button::L2, 0.0f);
						}
						this->queueControllerInput(ControllerEvent::Type::Axis, i, Button::TriggerL, 0.0f);
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
							this->queueControllerInput(ControllerEvent::Type::Down, i, Button::R2, 0.0f);
						}
						this->queueControllerInput(ControllerEvent::Type::Axis, i, Button::TriggerR, (float)states[i].Gamepad.bRightTrigger  * 0.003921569f);
					}
					else
					{
						if (oldTriggerState)
						{
							this->queueControllerInput(ControllerEvent::Type::Up, i, Button::R2, 0.0f);
						}
						this->queueControllerInput(ControllerEvent::Type::Axis, i, Button::TriggerR, 0.0f);
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
					this->queueControllerInput(ControllerEvent::Type::Axis, i, Button::AxisLX, 0.0f);
					this->queueControllerInput(ControllerEvent::Type::Axis, i, Button::AxisLY, 0.0f);
				}
				else if (newAxisState)
				{
					if (!oldAxisState || states[i].Gamepad.sThumbLX != this->xinputStates[i].Gamepad.sThumbLX)
					{
						this->queueControllerInput(ControllerEvent::Type::Axis, i, Button::AxisLX, (states[i].Gamepad.sThumbLX > 0 ? newAxisX * ONE_BY_32767 : newAxisX * ONE_BY_32768));
					}
					if (!oldAxisState || states[i].Gamepad.sThumbLY != this->xinputStates[i].Gamepad.sThumbLY)
					{
						// negative, because XBox uses physics logic rather than screen logic for Y
						this->queueControllerInput(ControllerEvent::Type::Axis, i, Button::AxisLY, -(states[i].Gamepad.sThumbLY > 0 ? newAxisY * ONE_BY_32767 : newAxisY * ONE_BY_32768));
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
					this->queueControllerInput(ControllerEvent::Type::Axis, i, Button::AxisRX, 0.0f);
					this->queueControllerInput(ControllerEvent::Type::Axis, i, Button::AxisRY, 0.0f);
				}
				else if (newAxisState)
				{
					if (!oldAxisState || states[i].Gamepad.sThumbRX != this->xinputStates[i].Gamepad.sThumbRX)
					{
						this->queueControllerInput(ControllerEvent::Type::Axis, i, Button::AxisRX, (states[i].Gamepad.sThumbRX > 0 ? newAxisX * ONE_BY_32767 : newAxisX * ONE_BY_32768));
					}
					if (!oldAxisState || states[i].Gamepad.sThumbRY != this->xinputStates[i].Gamepad.sThumbRY)
					{
						// negative, because XBox uses physics logic rather than screen logic for Y
						this->queueControllerInput(ControllerEvent::Type::Axis, i, Button::AxisRY, -(states[i].Gamepad.sThumbRY > 0 ? newAxisY * ONE_BY_32767 : newAxisY * ONE_BY_32768));
					}
				}
			}
		}
		memcpy(&this->xinputStates, &states, sizeof(XINPUT_STATE) * XUSER_MAX_COUNT);
		// disconnection events go after releasing all buttons and resetting all axises
		foreach (int, it, disconnectedControllers)
		{
			this->queueControllerInput(ControllerEvent::Type::Disconnected, (*it), Button::None, 0.0f);
		}
	}
#endif

	void Win32_Window::_setupStyles(DWORD& style, DWORD& exstyle, bool fullscreen)
	{
		style = STYLE_WINDOWED;
		exstyle = EXSTYLE_WINDOWED;
		if (fullscreen)
		{
			style = STYLE_FULLSCREEN;
			exstyle = EXSTYLE_FULLSCREEN;
		}
		else if (this->options.resizable)
		{
			style |= WS_SIZEBOX | WS_MAXIMIZEBOX;
		}
	}

	void Win32_Window::_adjustWindowSizeForClient(int x, int y, int& width, int& height, DWORD style, DWORD exstyle)
	{
		RECT rect;
		rect.left = x;
		rect.top = y;
		rect.right = x + width;
		rect.bottom = y + height;
		AdjustWindowRectEx(&rect, style, FALSE, exstyle);
		width = rect.right - rect.left;
		height = rect.bottom - rect.top;
	}

	void Win32_Window::queueControllerInput(const ControllerEvent::Type& type, int controllerIndex, const Button& buttonCode, float axisValue)
	{
		if (type != ControllerEvent::Type::Connected && type != ControllerEvent::Type::Disconnected)
		{
			this->_mouseMessages = 5;
			this->queueInputModeChange(InputMode::Controller);
		}
		Window::queueControllerInput(type, controllerIndex, buttonCode, axisValue);
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
			// handle only windowed variants of rsolution change here
			if (!april::window->isFullscreen() && (_sizeChanging || wParam == SIZE_MAXIMIZED || wParam == SIZE_RESTORED && !_initialSize))
			{
				RECT clientRect;
				GetClientRect(WIN32_WINDOW->hWnd, &clientRect);
				WIN32_WINDOW->width = clientRect.right - clientRect.left;
				WIN32_WINDOW->height = clientRect.bottom - clientRect.top;
				WIN32_WINDOW->_setRenderSystemResolution(april::window->getWidth(), april::window->getHeight(), april::window->isFullscreen());
				ShowWindow(WIN32_WINDOW->hWnd, SW_SHOW);
				UpdateWindow(WIN32_WINDOW->hWnd);
				april::window->queueSizeChange(april::window->getWidth(), april::window->getHeight(), april::window->isFullscreen());
				if (_sizeChanging) // this doesn't work right during fullscreen switching so it's only used when resizing the window
				{
					april::application->renderFrameSync();
				}
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
		static TOUCHINPUT touches[1000];
		static POINT _systemCursorPosition;
		static gvec2f position;
		switch (message)
		{
		case WM_TOUCH: // (Win7+ only)
			if (wParam > 0 && _getTouchInputInfo != NULL && _closeTouchInputHandle != NULL)
			{
				april::window->queueInputModeChange(InputMode::Touch);
				WIN32_WINDOW->_mouseMessages = 5;
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
						april::window->queueTouchInput(TouchEvent::Type::Down, 0, april::window->getCursorPosition());
					}
					else if ((touches[0].dwFlags & TOUCHEVENTF_UP) == TOUCHEVENTF_UP)
					{
						if (!april::window->isFullscreen())
						{
							ReleaseCapture();
						}
						april::window->queueTouchInput(TouchEvent::Type::Up, 0, april::window->getCursorPosition());
					}
					else if ((touches[0].dwFlags & TOUCHEVENTF_MOVE) == TOUCHEVENTF_MOVE)
					{
						april::window->queueTouchInput(TouchEvent::Type::Move, 0, april::window->getCursorPosition());
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
			april::window->queueQuitRequest(true);
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
					april::window->queueQuitRequest(true);
					return 0;
				}
				if (wParam == VK_RETURN)
				{
					WIN32_WINDOW->_systemToggleHotkeyFullscreen(); // this comes from the main thread so it can directly call the needed function
					return 0;
				}
			}
			april::window->queueKeyInput(KeyEvent::Type::Down, april::Key::fromInt((int)wParam), 0);
			return 0;
		case WM_SYSKEYUP:
			if (wParam == VK_MENU || wParam == VK_RETURN || wParam == VK_F4)
			{
				_altKeyDown = false;
			}
			// no break here, because this is still an input message that needs to be processed normally
		case WM_KEYUP:
			april::window->queueKeyInput(KeyEvent::Type::Up, april::Key::fromInt((int)wParam), 0);
			return 0;
		case WM_CHAR:
			april::window->queueKeyInput(KeyEvent::Type::Down, Key::None, wParam);
			break;
		case WM_LBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_RBUTTONDOWN:
			if (!_touchEnabled || (GetMessageExtraInfo() & MOUSEEVENTF_FROMTOUCH) != MOUSEEVENTF_FROMTOUCH)
			{
				WIN32_WINDOW->_mouseMessages = 0;
				if (!april::window->isFullscreen())
				{
					SetCapture((HWND)april::window->getBackendId());
				}
				april::window->queueInputModeChange(InputMode::Mouse);
				position.set((float)GET_X_LPARAM(lParam), (float)GET_Y_LPARAM(lParam));
				if (april::window->getInputMode() != InputMode::Touch)
				{
					april::window->queueMouseInput(MouseEvent::Type::Down, position, _getKeyFromMessage(message));
				}
				else // touch simulation is used, update cursor position and create a touch event
				{
					WIN32_WINDOW->_updateCursorPosition();
					april::window->queueTouchInput(TouchEvent::Type::Down, 0, position);
				}
			}
			break;
		case WM_LBUTTONUP:
		case WM_MBUTTONUP:
		case WM_RBUTTONUP:
			if (!_touchEnabled || (GetMessageExtraInfo() & MOUSEEVENTF_FROMTOUCH) != MOUSEEVENTF_FROMTOUCH)
			{
				WIN32_WINDOW->_mouseMessages = 0;
				if (!april::window->isFullscreen())
				{
					ReleaseCapture();
				}
				april::window->queueInputModeChange(InputMode::Mouse);
				position.set((float)GET_X_LPARAM(lParam), (float)GET_Y_LPARAM(lParam));
				if (april::window->getInputMode() != InputMode::Touch)
				{
					april::window->queueMouseInput(MouseEvent::Type::Up, position, _getKeyFromMessage(message));
				}
				else // touch simulation is used, update cursor position and create a touch event
				{
					WIN32_WINDOW->_updateCursorPosition();
					april::window->queueTouchInput(TouchEvent::Type::Up, 0, position);
				}
			}
			break;
		case WM_MOUSEMOVE:
			if (!_touchEnabled || (GetMessageExtraInfo() & MOUSEEVENTF_FROMTOUCH) != MOUSEEVENTF_FROMTOUCH)
			{
				if (WIN32_WINDOW->_mouseMessages > 0)
				{
					--WIN32_WINDOW->_mouseMessages;
				}
				if (WIN32_WINDOW->_mouseMessages == 0)
				{
					april::window->queueInputModeChange(InputMode::Mouse);
					position.set((float)GET_X_LPARAM(lParam), (float)GET_Y_LPARAM(lParam));
					if (april::window->getInputMode() != InputMode::Touch)
					{
						april::window->queueMouseInput(MouseEvent::Type::Move, position, Key::None);
					}
					else // touch simulation is used, update cursor position and create a touch event
					{
						WIN32_WINDOW->_updateCursorPosition();
						april::window->queueTouchInput(TouchEvent::Type::Move, 0, position);
					}
				}
			}
			break;
		case WM_MOUSEWHEEL:
			if (!_touchEnabled || (GetMessageExtraInfo() & MOUSEEVENTF_FROMTOUCH) != MOUSEEVENTF_FROMTOUCH)
			{
				_wheelDelta = (float)GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
				if ((GET_KEYSTATE_WPARAM(wParam) & MK_CONTROL) != MK_CONTROL)
				{
					april::window->queueMouseInput(MouseEvent::Type::Scroll, gvec2f(0.0f, -(float)_wheelDelta), Key::None);
				}
				else
				{
					april::window->queueMouseInput(MouseEvent::Type::Scroll, gvec2f(-(float)_wheelDelta, 0.0f), Key::None);
				}
			}
			break;
		case WM_MOUSEHWHEEL:
			if (!_touchEnabled || (GetMessageExtraInfo() & MOUSEEVENTF_FROMTOUCH) != MOUSEEVENTF_FROMTOUCH)
			{
				_wheelDelta = (float)GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
				if ((GET_KEYSTATE_WPARAM(wParam) & MK_CONTROL) != MK_CONTROL)
				{
					april::window->queueMouseInput(MouseEvent::Type::Scroll, gvec2f(-(float)_wheelDelta, 0.0f), Key::None);
				}
				else
				{
					april::window->queueMouseInput(MouseEvent::Type::Scroll, gvec2f(0.0f, -(float)_wheelDelta), Key::None);
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
					april::window->queueFocusChange(true);
				}
			}
			else
			{
				_altKeyDown = false; // because ALT keyup isn't processed when alt-tab-ing for some reason.
				april::window->queueFocusChange(false);
			}
			break;
		}
		return DefWindowProcW(hWnd, message, wParam, lParam);
	}

}
#endif
