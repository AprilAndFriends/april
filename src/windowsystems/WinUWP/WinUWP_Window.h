/// @file
/// @version 4.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Defines a WinUWP window.

#ifdef _WINUWP_WINDOW
#ifndef APRIL_WINUWP_WINDOW_H
#define APRIL_WINUWP_WINDOW_H

#include <gtypes/Matrix4.h>
#include <hltypes/hplatform.h>
#include <hltypes/hstring.h>

#include "aprilExport.h"
#include "Color.h"
#include "Timer.h"
#include "Window.h"
#include "WinUWP.h"

#define WINUWP_DELAY_SPLASH "delay_splash"
#define WINUWP_CURSOR_MAPPINGS "cursor_mappings"
#define WINP8_BACK_BUTTON_SYSTEM_HANDLING "back_button_system_handling"

namespace april
{
	class Texture;
	
	class WinUWP_Window : public Window
	{
	public:
		WinUWP_Window();
		~WinUWP_Window();
		
		bool create(int w, int h, bool fullscreen, chstr title, Window::Options options);
		
		hstr getParam(chstr param);
		void setParam(chstr param, chstr value);
		
		void setTitle(chstr title);
		HL_DEFINE_GET(int, width, Width);
		HL_DEFINE_GET(int, height, Height);
		void* getBackendId() const;

		void setResolution(int w, int h, bool fullscreen);
		void presentFrame();
		bool updateOneFrame();
		void checkEvents();
		hstr findCursorFile(chstr filename) const;
		
		void terminateMainLoop();
		void showVirtualKeyboard();
		void hideVirtualKeyboard();
		void changeSize(float w, float h); // required override instead of normal size changing
		
	protected:
		int width;
		int height;
		float delaySplash;
		bool backButtonSystemHandling;
		hmap<hstr, int> cursorMappings;

		Cursor* _createCursor(bool fromResource);
		void _refreshCursor();

	};
	
}

#endif
#endif
