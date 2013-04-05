/// @file
/// @author  Kresimir Spes
/// @version 3.0
///
/// @section LICENSE
///
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#import <Cocoa/Cocoa.h>

#include "april.h"
#include "Mac_Window.h"
#import "Mac_OpenGLView.h"
#import "Mac_WindowDelegate.h"
#import "Mac_LoadingOverlay.h"
// declared here instead of as class properties because C++ doesn't play nicely with forward declared objc classes

static AprilMacOpenGLView* mView = nil;
static NSWindow* mWindow = nil;
bool gReattachLoadingOverlay = false;

namespace april
{
    Mac_Window::Mac_Window() : Window()
    {
		this->name = APRIL_WS_MAC;
		this->retainLoadingOverlay = false;
    }

    Mac_Window::~Mac_Window()
    {
		if (mView)
		{
			[mView release];
			mView = nil;
		}
        if (mWindow)
		{
			[mWindow.delegate release];
			mWindow.delegate = nil;
			[mWindow release];
			mWindow = nil;
		}
    }
	
	int Mac_Window::getWidth()
	{
		return [mWindow.contentView bounds].size.width;
	}
	
	int Mac_Window::getHeight()
	{
		return [mWindow.contentView bounds].size.height;
	}
	
	void* Mac_Window::getBackendId()
	{
		return (void*) mWindow;
	}

	hstr Mac_Window::getParam(chstr param)
	{
		if (param == "retain_loading_overlay")
		{
			return this->retainLoadingOverlay ? "1" : "0";
		}
		return "";
	}
	
	void Mac_Window::setParam(chstr param, chstr value)
	{
		if (param == "retain_loading_overlay")
		{
			this->retainLoadingOverlay = (value == "1");
		}
		if (param == "reattach_loading_overlay")
		{
			gReattachLoadingOverlay = true;
		}
	}

	bool Mac_Window::create(int w, int h, bool fullscreen, chstr title, chstr options)
	{
		NSRect frame = NSMakeRect(0, 0, 1280, 800);
		NSUInteger styleMask = NSTitledWindowMask | NSMiniaturizableWindowMask;
		mWindow = [[NSWindow alloc] initWithContentRect:frame styleMask:styleMask backing: NSBackingStoreBuffered defer:false];
		[mWindow setBackgroundColor:[NSColor blackColor]];
		[mWindow setDelegate:[[AprilMacWindowDelegate alloc] init]];

//		[mWindow setContentSize:frame.size];

		createLoadingOverlay(mWindow);		
		/////
		[mWindow center];
		[mWindow makeKeyAndOrderFront:mWindow];
		[mWindow display];
//        [mWindow setLevel: NSScreenSaverWindowLevel-1];

		// A trick to force the window to display as early as possible while we continue with
		// initialization
		[[NSRunLoop currentRunLoop] runUntilDate: [NSDate dateWithTimeIntervalSinceNow:0.1]];
		mView = [[AprilMacOpenGLView alloc] init];
		mView.frame = frame;
//		[mWindow.contentView addSubview: mView];
		mWindow.contentView = mView;
	//	[mView addSubview:mImageView];


		return 1;
	}
	
	bool Mac_Window::destroy()
	{
        if (mView)   { [mView release];   mView   = nil; }
        if (mWindow) { [mWindow release]; mWindow = nil; }

		return 1;
	}
	
	void Mac_Window::setTitle(chstr title)
	{
	
	}
	
	bool Mac_Window::isCursorVisible()
	{
		return 1;
	}
	
	void Mac_Window::setCursorVisible(bool visible)
	{
		
	}
	
	gvec2 Mac_Window::getCursorPosition()
	{
		gvec2 pt;
		return pt;
	}

	bool Mac_Window::updateOneFrame()
	{
        Window::updateOneFrame();
		if (mOverlayWindow != nil)
		{
			float k = this->timer.diff(false);
			if (k > 0.5f) k = 0.05f;
			updateLoadingOverlay(k);
		}
		return 1;
	}
    
    void Mac_Window::presentFrame()
    {
        [mView presentFrame];
    }
}
