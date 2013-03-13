/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @author  Ivan Vucica
/// @version 3.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/// 
/// @section DESCRIPTION
/// 
/// Defines a Win32 window.

#ifdef HAVE_WIN32
#ifndef APRIL_WIN32_WINDOW_H
#define APRIL_WIN32_WINDOW_H
#include <hltypes/hplatform.h>
#if !_HL_WINRT
#include <hltypes/hstring.h>

#include "aprilExport.h"
#include "Timer.h"
#include "Window.h"

namespace april
{
	class Win32_Window : public Window
	{
	public:
		Win32_Window();
		~Win32_Window();
		bool create(int w, int h, bool fullscreen, chstr title, chstr options = "");
		bool destroy();

		void setTitle(chstr title);
		bool isCursorVisible();
		void setCursorVisible(bool value);
		int getWidth();
		int getHeight();
		void* getBackendId();
		void _setResolution(int w, int h);

		bool updateOneFrame();
		void presentFrame();
		void checkEvents();

		static LRESULT CALLBACK processCallback(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
		
	protected:
		HWND hWnd;
		
	};

}

#endif
#endif
#endif
