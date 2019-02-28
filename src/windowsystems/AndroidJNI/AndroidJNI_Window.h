/// @file
/// @version 5.2
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
		
		inline void setTitle(chstr title) { }
		inline bool isCursorVisible() const { return false; }
		inline void setCursorVisible(bool value) { }
		HL_DEFINE_GET(int, width, Width);
		HL_DEFINE_GET(int, height, Height);
		void* getBackendId() const;
		
		bool update(float timeDelta);
		
		void queueTouchInput(TouchEvent::Type type, int index, cgvec2f position);
		void queueControllerInput(ControllerEvent::Type type, int controllerIndex, Button buttonCode, float axisValue);

		void showVirtualKeyboard();
		void hideVirtualKeyboard();

		void handleFocusChange(bool focused);
		void handleActivityChange(bool active);
		
	protected:
		int width;
		int height;
		bool forcedFocus;

		void _systemCreate(int width, int height, bool fullscreen, chstr title, Window::Options options);
		
		Cursor* _createCursor(bool fromResource);
		void _refreshCursor() { }
		
		void _presentFrame(bool systemEnabled);

	};

}
#endif
#endif