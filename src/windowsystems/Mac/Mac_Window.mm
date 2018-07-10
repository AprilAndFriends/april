/// @file
/// @version 5.0
///
/// @section LICENSE
///
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#import <Cocoa/Cocoa.h>

#include <hltypes/hlog.h>

#import "Mac_OpenGLView.h"
#import "Mac_CocoaWindow.h"
#import "Mac_LoadingOverlay.h"
#include "Application.h"
#include "april.h"
#include "Mac_Window.h"
#include "Mac_Cursor.h"
#include "SystemDelegate.h"
// declared here instead of as class properties because C++ doesn't play nicely with forward declared objc classes

bool gReattachLoadingOverlay = false;

extern bool g_WindowFocusedBeforeSleep;

bool isPreLion()
{
	return !isLionOrNewer();
}

bool isLionOrNewer()
{
	static int result = -1;
	if (result == -1)
	{
		hversion version = april::getSystemInfo().osVersion;
		result = (version.major >= 10 && version.minor >= 7 ? 1 : 0);
	}
	return (result == 1);
}

namespace april
{
	bool usingCvDisplayLink = false; // purposely exposed like this so it can be overrided as extern in other porjects if needed. for now without an api.
	
	bool isUsingCVDisplayLink()
	{
		return usingCvDisplayLink;
	}
	
	bool hasDisplayLinkThreadStarted()
	{
		return (april::macGlView != NULL && april::macGlView->mDisplayLink != nil);
	}
	
	Mac_Window::Mac_Window() :
		Window()
	{
		this->ignoreUpdate = false;
		this->name = april::WindowType::Mac.getName();
		this->retainLoadingOverlay = fastHideLoadingOverlay = false;
		this->splashScreenFadeout = true;
		this->disableCursorCheck = false;
		this->displayLinkIgnoreSystemRedraw = false;
		this->cursorExtensions += ".plist";
		this->cursorExtensions += ".png";
		this->scalingFactor = 1.0f;
		this->width = 0;
		this->height = 0;
		if ([[NSScreen mainScreen] respondsToSelector:@selector(backingScaleFactor)])
		{
			this->scalingFactor = [NSScreen mainScreen].backingScaleFactor;
			hlog::writef(logTag, "Mac UI scaling factor: %.2f", this->scalingFactor);
		}
	}

	Mac_Window::~Mac_Window()
	{
		if (april::macGlView != NULL)
		{
			[april::macGlView destroy];
			[april::macGlView release];
			april::macGlView = nil;
		}
		if (april::macCocoaWindow != NULL)
		{
			[april::macCocoaWindow destroy];
			[april::macCocoaWindow release];
			april::macCocoaWindow = nil;
		}
	}
	
