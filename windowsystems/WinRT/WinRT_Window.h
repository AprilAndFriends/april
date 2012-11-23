/// @file
/// @author  Boris Mikic
/// @version 2.41
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/// 
/// @section DESCRIPTION
/// 
/// Defines a WinRT window.

#ifdef _WIN32
#ifndef APRIL_WINRT_WINDOW_H
#define APRIL_WINRT_WINDOW_H
#include <hltypes/hplatform.h>
#if _HL_WINRT
#include <hltypes/hstring.h>

#include "aprilExport.h"
#include "Timer.h"
#include "Window.h"
#include "WinRT_View.h"

namespace april
{
	class aprilExport WinRT_Window : public Window
	{
	public:
		WinRT_Window();
		~WinRT_Window();

		bool create(int w, int h, bool fullscreen, chstr title);
		bool destroy();

		//void setTitle(chstr title);
		//bool isCursorVisible();
		//void setCursorVisible(bool value);
		int getWidth() { return this->width; }
		int getHeight() { return this->height; }
		bool isTouchEnabled() { return this->touchEnabled; }
		void setTouchEnabled(bool value) { this->touchEnabled = value; }
		void* getBackendId();
		//void _setResolution(int w, int h);
		bool updateOneFrame();
		void presentFrame();
		void checkEvents();

	protected:
		int width;
		int height;
		bool touchEnabled;

	};

}

#endif
#endif
#endif