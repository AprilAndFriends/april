/// @file
/// @author  Boris Mikic
/// @version 1.52
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

	class aprilExport AndroidJNIWindow : public Window
	{
	public:
		AndroidJNIWindow(int w, int h, bool fullscreen, chstr title);
		~AndroidJNIWindow();
		
		//void _setActive(bool active) { mActive = active; }
		
		// implementations
		void enterMainLoop();
		bool updateOneFrame();
		void terminateMainLoop();
		void destroyWindow();
		//bool isRunning() { return mRunning; }
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
		
		// event handlers
		//void triggerKeyEvent(bool down, unsigned int keycode);
		//void triggerCharEvent(unsigned int chr);
		
		//void triggerMouseUpEvent(int button);
		//void triggerMouseDownEvent(int button);
		//void triggerMouseMoveEvent();
		//bool triggerQuitEvent();
		//void triggerFocusCallback(bool focused);
		
		//void triggerTouchscreenCallback(bool enabled);
		
		void beginKeyboardHandling();
		void terminateKeyboardHandling();
		void handleMouseEvent(MouseEventType type, float x, float y, MouseButton button);
		void handleKeyEvent(KeyEventType type, KeySym keyCode, unsigned int charCode);
		
		DeviceType getDeviceType();
		
	protected:
		float mWidth;
		float mHeight;
		bool mActive;
		//bool mRunning;
		harray<MouseInputEvent> mMouseEvents;
		harray<KeyInputEvent> mKeyEvents;
		
		// using void** so that jni.h doesn't have to be included in this header
		void _getVirtualKeyboardClasses(void** javaEnv, void** javaClassInputMethodManager, void** javaInputMethodManager, void** javaDecorView);
		
	};
}
#endif
