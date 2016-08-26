/// @file
/// @version 4.1
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
#include "Mac_QueuedEvents.h"
#include "SystemDelegate.h"
// declared here instead of as class properties because C++ doesn't play nicely with forward declared objc classes

static AprilMacOpenGLView* mView = nil;
AprilCocoaWindow* mWindow = nil;
bool gReattachLoadingOverlay = false;
april::Mac_Window* aprilWindow = NULL;

extern bool g_WindowFocusedBeforeSleep;

namespace april
{
	hversion getMacOSVersion();
}

bool isPreLion()
{
	return !isLionOrNewer();
}

bool isLionOrNewer()
{
	static int result = -1;
	if (result == -1)
	{
		hversion v = april::getMacOSVersion();
		result = v.major >= 10 && v.minor >= 7 ? 1 : 0;
	}
	return result == 1;
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
        return mView != NULL && mView->mDisplayLink != nil;
    }

	Mac_Window::Mac_Window() : Window()
	{
		this->ignoreUpdate = false;
		this->name = april::WindowType::Mac.getName();
		this->retainLoadingOverlay = fastHideLoadingOverlay = false;
		this->splashScreenFadeout = true;
		this->displayLinkIgnoreSystemRedraw = false;
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
	
	int Mac_Window::getWidth() const
	{
		NSRect bounds = [mWindow.contentView bounds];
		return bounds.size.width * this->scalingFactor;
	}

	int Mac_Window::getHeight() const
	{
		NSRect bounds = [mWindow.contentView bounds];
		return bounds.size.height * this->scalingFactor;
	}
	
	void* Mac_Window::getBackendId() const
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
		if (param == "delay_splash")
		{
			return this->splashScreenDelay;
		}

		if (param == "displayLinkIgnoreSystemRedraw")
		{
			return this->displayLinkIgnoreSystemRedraw  ? "1" : "0";
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
		if (param == "delay_splash")
		{
			this->splashScreenDelay = value;
		}
		if (param == "displayLinkIgnoreSystemRedraw")
		{
			this->displayLinkIgnoreSystemRedraw = (value == "1");
		}
	}
	
	void Mac_Window::updateCursorPosition(gvec2& pos)
	{
		this->cursorPosition = pos * this->scalingFactor;
	}

	bool Mac_Window::isCursorVisible() const
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
		hstr windowTitle;
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
		else
		{
			windowTitle = title;
		}
		if (!Window::create(w, h, fullscreen, windowTitle, options))
		{
			return false;
		}

		NSRect frame, defaultWndFrame;
		NSUInteger styleMask;

		bool lionFullscreen = isLionOrNewer();
		this->fpsCounter = options.fpsCounter;
		this->inputMode = MOUSE;
		this->displayLinkIgnoreSystemRedraw = options.mac_displayLinkIgnoreSystemRedraw;

		if (fullscreen)
		{
			frame = [[NSScreen mainScreen] frame];
			styleMask = NSBorderlessWindowMask;
			float factor = aprilWindow->getOptions().defaultWindowModeResolutionFactor;
			float tw = frame.size.width * factor, th = frame.size.height * factor;
			defaultWndFrame = NSMakeRect(frame.origin.x + (frame.size.width - tw) / 2.0f, frame.origin.y + (frame.size.height - th) / 2.0f, tw, th);
		}
		else
		{
			frame = NSMakeRect(0, 0, w / this->scalingFactor, h / this->scalingFactor);
			styleMask = NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask;
			if (options.resizable) styleMask |= NSResizableWindowMask;
		}
		
		mWindow = [[AprilCocoaWindow alloc] initWithContentRect:frame styleMask:styleMask backing: NSBackingStoreBuffered defer:false];
		[mWindow configure];
		setTitle(windowTitle);
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
        if (!isUsingCVDisplayLink())
        {
            [mWindow startRenderLoop];
        }
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
			hlog::write(logTag, "Application lost focus while going to sleep, canceling focus on wake.");
#endif
			g_WindowFocusedBeforeSleep = false;
		}
		if (this->focused != value)
		{
            this->focused = value;
            
            if (isUsingCVDisplayLink())
            {
                queueFocusChanged(value);
            }
            else
            {
                handleFocusChangeEvent(value);
            }
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
			[mWindow _setTitle:[NSString stringWithUTF8String:t.cStr()]];
		}
		else
		{
			[mWindow _setTitle:[NSString stringWithUTF8String:title.cStr()]];
		}
		this->title = title;
	}
	
	bool Mac_Window::updateOneFrame()
	{
		if (this->shouldIgnoreUpdate())
		{
			return true;
		}
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
		this->setIgnoreUpdateFlag(true);
		[mView presentFrame];
		[[NSRunLoop currentRunLoop] runUntilDate: [NSDate dateWithTimeIntervalSinceNow:0.01]];
		this->setIgnoreUpdateFlag(false);
	}
	
    void Mac_Window::dispatchQueuedEvents()
    {
        if (this->queuedEvents.size() > 0)
        {
            foreach (QueuedEvent*, it, this->queuedEvents)
            {
                (*it)->execute();
                delete *it;
            }
            this->queuedEvents.clear();
        }
    }
    
    void Mac_Window::queueWindowSizeChanged(int w, int h, bool fullscreen)
    {
        hmutex::ScopeLock lock(&renderThreadSyncMutex);
        this->queuedEvents += new WindowSizeChangedEvent(this, w, h, fullscreen);
    }

    void Mac_Window::queueFocusChanged(bool focused)
    {
        hmutex::ScopeLock lock(&renderThreadSyncMutex);
        this->queuedEvents += new FocusChangedEvent(this, focused);
    }

    void Mac_Window::dispatchWindowSizeChanged(int w, int h, bool fullscreen)
    {
        april::SystemDelegate* delegate = aprilWindow->getSystemDelegate();
        
        if (delegate)
        {
            delegate->onWindowSizeChanged(w, h, fullscreen);
        }
        else
        {
            hlog::write(april::logTag, "Mac_CocoaWindow: Ignoring onWindowSizeChange, delegate not set.");
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
		bool ret;
		hmutex::ScopeLock lock;
		lock.acquire(&this->ignoreUpdateMutex);
		ret = this->ignoreUpdate;
		lock.release();
		
		return ret;
	}
	
	void Mac_Window::setIgnoreUpdateFlag(bool value)
	{
		hmutex::ScopeLock lock;
		lock.acquire(&this->ignoreUpdateMutex);
		this->ignoreUpdate = value;
		lock.release();
	}
	
	void Mac_Window::terminateMainLoop()
	{
        [mWindow terminateMainLoop];
	}

	bool Mac_Window::destroy()
	{
		if (mView)
		{
            [mView destroy];
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
