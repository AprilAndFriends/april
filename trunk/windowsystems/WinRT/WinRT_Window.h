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

#ifdef HAVE_WINRT
#ifndef APRIL_WINRT_WINDOW_H
#define APRIL_WINRT_WINDOW_H
#include <hltypes/hplatform.h>
#if _HL_WINRT
#include <gtypes/Matrix4.h>
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
		
		bool create(int w, int h, bool fullscreen, chstr title, chstr options = "");
		bool destroy();
		
		//void setTitle(chstr title);
		void setCursorVisible(bool value);
		HL_DEFINE_GET(int, width, Width);
		HL_DEFINE_GET(int, height, Height);
		void* getBackendId();
		//void _setResolution(int w, int h);
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
#endif