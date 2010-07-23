/************************************************************************************\
This source file is part of the Awesome Portable Rendering Interface Library         *
For latest info, see http://libapril.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Ivan Vucica (ivan@vucica.net)                                     *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/

#include "Window.h"
#include "Keys.h"

namespace April
{
	Window::Window()
	{
		mUpdateCallback=0;
		mMouseDownCallback=0;
		mMouseUpCallback=0;
		mMouseMoveCallback=0;
		mKeyDownCallback=0;
		mKeyUpCallback=0;
		mCharCallback=0;
	}
	
	void Window::setUpdateCallback(bool (*callback)(float))
	{
		mUpdateCallback=callback;
	}

	void Window::setMouseCallbacks(void (*mouse_dn)(float,float,int),
										void (*mouse_up)(float,float,int),
										void (*mouse_move)(float,float))
	{
		mMouseDownCallback=mouse_dn;
		mMouseUpCallback=mouse_up;
		mMouseMoveCallback=mouse_move;
		
	}
	void Window::setKeyboardCallbacks(void (*key_dn)(unsigned int),
									  void (*key_up)(unsigned int),
									  void (*char_callback)(unsigned int))
	{
		mKeyDownCallback=key_dn;
		mKeyUpCallback=key_up;
		mCharCallback=char_callback;
	}
	
	
	bool Window::performUpdate(float time_increase)
	{
		return mUpdateCallback(time_increase);
	}
	
	void Window::handleKeyEvent(KeyEventType type, unsigned int keycode, unsigned int unicode)
	{
		switch (type) {
			case AKEYEVT_DOWN:
				if (mKeyDownCallback) 
					mKeyDownCallback(keycode);
				break;
			case AKEYEVT_UP:
				if (mKeyUpCallback)
					mKeyUpCallback(keycode);
			default:
				break;
		}
	}
	
	void Window::handleMouseEvent(MouseEventType event,float x,float y, MouseButton button)
	{
		
		switch (event) {
			case AMOUSEEVT_DOWN:
				if (mMouseDownCallback) 
					mMouseDownCallback(x,y,button);
				
				break;
			case AMOUSEEVT_UP:
				if (mMouseUpCallback)   
					mMouseUpCallback(x,y,button);
				break;

			case AMOUSEEVT_MOVE:
				if (mMouseMoveCallback) 
					mMouseMoveCallback(x,y);
				break;
		}
	}
	
	
	
	
}