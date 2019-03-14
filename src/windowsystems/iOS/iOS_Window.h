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
/// Defines an iOS window.

#ifdef _IOS_WINDOW
#ifndef APRIL_IOS_WINDOW_H
#define APRIL_IOS_WINDOW_H

#include "Timer.h"
#include "Window.h"

struct CGRect;

namespace april
{
	class iOS_Window : public Window
	{
	public:
		iOS_Window();
		~iOS_Window();
		
		void setCursorVisible(bool value);
		bool isCursorVisible() const;
		HL_DEFINE_GET(int, width, Width);
		HL_DEFINE_GET(int, height, Height);
		void setTitle(chstr value);
		void* getBackendId() const;
		bool isVirtualKeyboardVisible() const;
		
		void checkEvents();
		
		void destroyWindow();
		
		void showVirtualKeyboard();
		void hideVirtualKeyboard();
		void terminateMainLoop();
		void keyboardWasShown(float kbSize);
		void keyboardWasHidden();
		
		hstr getParam(chstr param);
		void setParam(chstr param, chstr value);
		
		void handleFocusChange(bool focused);
		
		void touchesBegan_withEvent_(void* nssetTouches, void* uieventEvent);
		void touchesEnded_withEvent_(void* nssetTouches, void* uieventEvent);
		void touchesMoved_withEvent_(void* nssetTouches, void* uieventEvent);
		void touchesCancelled_withEvent_(void* nssetTouches, void* uieventEvent);
		
		void injectChar(unsigned int inputChar);
		
		void setDeviceOrientationCallback(void (*do_callback)());
		void applicationWillResignActive();
		void applicationDidBecomeActive();
		
		void _setCursorPosition(float x, float y);
		Cursor* _createCursor(bool fromResource);
		float _getTouchScale() const;
		
	protected:
		int width;
		int height;
		int keyboardRequest;
		bool firstFrameDrawn;
		bool inputEventsMutex;
		bool retainLoadingOverlay;
		hmap<void*, int> osTouches;
		void (*exitFunction)(int);
		
		void _systemCreate(int width, int height, bool fullscreen, chstr title, Window::Options options);
		
		void _processEvents();
		
		void _presentFrame(bool systemEnabled);
		
	};
	
}

CGRect getScreenBounds();

#endif
#endif
