/************************************************************************************\
 This source file is part of the Awesome Portable Rendering Interface Library         *
 For latest info, see http://libapril.sourceforge.net/                                *
 **************************************************************************************
 Copyright (c) 2010 Ivan Vucica (ivan@vucica.net)                                     *
 *                                                                                    *
 * This program is free software; you can redistribute it and/or modify it under      *
 * the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
 \************************************************************************************/

#import <UIKit/UIKit.h>
#import "EAGLView.h"
#include "iOSWindow.h"
#include "RenderSystem.h"

static UIWindow *window;
static EAGLView *glview;

namespace April
{
    iOSWindow::iOSWindow(int w, int h, bool fullscreen, chstr title)
    {
		window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
		if(fullscreen)
			[UIApplication sharedApplication].statusBarHidden = YES;
		else
			[UIApplication sharedApplication].statusBarHidden = NO;
		mFullscreen = fullscreen;
		
		[window setBackgroundColor:[UIColor blueColor]];
		[window makeKeyAndVisible];
		
		glview = [[[EAGLView alloc] initWithFrame:[window bounds]] autorelease];
		glview.aprilWindowVoid = this;

		[window addSubview:glview];

		
		mRunning = true;
    }
    
    void iOSWindow::enterMainLoop()
    {
		while (mRunning) 
		{
			// parse UIKit events
			doEvents();
			handleDisplayAndUpdate();
		}
    }
	
	
    void iOSWindow::terminateMainLoop()
    {
        mRunning = false;
    }
    void iOSWindow::showSystemCursor(bool visible)
    {
        // no effect on iOS
    }
    bool iOSWindow::isSystemCursorShown()
    {
        return false; // iOS never shows system cursor
    }
    int iOSWindow::getWindowWidth()
    {
		// TODO dont swap width and height in case display is in portrait mode
        return window.bounds.size.height;
    }
    int iOSWindow::getWindowHeight()
    {
		// TODO dont swap width and height in case display is in portrait mode
        return window.bounds.size.width;
    }
    void iOSWindow::setWindowTitle(chstr title)
    {
        // no effect on iOS
    }
    gtypes::Vector2 iOSWindow::getCursorPos()
    {
        return gtypes::Vector2(0,0);
    }
    void iOSWindow::presentFrame()
    {
        // dummy
		
//		[glview setNeedsDisplay];
		[glview swapBuffers];
    }
	void* iOSWindow::getIDFromBackend()
	{
		return window;
	}
	void iOSWindow::doEvents()
	{
		SInt32 result;
		do {
			result = CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0, TRUE);
		} while(result == kCFRunLoopRunHandledSource);
	}

	
	void iOSWindow::_convertTouchesToCoordinates(void* nssetTouches)
	{
		// return value stored in mCursorX and mCursorY
		
		NSSet* touches = (NSSet*)nssetTouches;
		
		UITouch* touch = touches.anyObject; 
		float width = glview.bounds.size.width;
		float height = glview.bounds.size.height;
		
		CGPoint location = [touch locationInView:glview];
		
		
		//For "primary" landscape orientation, this is how we calc it
		mCursorX=location.y; 
		mCursorY=width-location.x;

		
	}
	
	void iOSWindow::touchesBegan_withEvent_(void* nssetTouches, void* uieventEvent)
	{
		_convertTouchesToCoordinates(nssetTouches);
		
		Window::MouseEventType mouseevt = AMOUSEEVT_DOWN;
		Window::MouseButton mousebtn = AMOUSEBTN_LEFT;
		
		handleMouseEvent(mouseevt, 
						 mCursorX, mCursorY,
						 mousebtn);
		
	}
	void iOSWindow::touchesEnded_withEvent_(void* nssetTouches, void* uieventEvent)
	{
		_convertTouchesToCoordinates(nssetTouches);
		
		Window::MouseEventType mouseevt = AMOUSEEVT_UP;
		Window::MouseButton mousebtn = AMOUSEBTN_LEFT;
		
		handleMouseEvent(mouseevt, 
						 mCursorX, mCursorY,
						 mousebtn);
		
	}
	
	
	void iOSWindow::touchesCancelled_withEvent_(void* nssetTouches, void* uieventEvent)
	{
		// FIXME needs to cancel touches, not treat them as "release"
		_convertTouchesToCoordinates(nssetTouches);
		
		Window::MouseEventType mouseevt = AMOUSEEVT_UP;
		Window::MouseButton mousebtn = AMOUSEBTN_LEFT;
		
		handleMouseEvent(mouseevt, 
						 mCursorX, mCursorY,
						 mousebtn);
		
	}
	
	
	void iOSWindow::touchesMoved_withEvent_(void* nssetTouches, void* uieventEvent)
	{
		_convertTouchesToCoordinates(nssetTouches);
		
		Window::MouseEventType mouseevt = AMOUSEEVT_MOVE;
		Window::MouseButton mousebtn = AMOUSEBTN_NONE;
		
		handleMouseEvent(mouseevt, 
						 mCursorX, mCursorY,
						 mousebtn);
		
	}
	
	
		//////////////
	void iOSWindow::handleDisplayAndUpdate()
	{
		//static unsigned int x=SDL_GetTicks();
		//float k=(SDL_GetTicks()-x)/1000.0f;
		//x=SDL_GetTicks();
		performUpdate(mTimer.diff(true));
		rendersys->presentFrame();
	}
	
	
}