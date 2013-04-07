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
#import "Mac_CocoaWindow.h"
#import "Mac_LoadingOverlay.h"
// declared here instead of as class properties because C++ doesn't play nicely with forward declared objc classes

float getMacOSVersion();

static AprilMacOpenGLView* mView = nil;
static AprilCocoaWindow* mWindow = nil;
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
	
	void Mac_Window::updateCursorPosition(gvec2& pos)
	{
		this->cursorPosition = pos;
	}

	bool Mac_Window::create(int w, int h, bool fullscreen, chstr title, chstr options)
	{
		NSRect frame;
		NSUInteger styleMask;
		
		bool lionFullscreen = getMacOSVersion() >= 10.7f;
		if (fullscreen)
		{
			frame = [[NSScreen mainScreen] frame];
			styleMask = NSBorderlessWindowMask;

			[[NSApplication sharedApplication] setPresentationOptions:
				 NSFullScreenWindowMask |
				 NSApplicationPresentationAutoHideMenuBar |
				 NSApplicationPresentationHideDock |
				 NSApplicationPresentationDisableMenuBarTransparency];
		}
		else
		{
			frame = NSMakeRect(0, 0, w, h);
			styleMask = NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask;
		}
		mWindow = [[AprilCocoaWindow alloc] initWithContentRect:frame styleMask:styleMask backing: NSBackingStoreBuffered defer:false];
		[mWindow configure];
		if (fullscreen)
		{
			[mWindow setLevel:NSMainMenuWindowLevel - 1];
			[mWindow setHidesOnDeactivate:YES];
		}
		else
		{
			[mWindow center];
		}
		if (lionFullscreen)
			[mWindow setCollectionBehavior:NSWindowCollectionBehaviorFullScreenPrimary];

		createLoadingOverlay(mWindow);

		[mWindow makeKeyAndOrderFront:mWindow];
		[mWindow setOpaque:YES];
		[mWindow display];
		// A trick to force the window to display as early as possible while we continue with initialization
		if (fullscreen)
		{
			if (lionFullscreen)
			{
				[[NSRunLoop currentRunLoop] runUntilDate: [NSDate dateWithTimeIntervalSinceNow:0.1]];
				[mWindow toggleFullScreen:nil];
				[[NSRunLoop currentRunLoop] runUntilDate: [NSDate dateWithTimeIntervalSinceNow:2.0]];
			}
		}
		else
		{
			[[NSRunLoop currentRunLoop] runUntilDate: [NSDate dateWithTimeIntervalSinceNow:0.1]];
		}
		mView = [[AprilMacOpenGLView alloc] init];
		mView.frame = frame;
		mWindow.contentView = mView;
		[mWindow startRenderLoop];
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
