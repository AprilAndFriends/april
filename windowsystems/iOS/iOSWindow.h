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
namespace April
{
	class iOSWindow : public Window
	{
	private:
		
		bool mRunning;
		bool mCursorVisible;
		Timer mTimer;
		float mCursorX, mCursorY;
		bool mFirstFrameDrawn;
		
		void _convertTouchesToCoordinates(void* touches);

	public:
		
		iOSWindow(int w, int h, bool fullscreen, chstr title);
		
		// implementations
		void enterMainLoop();
		void terminateMainLoop();
		void showSystemCursor(bool visible);
		bool isSystemCursorShown();
		int getWindowWidth();
		int getWindowHeight();
		void setWindowTitle(chstr title);
		gtypes::Vector2 getCursorPos();
		void presentFrame();
		void* getIDFromBackend();
		void doEvents();
		void beginKeyboardHandling();
		void terminateKeyboardHandling();
		void keyboardWasShown();
		void keyboardWasHidden();
		
		void handleDisplayAndUpdate();
		
		void touchesBegan_withEvent_(void* nssetTouches, void* uieventEvent);
		void touchesEnded_withEvent_(void* nssetTouches, void* uieventEvent);
		void touchesMoved_withEvent_(void* nssetTouches, void* uieventEvent);
		void touchesCancelled_withEvent_(void* nssetTouches, void* uieventEvent);
		
		bool textField_shouldChangeCharactersInRange_replacementString_(void* uitextfieldTextField, int nsrangeLocation, int nsrangeLength, chstr str);

	};
}

#endif