/// @file
/// @version 3.4
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
		inline bool isCursorVisible() { return false; }
		inline void setCursorVisible(bool value) { }
		HL_DEFINE_GET(int, width, Width);
		HL_DEFINE_GET(int, height, Height);
		void* getBackendId();
		
		void enterMainLoop();
		void presentFrame();
		
		void queueTouchEvent(MouseEventType type, gvec2 position, int index);
		void queueControllerEvent(ControllerEventType type, Button buttonCode);

		void beginKeyboardHandling();
		void terminateKeyboardHandling();

		void handleFocusChangeEvent(bool focused);
		void handleActivityChangeEvent(bool active);
		
	protected:
		int width;
		int height;
		bool forcedFocus;

		Cursor* _createCursor();
		void _refreshCursor() { }
		
	};

}
#endif
#endif