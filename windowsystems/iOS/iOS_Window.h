/// @file
/// @author  Kresimir Spes
/// @author  Ivan Vucica
/// @author  Boris Mikic
/// @version 2.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/// 
/// @section DESCRIPTION
/// 
/// Defines an iOS window.

#ifndef APRIL_IOS_WINDOW_H
#define APRIL_IOS_WINDOW_H

#include "Timer.h"
#include "Window.h"

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
	
	class iOSWindow : public Window
	{
	public:
		iOSWindow(int w, int h, bool fullscreen, chstr title);
		~iOSWindow();
		
		// implementations
		void enterMainLoop();
		bool updateOneFrame();
		void terminateMainLoop();
		void destroyWindow();
		void setCursorVisible(bool value);
		bool isCursorVisible();
		int getWidth();
		int getHeight();
		void setTitle(chstr value);
		gtypes::Vector2 getCursorPosition();
		void presentFrame();
		void* getIdFromBackend();
		void checkEvents();
		bool isVirtualKeyboardVisible();
		void beginKeyboardHandling();
		void terminateKeyboardHandling();
		void keyboardWasShown();
		void keyboardWasHidden();
		
		bool isRotating();
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
		
		void setDeviceOrientationCallback(void (*do_callback)(DeviceOrientation));
		void deviceOrientationDidChange();

		void applicationWillResignActive();
		void applicationDidBecomeActive();
		
	protected:
		int keyboardRequest;
		Timer timer;
		bool firstFrameDrawn;
		harray<InputEvent*> inputEvents;
		bool inputEventsMutex;
		bool retainLoadingOverlay;
		harray<UITouch*> touches;
		bool multiTouchActive;
		
		float _getTouchScale();
		harray<UITouch*> _convertTouchesToCoordinates(void* touches);
		void callTouchCallback();
		
	};
	
}

#endif