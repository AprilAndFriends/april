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

static AprilMacOpenGLView* gView = nil;
AprilCocoaWindow* gWindow = nil;
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
		return (gView != NULL && gView->mDisplayLink != nil);
	}

	Mac_Window::Mac_Window() : Window()
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
		if ([[NSScreen mainScreen] respondsToSelector:@selector(backingScaleFactor)])
		{
			this->scalingFactor = [NSScreen mainScreen].backingScaleFactor;
			hlog::writef(logTag, "Mac UI scaling factor: %.2f", this->scalingFactor);
		}
	}

	Mac_Window::~Mac_Window()
	{
		if (gView != NULL)
		{
			[gView destroy];
			[gView release];
			gView = nil;
		}
		if (gWindow != NULL)
		{
			[gWindow destroy];
			[gWindow release];
			gWindow = nil;
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
		gWindow = [[AprilCocoaWindow alloc] initWithContentRect:frame styleMask:styleMask backing: NSBackingStoreBuffered defer:false];
		[gWindow configure];
		setTitle(windowTitle);
		createLoadingOverlay(gWindow);
		if (fullscreen)
		{
			gWindow->mWindowedRect = defaultWndFrame;
			gWindow->mCustomFullscreenExitAnimation = true;
		}
		else
		{
			[gWindow center];
			gWindow->mWindowedRect = gWindow.frame;
		}
		if (lionFullscreen)
		{
			[gWindow setCollectionBehavior:NSWindowCollectionBehaviorFullScreenPrimary];
		}
		[gWindow makeKeyAndOrderFront:gWindow];
		[gWindow setOpaque:YES];
		[gWindow display];
		// A trick to force the window to display as early as possible while we continue with initialization
		[[NSRunLoop currentRunLoop] runUntilDate: [NSDate dateWithTimeIntervalSinceNow:0.1]];
		if (fullscreen)
		{
			if (lionFullscreen)
			{
				[gWindow toggleFullScreen:nil];
				[[NSRunLoop currentRunLoop] runUntilDate: [NSDate dateWithTimeIntervalSinceNow:0.1]];
			}
			else
			{
				[gWindow enterFullScreen];
			}
		}
		gView = [[AprilMacOpenGLView alloc] init];
		[gView initOpenGL];
		if (NSAppKitVersionNumber >= NSAppKitVersionNumber10_7)
		{
			[gView setWantsBestResolutionOpenGLSurface:YES];
		}
		gView.frame = frame;
		[gWindow setOpenGLView: gView];
		if (!isUsingCVDisplayLink())
		{
			[gWindow startRenderLoop];
		}
	}
	
	void Mac_Window::_systemDestroy()
	{
		if (gView != NULL)
		{
			[gView destroy];
			[gView release];
			gView = nil;
		}
		if (gWindow != NULL)
		{
			[gWindow destroy];
			[gWindow release];
			gWindow = nil;
		}
	}
	
	int Mac_Window::getWidth() const
	{
		NSRect bounds = [gWindow.contentView bounds];
		return bounds.size.width * this->scalingFactor;
	}

	int Mac_Window::getHeight() const
	{
		NSRect bounds = [gWindow.contentView bounds];
		return bounds.size.height * this->scalingFactor;
	}
	
	void* Mac_Window::getBackendId() const
	{
		return (void*)gWindow;
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
			this->retainLoadingOverlay = (value == "1");
		}
		else if (param == "reattach_loading_overlay")
		{
			gReattachLoadingOverlay = true;
		}
		else if (param == "fasthide_loading_overlay")
		{
			this->fastHideLoadingOverlay = (value == "1");
		}
		else if (param == "splashscreen_fadeout")
		{
			this->splashScreenFadeout = (value == "1");
		}
		else if (param == "delay_splash")
		{
			this->splashScreenDelay = value;
		}
		else if (param == "disableCursorCheck")
		{
			this->disableCursorCheck = (value == "1");
			if (this->disableCursorCheck)
			{
				hlog::write(logTag, "Disabling Mac Cursor Check");
			}
			else
			{
				hlog::write(logTag, "Enabling Mac Cursor Check");
			}
		}
		else if (param == "displayLinkIgnoreSystemRedraw")
		{
			this->displayLinkIgnoreSystemRedraw = (value == "1");
		}
		else
		{
			Window::setParam(param, value);
		}
	}
	
	bool Mac_Window::isCursorVisible() const
	{
		return (gView == NULL || !gView->mUseBlankCursor);
	}
	
	void Mac_Window::setCursor(Cursor* value)
	{
		if (value != NULL)
		{
			Mac_Cursor* mc = (Mac_Cursor*)value;
			[gView setCursor:mc->getNSCursor()];
		}
		else
		{
			[gView setCursor:NULL];
		}
		[gWindow invalidateCursorRectsForView:gView];
	}

	void Mac_Window::setCursorVisible(bool value)
	{
		if (gView != NULL && this->isCursorVisible() != value)
		{
			[gView setUseBlankCursor:!value];
			[gWindow invalidateCursorRectsForView:gView];
		}
	}
	
	void Mac_Window::setResolution(int width, int height, bool fullscreen)
	{
		if (fullscreen != [gWindow isFullScreen])
		{
			[gWindow platformToggleFullScreen];
		}
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
		if (![gWindow isMiniaturized])
		{
			this->onFocusChanged(true);
		}
		// sometimes MacOS forgets about checking cursor rects, so let's remind it..
		[gWindow invalidateCursorRectsForView:gView];
	}

	void Mac_Window::OnAppLostFocus()
	{
		this->onFocusChanged(false);
	}
	
	void Mac_Window::setTitle(chstr title)
	{
		if (this->fpsCounter)
		{
			hstr fpsTitle = title + hsprintf(" [FPS: %d]", april::application->getFps());
			// optimization to prevent setting title every frame
			if (this->fpsTitle != fpsTitle)
			{
				this->fpsTitle = fpsTitle;
				[gWindow _setTitle:[NSString stringWithUTF8String:this->fpsTitle.cStr()]];
			}
		}
		else
		{
			[gWindow _setTitle:[NSString stringWithUTF8String:title.cStr()]];
		}
		this->title = title;
	}
	
	void Mac_Window::checkEvents()
	{
		if ([[NSScreen mainScreen] respondsToSelector:@selector(backingScaleFactor)])
		{
			float scalingFactor = [NSScreen mainScreen].backingScaleFactor;
			if (scalingFactor != this->scalingFactor)
			{
				this->scalingFactor = scalingFactor;
				[gWindow onWindowSizeChange];
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
		if (this->fpsCounter)
		{
			this->setTitle(this->title);
		}
		return result;
	}
	
	void Mac_Window::_presentFrame()
	{
		// presentFrame() calls are always manually called, so let's make sure
		// Mac can update the view contents before we continue.
		bool displayLink = isUsingCVDisplayLink();
		if (displayLink)
		{
			// kspes@20170319 - Honestly, I don't remember why we needed to do 'ignore-update' functionality
			// it works fine without it. it may have been related to cvdisplay link, but we stopped using it since
			// Sooo, leaving it here like this for now. I need this to be able to draw a loading screen as early as
			// possible.
			this->setIgnoreUpdateFlag(true);
		}
		[gView presentFrame];
		[gView setNeedsDisplay:YES];
		[[NSRunLoop currentRunLoop] runUntilDate: [NSDate dateWithTimeIntervalSinceNow:0.01]];
		if (displayLink)
		{
			this->setIgnoreUpdateFlag(false);
		}
	}
	
	void Mac_Window::queueMessageBox(chstr title, harray<hstr> argButtons, harray<MessageBoxButton> argButtonTypes, chstr text, void (*callback)(MessageBoxButton))
	{
#define ns(s) [NSString stringWithUTF8String:s.cStr()]
		[AprilCocoaWindow showAlertView:ns(title) button1:ns(argButtons[0]) button2:ns(argButtons[1]) button3:ns(argButtons[2]) btn1_t:argButtonTypes[0] btn2_t:argButtonTypes[1] btn3_t:argButtonTypes[2] text:ns(text) callback:callback];
	}
	
	Cursor* Mac_Window::_createCursor(bool fromResource)
	{
		return new Mac_Cursor(fromResource);
	}

	bool Mac_Window::shouldIgnoreUpdate()
	{
		return false;
//		bool ret;
//		hmutex::ScopeLock lock;
//		lock.acquire(&this->ignoreUpdateMutex);
//		ret = this->ignoreUpdate;
//		lock.release();
//		
//		return ret;
	}
	
	void Mac_Window::setIgnoreUpdateFlag(bool value)
	{
		hmutex::ScopeLock lock(&this->ignoreUpdateMutex);
		this->ignoreUpdate = value;
	}
	
}
