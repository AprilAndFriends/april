/// @file
/// @version 3.3
///
/// @section LICENSE
///
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#import <Cocoa/Cocoa.h>

#include <hltypes/hlog.h>
#include "april.h"
#include "Mac_Window.h"
#include "Mac_Cursor.h"
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
		this->ignoreUpdate = false;
		this->name = APRIL_WS_MAC;
		this->retainLoadingOverlay = fastHideLoadingOverlay = false;
		this->splashScreenFadeout = true;
		this->cursorExtensions += ".plist";
		this->cursorExtensions += ".png";
        this->scalingFactor = 1.0f;
        if ([[NSScreen mainScreen] respondsToSelector:@selector(backingScaleFactor)])
        {
            this->scalingFactor = [NSScreen mainScreen].backingScaleFactor;
			hlog::writef(logTag, "Mac UI scaling factor: %.2f", this->scalingFactor);
        }

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
		return [mWindow.contentView bounds].size.width * this->scalingFactor;
	}
	
	int Mac_Window::getHeight()
	{
		return [mWindow.contentView bounds].size.height * this->scalingFactor;
	}
	
	void* Mac_Window::getBackendId()
	{
		return (void*)mWindow;
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
		if (param == "splashscreen_fadeout")
		{
			return this->splashScreenFadeout ? "1" : "0";
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
		if (param == "splashscreen_fadeout")
		{
			this->splashScreenFadeout = (value == "1");
		}
	}
	
	void Mac_Window::updateCursorPosition(gvec2& pos)
	{
		this->cursorPosition = pos * this->scalingFactor;
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
	
	void Mac_Window::setCursor(Cursor* value)
	{
		if (value != NULL)
		{
			Mac_Cursor* mc = (Mac_Cursor*) value;
			[mView setCursor:mc->getNSCursor()];
		}
		else
		{
			[mView setCursor:NULL];
		}
		[mWindow invalidateCursorRectsForView:mView];
	}

	void Mac_Window::setCursorVisible(bool visible)
	{
		if (mView == NULL) return;
		if (visible == isCursorVisible()) return;
		[mView setUseBlankCursor:!visible];
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
		this->fpsCounter = options.fpsCounter;
		this->inputMode = MOUSE;

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
			if (options.resizable) styleMask |= NSResizableWindowMask;
		}

        frame.origin.x /= this->scalingFactor;
        frame.origin.y /= this->scalingFactor;
        frame.size.width /= this->scalingFactor;;
        frame.size.height /= this->scalingFactor;;
        
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
        [mView initOpenGL];
        if (NSAppKitVersionNumber >= NSAppKitVersionNumber10_7)
        {
            [mView setWantsBestResolutionOpenGLSurface:YES];
        }
 		mView.frame = frame;
		[mWindow setOpenGLView: mView];
		[mWindow startRenderLoop];
		return 1;
	}
	
	void Mac_Window::setResolution(int w, int h, bool fullscreen)
	{
		bool state = [mWindow isFullScreen];
		if (fullscreen != state) [mWindow platformToggleFullScreen];
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
		if (this->fpsCounter)
		{
			hstr t = title + hsprintf(" [FPS: %d]", this->fps);
			// optimization to prevent setting title every frame
			if (t == this->fpsTitle) return;
			this->fpsTitle = t;
			[mWindow setTitle:[NSString stringWithUTF8String:t.c_str()]];
		}
		else
		{
			[mWindow setTitle:[NSString stringWithUTF8String:title.c_str()]];
		}
		this->title = title;
	}
	
	bool Mac_Window::updateOneFrame()
	{
		if ([[NSScreen mainScreen] respondsToSelector:@selector(backingScaleFactor)])
        {
            float scalingFactor = [NSScreen mainScreen].backingScaleFactor;
			if (scalingFactor != this->scalingFactor)
			{
				this->scalingFactor = scalingFactor;
				[mWindow onWindowSizeChange];
			}
        }
        bool result = Window::updateOneFrame();
		if (result && mOverlayWindow != nil)
		{
			float timeDelta = this->timer.diff(false);
			if (timeDelta > 0.5f) timeDelta = 0.5f;
			updateLoadingOverlay(timeDelta);
		}
		
		if (this->fpsCounter)
		{
			setTitle(this->title);
		}
		return result;
	}
    
    void Mac_Window::presentFrame()
    {
		// presentFrame() calls are always manually called, so let's make sure
		// Mac can update the view contents before we continue.
		ignoreUpdate = true;
        [mView presentFrame];
		[[NSRunLoop currentRunLoop] runUntilDate: [NSDate dateWithTimeIntervalSinceNow:0.01]];
		ignoreUpdate = false;
    }
	
	Cursor* Mac_Window::_createCursor()
	{
		return new Mac_Cursor();
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
