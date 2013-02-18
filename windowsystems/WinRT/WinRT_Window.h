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
		struct MouseInputEvent
		{
			MouseEventType type;
			gvec2 position;
			Key button;
		
			MouseInputEvent(MouseEventType _type, gvec2 _position, Key _button)
			{
				type = _type;
				position = _position;
				button = _button;
			}
		
		};
		
		struct KeyInputEvent
		{
			KeyEventType type;
			Key keyCode;
			unsigned int charCode;
		
			KeyInputEvent(KeyEventType _type, Key _keyCode, unsigned int _charCode)
			{
				type = _type;
				keyCode = _keyCode;
				charCode = _charCode;
			}
		
		};
		
		struct TouchInputEvent
		{
			harray<gvec2> touches;
		
			TouchInputEvent(harray<gvec2>& _touches)
			{
				touches = _touches;
			}
		
		};
		
		WinRT_Window();
		~WinRT_Window();
		
		bool create(int w, int h, bool fullscreen, chstr title, chstr options = "");
		bool destroy();
		
		//void setTitle(chstr title);
		void setCursorVisible(bool value);
		HL_DEFINE_GET(int, width, Width);
		HL_DEFINE_GET(int, height, Height);
		HL_DEFINE_ISSET(bool, touchEnabled, TouchEnabled);
		void* getBackendId();
		//void _setResolution(int w, int h);
		bool updateOneFrame();
		void presentFrame();
		void checkEvents();
		
		void handleTouchEvent(MouseEventType type, gvec2 position, int index);
		void handleMouseEvent(MouseEventType type, gvec2 position, Key button);
		void handleKeyEvent(KeyEventType type, Key keyCode, unsigned int charCode);
		
	protected:
		int width;
		int height;
		bool touchEnabled;
		bool multiTouchActive;
		bool hasStoredProjectionMatrix;
		gmat4 storedProjectionMatrix;
		Color backgroundColor;
		Texture* logoTexture;
		harray<gvec2> touches;
		harray<MouseInputEvent> mouseEvents;
		harray<KeyInputEvent> keyEvents;
		harray<TouchInputEvent> touchEvents;
		
		void _tryLoadLogoTexture();
		
	};
	
}

#endif
#endif
#endif