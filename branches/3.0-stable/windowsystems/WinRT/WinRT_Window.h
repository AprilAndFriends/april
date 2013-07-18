/// @file
/// @author  Boris Mikic
/// @version 3.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
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
#include "WinRT_View.h"

namespace april
{
	class Texture;
	
	class WinRT_Window : public Window
	{
	public:
		WinRT_Window();
		~WinRT_Window();
		
		bool create(int w, int h, bool fullscreen, chstr title, Window::Options options);
		bool destroy();
		void unassign();
		
		//void setTitle(chstr title);
		void setCursorVisible(bool value);
		HL_DEFINE_GET(int, width, Width);
		HL_DEFINE_GET(int, height, Height);
		void* getBackendId();
		void setResolution(int w, int h, bool fullscreen);
		bool updateOneFrame();
		void presentFrame();
		void checkEvents();
		
	protected:
		int width;
		int height;
		bool hasStoredProjectionMatrix;
		gmat4 storedProjectionMatrix;
		Color backgroundColor;
		Texture* logoTexture;
		
		void _tryLoadLogoTexture();
		
	};
	
}

#endif
#endif
