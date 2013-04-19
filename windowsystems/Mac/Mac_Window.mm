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

static AprilMacOpenGLView* mView = nil;
static AprilCocoaWindow* mWindow = nil;
bool gReattachLoadingOverlay = false;
april::Mac_Window* aprilWindow = NULL;

float getMacOSVersion()
{
#ifdef _DEBUG
//	return 10.6f; // uncomment this to test behaviour on older macs
#endif

	static float version = 0;
	if (version == 0)
	{
		SInt32 major, minor;
		if (Gestalt(gestaltSystemVersionMajor, &major) == noErr && Gestalt(gestaltSystemVersionMinor, &minor) == noErr)
		{
			version = major + minor / 10.0f;
		}
		else version = 10.3f; // just in case. < 10.4 is not supported.
	}
	return version;
}

namespace april
{
    Mac_Window::Mac_Window() : Window()
    {
#ifdef _DEBUG
		this->mResizable = 1;
#else
		this->mResizable = 0;
#endif
		this->name = APRIL_WS_MAC;
		this->retainLoadingOverlay = false;
		
		aprilWindow = this;
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
			[mWindow destroy];
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
	
	bool Mac_Window::isCursorVisible()
	{
		return ![mView isBlankCursorUsed];
	}
	
	void Mac_Window::setCursorVisible(bool visible)
	{
		if (visible != [mView isBlankCursorUsed]) return;

		[mWindow invalidateCursorRectsForView:mView];
		if (visible) [mView setDefaultCursor];
		else         [mView setBlankCursor];
	}
	
	bool Mac_Window::create(int w, int h, bool fullscreen, chstr title, chstr options)
	{
		if (!Window::create(w, h, fullscreen, title, options))
		{
			return false;
		}

		NSRect frame, defaultWndFrame;
		NSUInteger styleMask;
		
		bool lionFullscreen = getMacOSVersion() >= 10.7f;

		if (fullscreen)
		{
			frame = [[NSScreen mainScreen] frame];
			styleMask = NSBorderlessWindowMask;
			float tw = frame.size.width * 2.0f / 3.0f, th = frame.size.height * 2.0f / 3.0f;
			defaultWndFrame = NSMakeRect(frame.origin.x + (frame.size.width - tw) / 2.0f, frame.origin.y + (frame.size.height - th) / 2.0f, tw, th);
		}
		else
		{
			frame = NSMakeRect(0, 0, w, h);
			styleMask = NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask;
			if (aprilWindow->mResizable) styleMask |= NSResizableWindowMask;
		}

		mWindow = [[AprilCocoaWindow alloc] initWithContentRect:frame styleMask:styleMask backing: NSBackingStoreBuffered defer:false];
		[mWindow configure];
		setTitle(title);
		createLoadingOverlay(mWindow);

		if (fullscreen)
		{
			mWindow->mWindowedRect = defaultWndFrame;
		}
		else
		{
			[mWindow center];
			mWindow->mWindowedRect = mWindow.frame;
		}
		if (lionFullscreen)
			[mWindow setCollectionBehavior:NSWindowCollectionBehaviorFullScreenPrimary];

		[mWindow makeKeyAndOrderFront:mWindow];
		[mWindow setOpaque:YES];
		[mWindow display];
		// A trick to force the window to display as early as possible while we continue with initialization
		[[NSRunLoop currentRunLoop] runUntilDate: [NSDate dateWithTimeIntervalSinceNow:0.1]];
		if (fullscreen)
		{
			if (lionFullscreen)
			{
				[mWindow toggleFullScreen:nil];
				[[NSRunLoop currentRunLoop] runUntilDate: [NSDate dateWithTimeIntervalSinceNow:0.1]];
			}
			else
			{
				[mWindow enterFullScreen];
			}
		}
		mView = [[AprilMacOpenGLView alloc] init];
		mView.frame = frame;
		[mWindow setOpenGLView: mView];
		[mWindow startRenderLoop];
		return 1;
	}
	
	void Mac_Window::setFullscreen(bool value)
	{
		Window::setFullscreen(value);
		bool state = [mWindow isFullScreen];
		if (value != state) [mWindow platformToggleFullScreen];
	}
	
	void Mac_Window::setFullscreenFlag(bool value)
	{
		this->fullscreen = value;
	}

	void Mac_Window::setTitle(chstr title)
	{
		Window::setTitle(title);
		[mWindow setTitle:[NSString stringWithUTF8String:title.c_str()]];
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
	
	void Mac_Window::terminateMainLoop()
	{
		[[NSApplication sharedApplication] terminate:nil];
	}
	
	bool Mac_Window::destroy()
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
		
		return 1;
	}
}
