/************************************************************************************\
 This source file is part of the Awesome Portable Rendering Interface Library         *
 For latest info, see http://libapril.sourceforge.net/                                *
 **************************************************************************************
 Copyright (c) 2010 Ivan Vucica (ivan@vucica.net)                                     *
 *                                                                                    *
 * This program is free software; you can redistribute it and/or modify it under      *
 * the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
 \************************************************************************************/

#ifndef APRIL_IOSWINDOW_H_INCLUDED
#define APRIL_IOSWINDOW_H_INCLUDED

#include "Window.h"
#include "Timer.h"
namespace april
{
	// We're using input event queuing so we can dispatch them on the main thread
	class InputEvent
	{
	public:
		Window* mWindow;
		
		InputEvent(Window* wnd);
		virtual void execute() = 0;
	};
	
	class iOSWindow : public Window
	{
	protected:
		bool mFocused;
		bool mRunning;
		bool mCursorVisible;
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
	public:
		
		iOSWindow(int w, int h, bool fullscreen, chstr title);
		
		// implementations
		void enterMainLoop();
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
		void beginKeyboardHandling();
		void terminateKeyboardHandling();
		void keyboardWasShown();
		void keyboardWasHidden();
		float prefixRotationAngle();
		
		void handleDisplayAndUpdate();
		
		void touchesBegan_withEvent_(void* nssetTouches, void* uieventEvent);
		void touchesEnded_withEvent_(void* nssetTouches, void* uieventEvent);
		void touchesMoved_withEvent_(void* nssetTouches, void* uieventEvent);
		void touchesCancelled_withEvent_(void* nssetTouches, void* uieventEvent);
		void addInputEvent(InputEvent* event);
		InputEvent* popInputEvent();
		
		bool textField_shouldChangeCharactersInRange_replacementString_(void* uitextfieldTextField, int nsrangeLocation, int nsrangeLength, chstr str);
		
		void setDeviceOrientationCallback(void (*do_callback)(DeviceOrientation));
		void deviceOrientationDidChange();

		void applicationWillResignActive();
		void applicationDidBecomeActive();
	};
}

#endif