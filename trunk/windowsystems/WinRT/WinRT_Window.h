/// @file
/// @author  Boris Mikic
/// @version 2.51
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
		struct MouseInputEvent
		{
			MouseEventType type;
			gvec2 position;
			MouseButton button;
		
			MouseInputEvent(MouseEventType _type, gvec2 _position, MouseButton _button)
			{
				type = _type;
				position = _position;
				button = _button;
			}
		
		};

		struct KeyInputEvent
		{
			KeyEventType type;
			KeySym keyCode;
			unsigned int charCode;
		
			KeyInputEvent(KeyEventType _type, KeySym _keyCode, unsigned int _charCode)
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

		bool create(int w, int h, bool fullscreen, chstr title);
		bool destroy();

		//void setTitle(chstr title);
		void setCursorVisible(bool value);
		int getWidth() { return this->width; }
		int getHeight() { return this->height; }
		bool isTouchEnabled() { return this->touchEnabled; }
		void setTouchEnabled(bool value) { this->touchEnabled = value; }
		void* getBackendId();
		//void _setResolution(int w, int h);
		bool updateOneFrame();
		void presentFrame();
		void checkEvents();

		void handleTouchEvent(MouseEventType type, gvec2 position, int index);
		void handleMouseEvent(MouseEventType type, gvec2 position, MouseButton button);
		void handleKeyEvent(KeyEventType type, KeySym keyCode, unsigned int charCode);

	protected:
		int width;
		int height;
		bool touchEnabled;
		bool multiTouchActive;
		harray<gvec2> touches;
		harray<MouseInputEvent> mouseEvents;
		harray<KeyInputEvent> keyEvents;
		harray<TouchInputEvent> touchEvents;

	};

}

#endif
#endif
#endif