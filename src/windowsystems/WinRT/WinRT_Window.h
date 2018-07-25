/// @file
/// @version 5.2
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Defines a WinRT window.

#ifdef _WINRT_WINDOW
#ifndef APRIL_WINRT_WINDOW_H
#define APRIL_WINRT_WINDOW_H

#include <gtypes/Matrix4.h>
#include <hltypes/hplatform.h>
#include <hltypes/hstring.h>

#include "aprilExport.h"
#include "Color.h"
#include "Timer.h"
#include "Window.h"
#include "WinRT.h"

#define WINRT_DELAY_SPLASH "delay_splash"
#define WINRT_CURSOR_MAPPINGS "cursor_mappings"
#define WINP8_BACK_BUTTON_SYSTEM_HANDLING "back_button_system_handling"

namespace april
{
	class Texture;
	
	class WinRT_Window : public Window
	{
	public:
		WinRT_Window();
		~WinRT_Window();
		
		hstr getParam(chstr param);
		void setParam(chstr param, chstr value);
		
		void setTitle(chstr title);
		HL_DEFINE_GET(int, width, Width);
		HL_DEFINE_GET(int, height, Height);
		void* getBackendId() const;

		hstr findCursorFile(chstr filename) const;
		
		void showVirtualKeyboard();
		void hideVirtualKeyboard();
		void changeSize(float width, float height); // required override instead of normal size changing
		
	protected:
		int width;
		int height;
		float delaySplash;
		bool backButtonSystemHandling;
		hmap<hstr, unsigned int> cursorMappings;

		void _systemCreate(int width, int height, bool fullscreen, chstr title, Window::Options options);
		
		Cursor* _createCursor(bool fromResource);
		void _refreshCursor();

		void _systemSetResolution(int width, int height, bool fullscreen);
		
	};
	
}
#endif
#endif
