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
		Win32_Window(int w, int h, bool fullscreen, chstr title);
		~Win32_Window();

		void setTouchEnabled(bool value) { this->touchEnabled = value; }
		bool isTouchEnabled() { return this->touchEnabled; }
		
		//void _setActive(bool value) { this->active = value; }
		
		// implementations
		void enterMainLoop();
		bool updateOneFrame();
		void terminateMainLoop();
		void destroyWindow();
		void showSystemCursor(bool visible);
		bool isSystemCursorShown();
		int getWidth();
		int getHeight();
		void setTitle(chstr title);
		void _setResolution(int width, int height);
		void presentFrame();
		void* getIDFromBackend();
		void doEvents();

		static LRESULT CALLBACK processCallback(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
		
	protected:
		hstr fpsTitle;
		HWND hWnd;
		april::Timer globalTimer;
		bool touchEnabled; // whether or not a Win7+ touchscreen was detected
		
		LRESULT _processEvents(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	private:
		bool _touchDown;
		bool _doubleTapDown;
		int _nMouseMoveMessages;
		float _wheelDelta;
		float _lastTime;
		float _fpsTimer;
		int _fps;

	};
}

#endif
