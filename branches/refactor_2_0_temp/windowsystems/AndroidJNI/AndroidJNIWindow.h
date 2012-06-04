/// @file
/// @author  Boris Mikic
/// @version 1.85
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/// 
/// @section DESCRIPTION
/// 
/// Defines an Android JNI window.

#ifndef APRIL_ANDROID_JNI_WINDOW_H
#define APRIL_ANDROID_JNI_WINDOW_H

#include <hltypes/hstring.h>

#include "Window.h"
#include "aprilExport.h"

namespace april
{
	struct MouseInputEvent
	{
		Window::MouseEventType type;
		float x;
		float y;
		Window::MouseButton button;

		MouseInputEvent(Window::MouseEventType _type, float _x, float _y, Window::MouseButton _button)
		{
			type = _type;
			x = _x;
			y = _y;
			button = _button;
		}

	};

	struct KeyInputEvent
	{
		Window::KeyEventType type;
		KeySym keyCode;
		unsigned int charCode;

		KeyInputEvent(Window::KeyEventType _type, KeySym _keyCode, unsigned int _charCode)
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

	class aprilExport AndroidJNIWindow : public Window
	{
	public:
		AndroidJNIWindow(int w, int h, bool fullscreen, chstr title);
		~AndroidJNIWindow();
		
		// implementations
		void enterMainLoop();
		bool updateOneFrame();
		void terminateMainLoop();
		void destroyWindow();
		void showSystemCursor(bool visible);
		bool isSystemCursorShown();
		int getWidth();
		int getHeight();
		void setWindowTitle(chstr title);
		//void _setResolution(int w, int h);
		gvec2 getCursorPosition();
		void presentFrame();
		void* getIDFromBackend();
		void doEvents();
		
		void beginKeyboardHandling();
		void terminateKeyboardHandling();
		void handleTouchEvent(MouseEventType type, float x, float y, int index);
		void handleMouseEvent(MouseEventType type, float x, float y, MouseButton button);
		void handleKeyEvent(KeyEventType type, KeySym keyCode, unsigned int charCode);
		
		DeviceType getDeviceType();
		
	protected:
		float mWidth;
		float mHeight;
		bool mMultiTouchActive;
		harray<gvec2> mTouches;
		harray<MouseInputEvent> mMouseEvents;
		harray<KeyInputEvent> mKeyEvents;
		harray<TouchInputEvent> mTouchEvents;
		
		// using void** so that jni.h doesn't have to be included in this header
		void _getVirtualKeyboardClasses(void** javaEnv, void** javaClassInputMethodManager, void** javaInputMethodManager, void** javaView);
		
	};
}
#endif
