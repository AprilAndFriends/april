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
/// Defines an iOS window.

#ifndef APRIL_IOS_WINDOW_H
#define APRIL_IOS_WINDOW_H

#include "Timer.h"
#include "Window.h"

struct CGRect;

namespace april
{
	// We're using input event queuing so we can dispatch them on the main thread
	class InputEvent
	{
	public:
		Window* window;
		
		InputEvent();
		virtual ~InputEvent();
		InputEvent(Window* wnd);
		virtual void execute() = 0;
		
	};
	
	class iOS_Window : public Window
	{
	public:
		iOS_Window();
		~iOS_Window();
		
		// implementations
		bool update(float timeDelta);
		void destroyWindow();
		void setCursorVisible(bool value);
		bool isCursorVisible() const;
		int getWidth() const;
		int getHeight() const;
		void setTitle(chstr value);
		gtypes::Vector2 getCursorPosition() const;
		void* getBackendId() const;
		void checkEvents();
		bool isVirtualKeyboardVisible() const;
		
		void showVirtualKeyboard();
		void hideVirtualKeyboard();
		void terminateMainLoop();
		void keyboardWasShown(float kbSize);
		void keyboardWasHidden();
		
		bool isRotating() const;
		hstr getParam(chstr param);
		void setParam(chstr param, chstr value);
		
		void handleDisplayAndUpdate();
		
		void touchesBegan_withEvent_(void* nssetTouches, void* uieventEvent);
		void touchesEnded_withEvent_(void* nssetTouches, void* uieventEvent);
		void touchesMoved_withEvent_(void* nssetTouches, void* uieventEvent);
		void touchesCancelled_withEvent_(void* nssetTouches, void* uieventEvent);
		void addInputEvent(InputEvent* event);
		InputEvent* popInputEvent();
		
		void injectiOSChar(unsigned int inputChar);
		
		void setDeviceOrientationCallback(void (*do_callback)());
		void applicationWillResignActive();
		void applicationDidBecomeActive();
		
		void _setCursorPosition(float x, float y);
		Cursor* _createCursor(bool fromResource);
		float _getTouchScale() const;
		
	protected:
		int keyboardRequest;
		bool firstFrameDrawn;
		harray<InputEvent*> inputEvents;
		bool inputEventsMutex;
		bool retainLoadingOverlay;
		void (*exitFunction)(int);
		
		void _systemCreate(int w, int h, bool fullscreen, chstr title, Window::Options options);
		
		void callTouchCallback();
		
		void _presentFrame();
		
	};
	
}

bool isiOS8OrNewer();
CGRect getScreenBounds();

#endif