	void Mac_Window::_systemCreate(int width, int height, bool fullscreen, chstr title, Window::Options options)
	{
		hstr windowTitle = title;
		if (title == "")
		{
			NSString* nsTitle = [[[NSBundle mainBundle] localizedInfoDictionary] objectForKey:@"CFBundleDisplayName"];
			if (nsTitle == nil)
			{
				windowTitle = "UNDEFINED TITLE";
			}
			else
			{
				windowTitle = [nsTitle UTF8String];
			}
		}
		Window::_systemCreate(width, height, fullscreen, windowTitle, options);
		NSRect frame;
		NSRect defaultWndFrame;
		NSUInteger styleMask;
		bool lionFullscreen = isLionOrNewer();
		this->fpsCounter = options.fpsCounter;
		this->inputMode = InputMode::Mouse;
		this->displayLinkIgnoreSystemRedraw = options.mac_displayLinkIgnoreSystemRedraw;
		if (fullscreen)
		{
			frame = [[NSScreen mainScreen] frame];
			styleMask = NSBorderlessWindowMask;
			float factor = options.defaultWindowModeResolutionFactor;
			float tw = frame.size.width * factor, th = frame.size.height * factor;
			defaultWndFrame = NSMakeRect(frame.origin.x + (frame.size.width - tw) / 2.0f, frame.origin.y + (frame.size.height - th) / 2.0f, tw, th);
		}
		else
		{
			frame = NSMakeRect(0, 0, width / this->scalingFactor, height / this->scalingFactor);
			styleMask = NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask;
			if (options.resizable)
			{
				styleMask |= NSResizableWindowMask;
			}
		}
		april::macCocoaWindow = [[AprilCocoaWindow alloc] initWithContentRect:frame styleMask:styleMask backing: NSBackingStoreBuffered defer:false];
		[april::macCocoaWindow configure];
		this->setTitle(windowTitle);
		createLoadingOverlay(april::macCocoaWindow);
		if (fullscreen)
		{
			april::macCocoaWindow->mWindowedRect = defaultWndFrame;
			april::macCocoaWindow->mCustomFullscreenExitAnimation = true;
		}
		else
		{
			[april::macCocoaWindow center];
			april::macCocoaWindow->mWindowedRect = april::macCocoaWindow.frame;
		}
		if (lionFullscreen)
		{
			[april::macCocoaWindow setCollectionBehavior:NSWindowCollectionBehaviorFullScreenPrimary];
		}
		if (!april::window->getOptions().minimized)
		{
			[april::macCocoaWindow makeKeyAndOrderFront:april::macCocoaWindow];
		}
		[april::macCocoaWindow setOpaque:YES];
		[april::macCocoaWindow display];
		NSRect bounds = [april::macCocoaWindow.contentView bounds];
		this->width = bounds.size.width * this->scalingFactor;
		this->height = bounds.size.height * this->scalingFactor;
		// A trick to force the window to display as early as possible while we continue with initialization
		[[NSRunLoop currentRunLoop] runUntilDate: [NSDate dateWithTimeIntervalSinceNow:0.1]];
		if (fullscreen)
		{
			if (lionFullscreen)
			{
				[april::macCocoaWindow toggleFullScreen:nil];
				[[NSRunLoop currentRunLoop] runUntilDate: [NSDate dateWithTimeIntervalSinceNow:0.1]];
			}
			else
			{
				[april::macCocoaWindow enterFullScreen];
			}
		}
		april::macGlView = [[AprilMacOpenGLView alloc] init];
		[april::macGlView initApril];
		if (NSAppKitVersionNumber >= NSAppKitVersionNumber10_7)
		{
			[april::macGlView setWantsBestResolutionOpenGLSurface:YES];
		}
		april::macGlView.frame = frame;
		[april::macCocoaWindow setOpenGLView:april::macGlView];
		if (!isUsingCVDisplayLink())
		{
			[april::macCocoaWindow startRenderLoop];
		}
	}
	
	void Mac_Window::_systemDestroy()
	{
		if (april::macGlView != NULL)
		{
			[april::macGlView destroy];
			[april::macGlView release];
			april::macGlView = nil;
		}
		if (april::macCocoaWindow  != NULL)
		{
			[april::macCocoaWindow  destroy];
			[april::macCocoaWindow  release];
			april::macCocoaWindow  = nil;
		}
	}
	
	void* Mac_Window::getBackendId() const
	{
		return (void*)april::macCocoaWindow;
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
		if (param == "delay_splash")
		{
			return this->splashScreenDelay;
		}
		if (param == "disableCursorCheck")
		{
			return this->disableCursorCheck ? "1" : "0";
		}
		if (param == "displayLinkIgnoreSystemRedraw")
		{
			return this->displayLinkIgnoreSystemRedraw  ? "1" : "0";
		}
		return Window::getParam(param);
	}
	
	void Mac_Window::setParam(chstr param, chstr value)
	{
		if (param == "retain_loading_overlay")
		{
			this->retainLoadingOverlay = (value == "1"); // TODO - should use true/false
			return;
		}
		if (param == "reattach_loading_overlay")
		{
			gReattachLoadingOverlay = true;
			return;
		}
		if (param == "fasthide_loading_overlay")
		{
			this->fastHideLoadingOverlay = (value == "1"); // TODO - should use true/false
			return;
		}
		if (param == "splashscreen_fadeout")
		{
			this->splashScreenFadeout = (value == "1"); // TODO - should use true/false
			return;
		}
		if (param == "delay_splash")
		{
			this->splashScreenDelay = (float)value;
			return;
		}
		if (param == "disableCursorCheck")
		{
			this->disableCursorCheck = (value == "1"); // TODO - should use true/false
			if (this->disableCursorCheck)
			{
				hlog::write(logTag, "Disabling Mac Cursor Check");
			}
			else
			{
				hlog::write(logTag, "Enabling Mac Cursor Check");
			}
			return;
		}
		if (param == "displayLinkIgnoreSystemRedraw")
		{
			this->displayLinkIgnoreSystemRedraw = (value == "1"); // TODO - should use true/false
			return;
		}
		Window::setParam(param, value);
	}
	
