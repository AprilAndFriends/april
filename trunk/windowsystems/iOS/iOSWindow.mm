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
		
		[window setBackgroundColor:[UIColor blueColor]];
		[window makeKeyAndVisible];
		
		glview = [[[EAGLView alloc] initWithFrame:[window bounds]] autorelease];
		glview.aprilWindow = this;
		[window addSubview:glview];

		
		mRunning = true;
    }
    
    void iOSWindow::enterMainLoop()
    {
		while (mRunning) 
		{
			// parse UIKit events
			SInt32 result;
			do {
				result = CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0, TRUE);
			} while(result == kCFRunLoopRunHandledSource);
		
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
        return window.bounds.size.width;
    }
    int iOSWindow::getWindowHeight()
    {
        return window.bounds.size.height;
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