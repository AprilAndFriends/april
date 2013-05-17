/// @file
/// @author  Kresimir Spes
/// @version 3.0
///
/// @section LICENSE
///
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#import <Cocoa/Cocoa.h>

#include <hltypes/hlog.h>
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

extern bool g_WindowFocusedBeforeSleep;

namespace april
{
	float getMacOSVersion();
}

bool isPreLion()
{
	static bool result = april::getMacOSVersion() < 10.7f;
	return result;
}

bool isLionOrNewer()
{
	static bool result = april::getMacOSVersion() >= 10.7f;
	return result;
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
		this->retainLoadingOverlay = fastHideLoadingOverlay = false;
		
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
		if (param == "fasthide_loading_overlay")
		{
			return this->fastHideLoadingOverlay ? "1" : "0";
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
		if (param == "fasthide_loading_overlay")
		{
			this->fastHideLoadingOverlay = (value == "1");
		}
	}
	
	void Mac_Window::updateCursorPosition(gvec2& pos)
	{
		this->cursorPosition = pos;
	}
	
	bool Mac_Window::isCursorInside()
	{
		if (mView != NULL && mView->mUseBlankCursor) return [NSCursor currentCursor] == mView->mBlankCursor;
		else return Window::isCursorInside();
	}
	
	bool Mac_Window::isCursorVisible()
	{
		if (mView == NULL) return 1;
		return !mView->mUseBlankCursor;
	}
	
	void Mac_Window::setCursorVisible(bool visible)
	{
		if (mView == NULL) return;
		if (visible != mView->mUseBlankCursor) return;
		if (visible) [mView setDefaultCursor];
		else         [mView setBlankCursor];
		[mWindow invalidateCursorRectsForView:mView];
	}
	
	bool Mac_Window::create(int w, int h, bool fullscreen, chstr title, Window::Options options)
	{
		if (!Window::create(w, h, fullscreen, title, options))
		{
			return false;
		}

		NSRect frame, defaultWndFrame;
		NSUInteger styleMask;
		
		bool lionFullscreen = isLionOrNewer();

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
			mWindow->mCustomFullscreenExitAnimation = true;
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
	
	void Mac_Window::onFocusChanged(bool value)
	{
		if (value == false && g_WindowFocusedBeforeSleep)
		{
#ifdef _DEBUG
			hlog::write(april::logTag, "Application lost focus while going to sleep, canceling focus on wake.");
#endif
			g_WindowFocusedBeforeSleep = false;
		}
		if (this->focused != value)
		{
			handleFocusChangeEvent(value);
		}
	}
	
	void Mac_Window::OnAppGainedFocus()
	{
		if (![mWindow isMiniaturized]) onFocusChanged(1);
		// sometimes MacOS forgets about checking cursor rects, so let's remind it..
		[mWindow invalidateCursorRectsForView:mView];
	}

	void Mac_Window::OnAppLostFocus()
	{
		onFocusChanged(0);
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
