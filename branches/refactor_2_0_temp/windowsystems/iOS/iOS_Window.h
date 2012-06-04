/// @file
/// @author  Ivan Vucica
/// @version 1.31
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
		Window* mWindow;
		InputEvent();
		virtual ~InputEvent();
		InputEvent(Window* wnd);
		virtual void execute() = 0;
	};
	
	class iOSWindow : public Window
	{
	protected:
		bool mFocused;
		bool mRunning;
		bool mCursorVisible;
		int mKeyboardRequest;
		Timer mTimer;
		float mCursorX, mCursorY;
		bool mFirstFrameDrawn;
		harray<InputEvent*> mInputEvents;
		bool mInputEventsMutex;
		
		float _getTouchScale();
		harray<UITouch*> _convertTouchesToCoordinates(void* touches);
		void callTouchCallback();
		harray<UITouch*> mTouches;
		bool mMultiTouchActive;
		
		bool mRetainLoadingOverlay;
	public:
		
		static iOSWindow* getSingleton() { return (iOSWindow*) mSingleton; }
		
		iOSWindow(int w, int h, bool fullscreen, chstr title);
		DeviceType getDeviceType();
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
		gtypes::Vector2 getCursorPosition();
		void presentFrame();
		void* getIDFromBackend();
		void doEvents();
		bool isKeyboardVisible();
		void beginKeyboardHandling();
		void terminateKeyboardHandling();
		void keyboardWasShown();
		void keyboardWasHidden();
		float prefixRotationAngle();
		
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
	};
}

#endif