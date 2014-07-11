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
/// Defines an Android JNI window.

#ifdef _ANDROID
#ifndef APRIL_ANDROID_JNI_WINDOW_H
#define APRIL_ANDROID_JNI_WINDOW_H

#include <hltypes/harray.h>
#include <hltypes/hstring.h>

#include "aprilExport.h"
#include "Timer.h"
#include "Window.h"

namespace april
{
	class AndroidJNI_Window : public Window
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

		AndroidJNI_Window();
		~AndroidJNI_Window();
		bool create(int w, int h, bool fullscreen, chstr title, chstr options = "");
		
		void setTitle(chstr title) { }
		bool isCursorVisible() { return false; }
		void setCursorVisible(bool value) { }
		HL_DEFINE_GET(int, width, Width);
		HL_DEFINE_GET(int, height, Height);
		bool isTouchEnabled() { return true; }
		void setTouchEnabled(bool value) { }
		void* getBackendId();
		
		void enterMainLoop();
		bool updateOneFrame();
		void presentFrame();
		void checkEvents();
		
		void beginKeyboardHandling();
		void terminateKeyboardHandling();
		void handleTouchEvent(MouseEventType type, gvec2 position, int index);
		void handleMouseEvent(MouseEventType type, gvec2 position, Key button);
		void handleKeyEvent(KeyEventType type, Key keyCode, unsigned int charCode);

		void handleFocusChangeEvent(bool focused);
		void handleActivityChangeEvent(bool active);
		
	protected:
		int width;
		int height;
		bool multiTouchActive;
		bool forcedFocus;
		harray<gvec2> touches;
		harray<MouseInputEvent> mouseEvents;
		harray<KeyInputEvent> keyEvents;
		harray<TouchInputEvent> touchEvents;
		
	};

}
#endif
#endif