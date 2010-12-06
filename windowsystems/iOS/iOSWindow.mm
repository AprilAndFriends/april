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
#import <QuartzCore/CAEAGLLayer.h>
#import "EAGLView.h"
#import "AprilViewController.h"
#import "iOSWindow.h"
#import "RenderSystem.h"
#import "AprilUIKitDelegate.h"
#import <hltypes/exception.h>

static AprilUIKitDelegate *appDelegate;
static UIWindow *window;
static EAGLView *glview;
static AprilViewController *viewcontroller;

namespace April
{
    iOSWindow::iOSWindow(int w, int h, bool fullscreen, chstr title)
    {
		appDelegate = ((AprilUIKitDelegate*)[[UIApplication sharedApplication] delegate]);
		viewcontroller = [appDelegate viewController];
		window = [appDelegate window];
		if(fullscreen)
			[UIApplication sharedApplication].statusBarHidden = YES;
		else
			[UIApplication sharedApplication].statusBarHidden = NO;
		mFullscreen = fullscreen;
		
		[window setBackgroundColor:[UIColor blackColor]];
		
		mFirstFrameDrawn = false; // show window after drawing first frame
		
		CGRect frame = window.frame;
		float wi = frame.size.height;
		float hi = frame.size.width;
		frame.size.width = wi;
		frame.size.height = hi;
		glview = [[[EAGLView alloc] initWithFrame:frame] autorelease];
				   
				   
				   //(window.bounds.size.height/2 - w/2, window.bounds.size.width/2 - h/2, h, w)] autorelease]; // FIXME on portrait orientations don't flip window.bounds.size.width and window.bounds.size.height
		if(!glview)
			throw hl_exception("iOSWindow failed to create glview");
		glview.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
		glview.aprilWindowVoid = this;
		
		
		
		[viewcontroller.view addSubview:glview];
		
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
#if __IPHONE_3_2 //__IPHONE_OS_VERSION_MIN_REQUIRED >= 30200
		CAEAGLLayer *caeagllayer = ((CAEAGLLayer*)glview.layer);
		if ([caeagllayer respondsToSelector:@selector(contentsScale)]) {
			return window.bounds.size.height * caeagllayer.contentsScale;
		}
#endif
		
        return window.bounds.size.height;
    }
    int iOSWindow::getWindowHeight()
    {
		// TODO dont swap width and height in case display is in portrait mode
#if __IPHONE_3_2 //__IPHONE_OS_VERSION_MIN_REQUIRED >= 30200
		CAEAGLLayer *caeagllayer = ((CAEAGLLayer*)glview.layer);
		if ([caeagllayer respondsToSelector:@selector(contentsScale)]) {
			return window.bounds.size.width * caeagllayer.contentsScale;
		}
#endif
        return window.bounds.size.width;
    }

    void iOSWindow::setWindowTitle(chstr title)
    {
        // no effect on iOS
    }
	
    gtypes::Vector2 iOSWindow::getCursorPos()
    {
        return gtypes::Vector2(mCursorX,mCursorY);
    }
	
    void iOSWindow::presentFrame()
    {
		if(mFirstFrameDrawn)
		{
			[glview swapBuffers];
		}
		else
		{
			[glview setNeedsDisplay];
			[glview swapBuffers];

			doEvents();

			if([[window subviews] count])
			{/*
				id defaultImageView = [[window subviews] objectAtIndex:0];
				if(defaultImageView && [defaultImageView isKindOfClass:[UIImageView class]])
				{
					[defaultImageView removeFromSuperview];
				}*/
			}
			mFirstFrameDrawn = true;
			
			
		}
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
		
		width;height;
		
		CGPoint location = [touch locationInView:glview];
		
		//For "primary" landscape orientation, this is how we calc it
		mCursorX=location.x; 
		mCursorY=location.y;
		
#if __IPHONE_3_2 //__IPHONE_OS_VERSION_MIN_REQUIRED >= 30200
		CAEAGLLayer* caeagllayer = (CAEAGLLayer*)[glview layer];
		if ([caeagllayer respondsToSelector:@selector(contentsScale)]) {
			mCursorX *= [caeagllayer contentsScale];
			mCursorY *= [caeagllayer contentsScale];
		}
#endif
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
	
	void iOSWindow::beginKeyboardHandling()
	{
		[glview beginKeyboardHandling];
	}
	void iOSWindow::terminateKeyboardHandling()
	{
		[glview terminateKeyboardHandling];
	}
	
	bool iOSWindow::textField_shouldChangeCharactersInRange_replacementString_(void* uitextfieldTextField, int nsrangeLocation, int nsrangeLength, chstr str)
	{
		
		if (nsrangeLocation==0 && str.size()==0) {
			// deploy backspace
			handleKeyEvent(AKEYEVT_DOWN, AK_BACK, 8);
			handleKeyEvent(AKEYEVT_UP, AK_BACK, 8);
		}
		else if (str.size())
		{
			int inputChar = str[0];

			//if(isalnum(inputChar) || inputChar == ' ')
			if(inputChar >= 32 && inputChar <= 127)
			{
				// deploy keypress
				April::KeySym keycode = AK_NONE; // FIXME incorrect, might cause a nasty bug. 
												 // however, writing a translation table atm 
												 // isn't the priority.
			
				handleKeyEvent(AKEYEVT_DOWN, keycode, inputChar);
				handleKeyEvent(AKEYEVT_UP, keycode, inputChar);
			}
		}
		return NO;
	}
	
	void iOSWindow::keyboardWasShown()
	{
		if(mVKeyboardCallback)
			mVKeyboardCallback(true);
	}
	void iOSWindow::keyboardWasHidden()
	{
		if(mVKeyboardCallback)
			mVKeyboardCallback(false);
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