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
#import "AprilViewController.h"
#import "EAGLView.h"
#import "iOSWindow.h"
#import "RenderSystem.h"
#import "AprilUIKitDelegate.h"
#import <hltypes/exception.h>

static AprilUIKitDelegate *appDelegate;
static UIWindow *window;
static EAGLView *glview;
static AprilViewController *viewcontroller;

namespace april
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
				
		mFirstFrameDrawn = false; // show window after drawing first frame
		
		CGRect frame = window.frame;
		float wi = frame.size.height;
		float hi = frame.size.width;
		
		if ([[UIScreen mainScreen] respondsToSelector:@selector(scale)])
		{
			frame.size.width = wi;
			frame.size.height = hi;
		}
		
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
    int iOSWindow::getWidth()
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
    int iOSWindow::getHeight()
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
	
    gtypes::Vector2 iOSWindow::getCursorPosition()
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
			{
				id defaultImageView = [[window subviews] objectAtIndex:0];
				if(defaultImageView && [defaultImageView isKindOfClass:[UIImageView class]])
				{
					[glview removeFromSuperview];
					[defaultImageView removeFromSuperview];
					
					[viewcontroller setView:glview];
					[window addSubview:glview];
				}
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
		
#pragma unused(width)
#pragma unused(height)
		
		CGPoint location = [touch locationInView:glview];
		
		//For "primary" landscape orientation, this is how we calc it
		mCursorX=location.x; 
		mCursorY=location.y;
		
		// detect pre-3.2 devices
		if (![[UIScreen mainScreen] respondsToSelector:@selector(scale)])
		{
			// ... except on pre-3.2 devices where we did a
			// FIXME hack with locking orientation, and the
			// gui stays incorrectly rotated.
			mCursorX = height-location.y;
			mCursorY = location.x;
		}
		
		
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
	float iOSWindow::prefixRotationAngle()
	{
		// this function needs updating if we want
		// support for truly portrait-rendered apps
		
		// currently, in the same way as is the case
		// in rest of april for iOS, we only provide
		// support for landscape orientations
		
		if (![[UIScreen mainScreen] respondsToSelector:@selector(scale)])
		{
			// pre-3.2 devices use a hack
			return 90.0f;
		}
		
		//NSLog(@"Orientation: %d", viewcontroller.interfaceOrientation);
		switch (viewcontroller.interfaceOrientation) {
			case UIInterfaceOrientationPortrait:
				return 90.0f; // simulate left landscape orientation (needed only until we transform into a landscape orientation on earlier iOS)
			case UIInterfaceOrientationLandscapeLeft:
			case UIInterfaceOrientationLandscapeRight:
				return 0.0f; // any landscape orientation will be well supported elsewhere
				
			case UIInterfaceOrientationPortraitUpsideDown:
				return -90.0f; // this shouldn't occur except if someone plays with shouldAutorotateToInterfaceOrientation in AprilViewController
				
			default:
				break;
		}
		return 0;
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
				april::KeySym keycode = AK_NONE; // FIXME incorrect, might cause a nasty bug. 
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
	
	void iOSWindow::setDeviceOrientationCallback(void (*do_callback)(DeviceOrientation))
	{
			
		if(do_callback)
		{
			[[UIDevice currentDevice] beginGeneratingDeviceOrientationNotifications];
		}
		else
		{
			[[UIDevice currentDevice] endGeneratingDeviceOrientationNotifications];
		}
		
		// update mDeviceOrientationCallback
		Window::setDeviceOrientationCallback(do_callback);
		
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
	
	void iOSWindow::deviceOrientationDidChange()
	{
		if(mDeviceOrientationCallback)
		{
			DeviceOrientation newOrientation;
			switch ([[UIDevice currentDevice] orientation]) {
				case UIDeviceOrientationUnknown:
					newOrientation = ADEVICEORIENTATION_NONE;
				case UIDeviceOrientationPortrait:
					newOrientation = ADEVICEORIENTATION_PORTRAIT;
				case UIDeviceOrientationPortraitUpsideDown:
					newOrientation = ADEVICEORIENTATION_PORTRAIT_UPSIDEDOWN;
				case UIDeviceOrientationLandscapeLeft:
					newOrientation = ADEVICEORIENTATION_LANDSCAPE_LEFT;
				case UIDeviceOrientationLandscapeRight:
					newOrientation = ADEVICEORIENTATION_LANDSCAPE_RIGHT;
				case UIDeviceOrientationFaceUp:
					newOrientation = ADEVICEORIENTATION_FACE_UP;
				case UIDeviceOrientationFaceDown:
					newOrientation = ADEVICEORIENTATION_FACE_DOWN;
				// not adding default, so we get a warning in case
				// a new orientation is added to backing API
			}
			newOrientation = ADEVICEORIENTATION_NONE;
			mDeviceOrientationCallback(newOrientation);
		}
	}
	void iOSWindow::applicationWillResignActive()
	{
		if (mFocusCallback) {
			mFocusCallback(false);
		}
	}
	void iOSWindow::applicationDidBecomeActive()
	{
		if (mFocusCallback) {
			mFocusCallback(true);
		}
	}
	
}