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
/// Defines an Android JNI window.

#ifdef _ANDROIDJNI_WINDOW
#ifndef APRIL_ANDROID_JNI_WINDOW_H
#define APRIL_ANDROID_JNI_WINDOW_H

#include <hltypes/harray.h>
#include <hltypes/hstring.h>

#include "aprilExport.h"
#include "Timer.h"
#include "Window.h"

namespace april
{
	class Cursor;

	class AndroidJNI_Window : public Window
	{
	public:
		AndroidJNI_Window();
		~AndroidJNI_Window();
		bool create(int w, int h, bool fullscreen, chstr title, Window::Options options);
		
		inline void setTitle(chstr title) { }
		inline bool isCursorVisible() const { return false; }
		inline void setCursorVisible(bool value) { }
		HL_DEFINE_GET(int, width, Width);
		HL_DEFINE_GET(int, height, Height);
		void* getBackendId() const;
		
		void enterMainLoop();
		void presentFrame();
		
		void queueTouchEvent(MouseInputEvent::Type type, cgvec2 position, int index);
		void queueControllerEvent(ControllerInputEvent::Type type, int controllerIndex, Button buttonCode, float axisValue);

		void showVirtualKeyboard();
		void hideVirtualKeyboard();

		void handleFocusChangeEvent(bool focused);
		void handleActivityChange(bool active);
		
	protected:
		int width;
		int height;
		bool forcedFocus;

		Cursor* _createCursor(bool fromResource);
		void _refreshCursor() { }
		
	};

}
#endif
#endif