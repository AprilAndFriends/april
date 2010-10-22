/************************************************************************************\
This source file is part of the Awesome Portable Rendering Interface Library         *
For latest info, see http://libapril.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Ivan Vucica (ivan@vucica.net)                                     *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/

#ifdef __APPLE__
#include <TargetConditionals.h>
#endif

#ifdef _WIN32
#include <windows.h>
#endif


#if !defined(__APPLE__) || defined(__APPLE__) && (TARGET_OS_MAC) && !(TARGET_OS_IPHONE)
	#ifdef HAVE_GLUT
		#include "GLUTWindow.h"
	#endif
	#ifdef HAVE_SDL
		#include "SDLWindow.h"
	#endif
	#ifdef _WIN32
		#include "Win32Window.h"
	#endif
#elif (TARGET_OS_IPHONE)
	#import <UIKit/UIKit.h>
	#include "iOSWindow.h"
#endif


#include "Window.h"
#include "Keys.h"
#include "RenderSystem.h"

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
		mQuitCallback=0;
		mFocusCallback=0;
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
	void Window::setQuitCallback(bool (*quit_callback)(bool))
	{
		mQuitCallback = quit_callback;
	}
	void Window::setWindowFocusCallback(void (*focus_callback)(bool))
	{
		mFocusCallback = focus_callback;
	}
	
	bool Window::isFullscreen()
	{
		return mFullscreen;
	}
	
	bool Window::performUpdate(float time_increase)
	{
		return mUpdateCallback(time_increase);
	}
	
	void Window::handleKeyEvent(KeyEventType type, KeySym keycode, unsigned int unicode)
	{
		if(keycode == AK_UNKNOWN)
		{
			mRenderSystem->logMessage("key event on unknown key");
			keycode = AK_NONE;
		}
		
		switch (type) {
			case AKEYEVT_DOWN:
				if (mKeyDownCallback && keycode != AK_NONE) 
					mKeyDownCallback(keycode);

				if(unicode && mCharCallback)
					mCharCallback(unicode);
				break;
			case AKEYEVT_UP:
				if (mKeyUpCallback && keycode != AK_NONE)
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
	
	bool Window::handleQuitRequest(bool can_reject)
	{
		// returns whether or not the windowing system is permitted to close the window
		if(mQuitCallback)
		{
			return mQuitCallback(can_reject);
		}
		return true;
	}
	
	
	void Window::handleFocusEvent(bool has_focus)
	{
		if(has_focus)
		{
			printf("GAINED FOCUS\n");
		}
		else 
		{
			printf("LOST FOCUS\n");
		}

		if(mFocusCallback)
		{
			mFocusCallback(has_focus);
		}
	}
	
	void Window::beginKeyboardHandling()
	{
		// ignore by default
		// used only for softkeyboards, e.g. iOS
	}
	void Window::terminateKeyboardHandling()
	{
		// ignore by default
		// used only for softkeyboards, e.g. iOS
	}
	
	///////////////////////
	// non members
	///////////////////////
	
	Window* createAprilWindow(chstr winsysname, int w, int h, bool fullscreen, chstr title)
	{
#if !defined(__APPLE__) || defined(__APPLE__) && (TARGET_OS_MAC) && !(TARGET_OS_IPHONE)
		// desktop
	#ifdef HAVE_SDL
		if (winsysname=="SDL") {
			return new SDLWindow(w,h,fullscreen,title);
		}
	#endif
	#ifdef _WIN32
		if(winsysname=="Win32") {
			return new Win32Window(w,h,fullscreen,title);
		}
	#endif
	#ifdef HAVE_GLUT
		return new GLUTWindow(w,h,fullscreen,title);
	#endif
		
#elif (TARGET_OS_IPHONE)
		// iOS
		return new iOSWindow(w,h,fullscreen,title);
#endif
		return 0;
	}
	
	
	gtypes::Vector2 getDesktopResolution()
	{
#ifdef _WIN32
		return gtypes::Vector2(GetSystemMetrics(SM_CXSCREEN),GetSystemMetrics(SM_CYSCREEN));
#elif (TARGET_OS_IPHONE)
		
		UIScreen* mainScreen = [UIScreen mainScreen];
		float scale=1;
		
		if ([mainScreen respondsToSelector:@selector(scale:)]) {
			scale = [mainScreen scale];
		}
		
		
		
		// TODO dont flip width and height when in portrait mode
		int w = [UIScreen mainScreen].bounds.size.height * scale;
		int h = [UIScreen mainScreen].bounds.size.width * scale;
		return gtypes::Vector2(w, h);
#else
		return gtypes::Vector2(1024,768);
#endif
	}
	
}
