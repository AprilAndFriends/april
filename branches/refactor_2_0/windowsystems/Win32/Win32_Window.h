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
/// 
/// @section DESCRIPTION
/// 
/// Defines a Win32 window.

#ifdef _WIN32
#ifndef APRIL_WIN32_WINDOW_H
#define APRIL_WIN32_WINDOW_H
#include <windows.h>

#include <hltypes/hstring.h>

#include "aprilExport.h"
#include "Timer.h"
#include "Window.h"

namespace april
{
	class aprilExport Win32_Window : public Window
	{
	public:
		Win32_Window();
		~Win32_Window();
		bool create(int w, int h, bool fullscreen, chstr title);
		bool destroy();

		void setTitle(chstr title);
		bool isCursorVisible();
		void setCursorVisible(bool value);
		int getWidth();
		int getHeight();
		bool isTouchEnabled() { return this->touchEnabled; }
		void setTouchEnabled(bool value) { this->touchEnabled = value; }
		void* getBackendId();
		void _setResolution(int w, int h);

		void enterMainLoop();
		bool updateOneFrame();
		void terminateMainLoop();
		void presentFrame();
		void checkEvents();

		static LRESULT CALLBACK processCallback(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
		
	protected:
		HWND hWnd;
		april::Timer globalTimer;
		bool touchEnabled; // whether or not a Win7+ touchscreen was detected
		
	private:
		float _lastTime;
		hstr _fpsTitle;
		float _fpsTimer;
		int _fps;

	};

}

#endif
#endif