	bool Mac_Window::isCursorVisible() const
	{
		return (april::macGlView == NULL || !april::macGlView->mUseBlankCursor);
	}
	
	void Mac_Window::setCursor(Cursor* value)
	{
		if (value != NULL)
		{
			Mac_Cursor* cursor = (Mac_Cursor*)value;
			[april::macGlView setCursor:cursor->getNSCursor()];
		}
		else
		{
			[april::macGlView setCursor:NULL];
		}
		[april::macCocoaWindow invalidateCursorRectsForView:april::macGlView];
	}

	void Mac_Window::setCursorVisible(bool value)
	{
		if (april::macGlView != NULL && this->isCursorVisible() != value)
		{
			[april::macGlView setUseBlankCursor:!value];
			[april::macCocoaWindow invalidateCursorRectsForView:april::macGlView];
		}
	}
	
	void Mac_Window::_systemSetResolution(int width, int height, bool fullscreen)
	{
		if (fullscreen != [april::macCocoaWindow isFullScreen])
		{
			[april::macCocoaWindow platformToggleFullScreen];
		}
	}
	
	void Mac_Window::setSystemWindowSize(int width, int height)
	{
		this->width = width;
		this->height = height;
	}
	
	void Mac_Window::setFullscreenFlag(bool value)
	{
		this->fullscreen = value;
	}
	
	void Mac_Window::onFocusChanged(bool value)
	{
		if (!value && g_WindowFocusedBeforeSleep)
		{
#ifdef _DEBUG
			hlog::write(logTag, "Application lost focus while going to sleep, canceling focus on wake.");
#endif
			g_WindowFocusedBeforeSleep = false;
		}
		value ? april::application->resume() : april::application->suspend();
		this->queueFocusChange(value);
	}
	
	void Mac_Window::OnAppGainedFocus()
	{
		if (![april::macCocoaWindow isMiniaturized])
		{
			this->onFocusChanged(true);
		}
		// sometimes MacOS forgets about checking cursor rects, so let's remind it..
		[april::macCocoaWindow invalidateCursorRectsForView:april::macGlView];
	}

	void Mac_Window::OnAppLostFocus()
	{
		this->onFocusChanged(false);
	}
	
	void Mac_Window::setTitle(chstr title)
	{		
		this->title = title;
	}
	
	void Mac_Window::checkEvents()
	{
		NSScreen* mainScreen = april::macCocoaWindow.screen;
		if (mainScreen == nil)
		{
			mainScreen = [NSScreen mainScreen];
		}
		if ([mainScreen respondsToSelector:@selector(backingScaleFactor)])
		{
			float scalingFactor = mainScreen.backingScaleFactor;
			if (scalingFactor != this->scalingFactor)
			{
				this->scalingFactor = scalingFactor;
				[april::macCocoaWindow  onWindowSizeChange];
			}
		}
		Window::checkEvents();
	}
	
	bool Mac_Window::update(float timeDelta)
	{
		if (this->shouldIgnoreUpdate())
		{
			return true;
		}
		bool result = Window::update(timeDelta);
		if (result && gOverlayWindow != nil)
		{
			updateLoadingOverlay(timeDelta);
		}		
		return result;
	}
	
	void Mac_Window::_presentFrame(bool systemEnabled)
	{
		Window::_presentFrame(systemEnabled);
		bool displayLink = isUsingCVDisplayLink();
		if (displayLink)
		{
			// kspes@20170319 - Honestly, I don't remember why we needed to do 'ignore-update' functionality
			// it works fine without it. it may have been related to cvdisplay link, but we stopped using it since
			// Sooo, leaving it here like this for now. I need this to be able to draw a loading screen as early as
			// possible.
			this->setIgnoreUpdateFlag(true);
		}
		[april::macGlView presentFrame];
		//[april::macGlView setNeedsDisplay:YES];
		if (displayLink)
		{
			this->setIgnoreUpdateFlag(false);
		}
	}
	
	Cursor* Mac_Window::_createCursor(bool fromResource)
	{
		return new Mac_Cursor(fromResource);
	}

	bool Mac_Window::shouldIgnoreUpdate()
	{
		return false;
		/*
		hmutex::ScopeLock lock(&this->ignoreUpdateMutex);
		return this->ignoreUpdate;
		*/
	}
	
	void Mac_Window::setIgnoreUpdateFlag(bool value)
	{
		hmutex::ScopeLock lock(&this->ignoreUpdateMutex);
		this->ignoreUpdate = value;
	}
	
}
