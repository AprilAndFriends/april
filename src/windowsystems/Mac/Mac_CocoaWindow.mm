/// @file
/// @version 3.5
///
/// @section LICENSE
///
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#import <Cocoa/Cocoa.h>
#import "Mac_CocoaWindow.h"
#include <hltypes/hlog.h>
#include <hltypes/hthread.h>
#include "SystemDelegate.h"
#include "Mac_LoadingOverlay.h"
#include "Mac_Window.h"
#include "Mac_Keys.h"
#include "april.h"

extern bool gReattachLoadingOverlay;
static bool gFullscreenToggleRequest = false;

@implementation AprilCocoaWindow

- (void)timerEvent:(NSTimer*) t
{
	// Avoid CPU overload while the app is waiting for screen to refresh
	if (mView->mStartedDrawing)
	{
		hthread::sleep(1);
		return;
	}

	mView->mStartedDrawing = true;
	[mView setNeedsDisplay:YES];
	
	if (gFullscreenToggleRequest)
	{
		[self toggleFullScreen:nil];
		gFullscreenToggleRequest = false;
	}
}

- (void)queryUpdated:(NSNotification*) note
{
	// needed for screenshot notifications because mac messes up the custom cursor when making a screenshot in fullscreen...sigh..
	[self invalidateCursorRectsForView:mView];
}

- (void)configure
{
	april::initMacKeyMap();
	mCustomFullscreenExitAnimation = false;

	[self setBackgroundColor:[NSColor blackColor]];
	[self setOpaque:YES];
	[self setDelegate:self];
	[self setAcceptsMouseMovedEvents:YES];
	
	// setup screenshot listening to counter apple's bug..
	mMetadataQuery = [[NSMetadataQuery alloc] init];
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(queryUpdated:) name:NSMetadataQueryDidStartGatheringNotification object:mMetadataQuery];
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(queryUpdated:) name:NSMetadataQueryDidUpdateNotification object:mMetadataQuery];
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(queryUpdated:) name:NSMetadataQueryDidFinishGatheringNotification object:mMetadataQuery];
	
	[mMetadataQuery setDelegate:self];
	[mMetadataQuery setPredicate:[NSPredicate predicateWithFormat:@"kMDItemIsScreenCapture = 1"]];
	[mMetadataQuery startQuery];
}

- (void)startRenderLoop
{
	mTimer = [NSTimer timerWithTimeInterval:1 / 1000.0f target:self selector:@selector(timerEvent:) userInfo:nil repeats:YES];
	[[NSRunLoop currentRunLoop] addTimer:mTimer forMode:NSDefaultRunLoopMode];
	[[NSRunLoop currentRunLoop] addTimer:mTimer forMode:NSEventTrackingRunLoopMode];
}

- (BOOL)windowShouldClose:(NSWindow*) sender
{
    bool displayLink = april::isUsingCVDisplayLink();
    hmutex::ScopeLock lock;
    if (displayLink)
    {
        lock.acquire(&aprilWindow->renderThreadSyncMutex);
    }
	if (aprilWindow->handleQuitRequest(true))
	{
        if (displayLink)
        {
            lock.release();
        }
		[NSApp terminate:nil];
		return YES;
	}
	else
	{
		NSLog(@"Aborting window close request per app's request.");
	}
	return NO;
}

- (void)onWindowSizeChange
{
	NSSize size = [mView bounds].size;
	if (size.width == 0 || size.height == 0)
	{
		NSLog(@"onWindowSizeChange reported 0x0 size, ignoring");
		return;
	}
	if ([self inLiveResize])
	{
		mWindowedRect = [self frame];
	}
    int w = size.width * aprilWindow->scalingFactor, h = size.height * aprilWindow->scalingFactor;
    bool fullscreen = [self isFullScreen];
    if (april::isUsingCVDisplayLink())
    {
        aprilWindow->queueWindowSizeChanged(w, h, fullscreen);
    }
    else
    {
        aprilWindow->dispatchWindowSizeChanged(w, h, fullscreen);
    }
}

- (void)windowDidResize:(NSNotification*) notification
{
	[self onWindowSizeChange];
	updateLoadingOverlay(0);
}

- (void)windowDidMiniaturize:(NSNotification *)notification
{
#ifdef _DEBUG
	hlog::write(logTag, "User minimized window.");
#endif
	aprilWindow->onFocusChanged(0);
}

- (void)windowDidDeminiaturize:(NSNotification *)notification
{
#ifdef _DEBUG
	hlog::write(logTag, "User unminimized window.");
#endif
	aprilWindow->onFocusChanged(1);
}

- (BOOL)isFullScreen
{
	int style = [self styleMask];
	return (style == NSBorderlessWindowMask || style == NSFullScreenWindowMask); // this covers both 10.7 and older macs
}

- (void)enterFullScreen
{
	aprilWindow->setFullscreenFlag(true);
	NSRect prevFrame = [self frame];
	[self setStyleMask:NSBorderlessWindowMask];
	[self setFrame: [[NSScreen mainScreen] frame] display:YES];
	
	if (isPreLion()) // 10.7+ is set in willUseFullScreenPresentationOptions
	{
		[[NSApplication sharedApplication] setPresentationOptions: NSApplicationPresentationAutoHideMenuBar | NSApplicationPresentationHideDock];
	}
	if (!NSEqualRects(prevFrame, [self frame]))
	{
		[self onWindowSizeChange];
	}
}

- (NSApplicationPresentationOptions)window:(NSWindow*) window willUseFullScreenPresentationOptions:(NSApplicationPresentationOptions) proposedOptions
{
	return (proposedOptions| NSApplicationPresentationAutoHideToolbar | NSApplicationPresentationAutoHideMenuBar);
}

- (void)windowWillEnterFullScreen:(NSNotification*) notification
{
	[self enterFullScreen]; // this gets called on 10.7+, while < 10.7 call enterFullScreen directly
}

- (void)setWindowedStyleMask
{
	NSUInteger mask;
	mask = NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask;
	if (aprilWindow->getOptions().resizable) mask |= NSResizableWindowMask;
	[self setStyleMask:mask];
	if (isLionOrNewer()) // workarround for bug in macos
	{
		[[self standardWindowButton:NSWindowMiniaturizeButton] setEnabled:YES];
	}
}

- (void)exitFullScreen
{
	NSRect prevFrame = [self frame];
	[[NSApplication sharedApplication] setPresentationOptions: NSApplicationPresentationDefault];
	
	[self setWindowedStyleMask];
	[self setFrame:mWindowedRect display:YES];
	
	if (!NSEqualRects(prevFrame, [self frame]))
	{
		[self onWindowSizeChange];
	}
	aprilWindow->setFullscreenFlag(false);
}

- (void)window:(NSWindow*) window startCustomAnimationToExitFullScreenWithDuration:(NSTimeInterval) duration
{
	[NSAnimationContext beginGrouping];
	[[NSAnimationContext currentContext] setDuration:duration];
	[[self animator] setFrame:mWindowedRect display:NO];
	[NSAnimationContext endGrouping];
}

- (NSArray*) customWindowsToExitFullScreenForWindow:(NSWindow*) window
{
	// counter bad fullscreen exit animation when window started in fullscreen. all subsequent animations look good
	aprilWindow->setFullscreenFlag(false);
	if (!mCustomFullscreenExitAnimation) return nil;
	mCustomFullscreenExitAnimation = false;
	return [NSArray arrayWithObjects:self, nil];
}

- (void)windowWillExitFullScreen:(NSNotification*) notification
{
	[self setWindowedStyleMask];
}

- (void)windowDidExitFullScreen:(NSNotification*) notification
{
	[[NSApplication sharedApplication] setPresentationOptions: NSApplicationPresentationDefault];

	[self setWindowedStyleMask];
	[self setFrame:mWindowedRect display:YES];
	// for some reason, MacOSX 10.9 forgets the title after returning from fullscreen..
	[self setTitle:[NSString stringWithUTF8String:aprilWindow->getTitle().cStr()]];


}

- (gvec2)transformCocoaPoint:(NSPoint) point
{
	// TODO: optimize
	gvec2 pt(point.x, [self.contentView bounds].size.height - point.y);
	return pt;
}

- (april::Key)getMouseButtonCode:(NSEvent*) event
{
	april::Key button;
	int n = event.buttonNumber;
	if      (n == 0) button = april::AK_LBUTTON;
	else if (n == 1) button = april::AK_RBUTTON;
	else             button = april::AK_MBUTTON;
	return button;
}

- (void)mouseDown:(NSEvent*) event
{
	gvec2 pos = [self transformCocoaPoint:[event locationInWindow]];
	((april::Mac_Window*) april::window)->updateCursorPosition(pos);
    
    if (april::isUsingCVDisplayLink())
    {
        hmutex::ScopeLock lock(&aprilWindow->renderThreadSyncMutex);
        aprilWindow->queueMouseEvent(april::Window::MOUSE_DOWN, pos * aprilWindow->scalingFactor, [self getMouseButtonCode:event]);
    }
    else
    {
        aprilWindow->handleMouseEvent(april::Window::MOUSE_DOWN, pos * aprilWindow->scalingFactor, [self getMouseButtonCode:event]);
    }
}

- (void)rightMouseDown:(NSEvent*) event
{
	[self mouseDown:event];
}

- (void)otherMouseDown:(NSEvent*) event
{
	[self mouseDown:event];	
}

- (void)mouseUp:(NSEvent*) event
{
	gvec2 pos = [self transformCocoaPoint:[event locationInWindow]];
	((april::Mac_Window*) april::window)->updateCursorPosition(pos);
    if (april::isUsingCVDisplayLink())
    {
        hmutex::ScopeLock lock(&aprilWindow->renderThreadSyncMutex);
        aprilWindow->queueMouseEvent(april::Window::MOUSE_UP, pos * aprilWindow->scalingFactor, [self getMouseButtonCode:event]);
    }
    else
    {
        aprilWindow->handleMouseEvent(april::Window::MOUSE_UP, pos * aprilWindow->scalingFactor, [self getMouseButtonCode:event]);
    
    }
}

- (void)rightMouseUp:(NSEvent*) event
{
	[self mouseUp:event];
}

- (void)otherMouseUp:(NSEvent*) event
{
	[self mouseUp:event];
}

- (void)mouseMoved:(NSEvent*) event
{
	gvec2 pos = [self transformCocoaPoint:[event locationInWindow]];
	((april::Mac_Window*) april::window)->updateCursorPosition(pos);
    if (april::isUsingCVDisplayLink())
    {
        hmutex::ScopeLock lock(&aprilWindow->renderThreadSyncMutex);
        aprilWindow->queueMouseEvent(april::Window::MOUSE_MOVE, pos * aprilWindow->scalingFactor, april::AK_NONE);
    }
    else
    {
        aprilWindow->handleMouseEvent(april::Window::MOUSE_MOVE, pos * aprilWindow->scalingFactor, april::AK_NONE);
    }
	
	// Hack for Lion fullscreen bug, when the user moves the cursor quickly to and from the dock area,
	// the cursor gets reset to the default arrow, this hack counters that.
	if (isLionOrNewer() && [self isFullScreen])
	{
		static bool prevOnEdges = false;
		
		NSSize size = self.frame.size;
		// cover all 3 posibilities for dock position
		bool onEdges = pos.x < size.width * 0.01f || pos.x > size.width * 0.99f || pos.y > size.height * 0.99f;
		
		if (!onEdges && prevOnEdges)
		{
			[self invalidateCursorRectsForView:mView];
		}
		prevOnEdges = onEdges;
	}
}

- (void)mouseDragged:(NSEvent*) event
{
	[self mouseMoved:event];
}

- (void)rightMouseDragged:(NSEvent*) event
{
	[self mouseMoved:event];
}

- (void)otherMouseDragged:(NSEvent*) event
{
	[self mouseMoved:event];
}

- (void)onKeyDown:(unsigned int) keyCode unicode:(NSString*) unicode
{
	unsigned int unichr = 0;
	if ([unicode length] > 0)
	{
		unichr = [unicode characterAtIndex:0];
	}
    if (april::isUsingCVDisplayLink())
    {
        hmutex::ScopeLock lock(&aprilWindow->renderThreadSyncMutex);
        aprilWindow->queueKeyEvent(april::Window::KEY_DOWN, (april::Key) keyCode, unichr);
    }
    else
    {
        aprilWindow->handleKeyEvent(april::Window::KEY_DOWN, (april::Key) keyCode, unichr);
    }
}

- (void)onKeyUp:(unsigned int) keyCode
{
    if (april::isUsingCVDisplayLink())
    {
        hmutex::ScopeLock lock(&aprilWindow->renderThreadSyncMutex);
        aprilWindow->queueKeyEvent(april::Window::KEY_UP, (april::Key) keyCode, 0);
    }
    else
    {
        aprilWindow->handleKeyEvent(april::Window::KEY_UP, (april::Key) keyCode, 0);
    }
}

- (unsigned int) processKeyCode:(NSEvent*) event
{
	NSString* chr = [event characters];
	if ([chr length] == 1)
	{
		unichar c = [chr characterAtIndex:0];
		if (c >= 'a' && c <= 'z') return toupper(c);
	}
	return april::getAprilMacKeyCode([event keyCode]);
}

- (void)_preLionToggleFullscreen:(NSValue*) param
{
	if ([self isFullScreen])
	{
		[self exitFullScreen];
		aprilWindow->setFullscreenFlag(false);
	}
	else
	{
		[self enterFullScreen];
		aprilWindow->setFullscreenFlag(true);
	}
}

- (void)platformToggleFullScreen
{
	if (isLionOrNewer())
	{
		aprilWindow->setFullscreenFlag([self isFullScreen]);
		// doing it this way because direct toggling on lion+ throws some weird objc exception depending on where the call was being made from...
		if (april::isUsingCVDisplayLink())
		{
			[self performSelectorOnMainThread:@selector(toggleFullScreen:) withObject:nil waitUntilDone:NO];
		}
		else
		{
			gFullscreenToggleRequest = true;
		}
	}
	else
	{
		if (april::isUsingCVDisplayLink())
		{
			[self performSelectorOnMainThread:@selector(_preLionToggleFullscreen:) withObject:nil waitUntilDone:NO];
		}
		else
		{
			[self _preLionToggleFullscreen:nil];
		}
	}
	// setting title again because Cocoa forgets it for some reason when swiching from fullscreen
	[self _setTitle:[NSString stringWithUTF8String:aprilWindow->getTitle().cStr()]];
}

- (void)keyDown:(NSEvent*) event
{
	if ((event.modifierFlags & (NSCommandKeyMask | NSControlKeyMask)) == (NSCommandKeyMask | NSControlKeyMask))
	{
		NSString* s = [event charactersIgnoringModifiers];
		if ([s isEqualTo:@"f"]) // fullscreen toggle
		{
			[self platformToggleFullScreen];
			return;
		}
	}
	[self onKeyDown:[self processKeyCode:event] unicode:[event characters]];
}

- (void)keyUp:(NSEvent*) event
{
	if (event.modifierFlags & NSCommandKeyMask)
	{
		NSString* s = [event characters];
		if ([s isEqualTo:@"f"]) return;
	}

	[self onKeyUp:[self processKeyCode:event]];
}

- (void)scrollWheel:(NSEvent*) event
{
	gvec2 vec(-[event deltaX], -[event deltaY]);
    if (april::isUsingCVDisplayLink())
    {
        hmutex::ScopeLock lock(&aprilWindow->renderThreadSyncMutex);
        aprilWindow->queueMouseEvent(april::Window::MOUSE_SCROLL, vec, april::AK_NONE);
    }
    else
    {
        aprilWindow->handleMouseEvent(april::Window::MOUSE_SCROLL, vec, april::AK_NONE);
    }
}

- (void)flagsChanged:(NSEvent*) event // special NSWindow function for modifier keys
{
	static unsigned int prevFlags = 0;
	unsigned int flags = [event modifierFlags];
	unsigned int keyCode = april::getAprilMacKeyCode([event keyCode]);
	
#define processFlag(mask) if      ((flags & mask)     == mask && (prevFlags & mask) == 0) [self onKeyDown:keyCode unicode:@""];\
					      else if ((prevFlags & mask) == mask &&     (flags & mask) == 0) [self onKeyUp:keyCode];
	processFlag(NSControlKeyMask);
	processFlag(NSAlternateKeyMask);
	processFlag(NSShiftKeyMask);
	processFlag(NSCommandKeyMask);
	
	prevFlags = flags;
}

- (void)windowDidResignKey:(NSNotification*) notification
{
	if (gReattachLoadingOverlay)
	{
		gReattachLoadingOverlay = false;
		reattachLoadingOverlay();
	}
}

- (BOOL)canBecomeKeyWindow
{
	return YES;
}

- (BOOL)acceptsFirstResponder
{
	return YES;
}

- (void)setOpenGLView:(AprilMacOpenGLView*) view
{
	mView = view;
	self.contentView = view;
}

- (void)_setTitle:(NSString*) title
{
    if (april::isUsingCVDisplayLink())
    {
        [self performSelectorOnMainThread:@selector(setTitle:) withObject:title waitUntilDone:NO];
    }
    else
    {
        [self setTitle:title];
    }
}

- (void)_showAlertView:(NSValue*) _params
{
    MessageBoxParams* p_params = (MessageBoxParams*)[_params pointerValue];
    MessageBoxParams params = *p_params;
    delete p_params;
    NSString* title = [NSString stringWithUTF8String:params.title.cStr()];
    NSString* text = [NSString stringWithUTF8String:params.text.cStr()];
    NSString* button1 = [NSString stringWithUTF8String:params.button1.cStr()];
    NSString* button2 = [NSString stringWithUTF8String:params.button2.cStr()];
    NSString* button3 = [NSString stringWithUTF8String:params.button3.cStr()];

    int clicked = NSRunAlertPanel(title, @"%@", button1, button2, button3, text);
    switch (clicked)
    {
        case NSAlertDefaultReturn:
            clicked = 0;
            break;
        case NSAlertAlternateReturn:
            clicked = 1;
            break;
        case NSAlertOtherReturn:
            clicked = 2;
            break;
    }
    
    if (params.callback != NULL)
    {
        (*params.callback)(params.btnTypes[clicked]);
    }
}

- (void)showAlertView:(NSString*) title button1:(NSString*) btn1 button2:(NSString*) btn2 button3:(NSString*) btn3 btn1_t:(april::MessageBoxButton) btn1_t btn2_t:(april::MessageBoxButton) btn2_t btn3_t:(april::MessageBoxButton) btn3_t text:(NSString*) text callback:(MessageBoxCallback) callback
{
    MessageBoxParams* p = new MessageBoxParams();
    p->title = [title UTF8String];
    p->text = [text UTF8String];
    p->button1 = [btn1 UTF8String];
    p->button2 = [btn2 UTF8String];
    p->button3 = [btn3 UTF8String];
    p->btnTypes += btn1_t;
    p->btnTypes += btn2_t;
    p->btnTypes += btn3_t;
    p->callback = callback;
    if (april::isUsingCVDisplayLink())
    {
        [self performSelectorOnMainThread:@selector(_showAlertView:) withObject:[NSValue valueWithPointer:p] waitUntilDone:NO];
    }
    else
    {
        [self _showAlertView:[NSValue valueWithPointer:&p]];
    }
}

- (void)_terminateMainLoop:(void*) param
{
    [[NSApplication sharedApplication] terminate:nil];
}

- (void)terminateMainLoop
{
    if (april::isUsingCVDisplayLink())
    {
        [self performSelectorOnMainThread:@selector(_terminateMainLoop:) withObject:nil waitUntilDone:NO];
    }
    else
    {
        [self _terminateMainLoop:nil];
    }
}

- (void)destroy
{
	if (mTimer != nil)
	{
		[mTimer invalidate];
		mTimer = nil;
	}
	if (mMetadataQuery != nil)
	{
		[mMetadataQuery release];
		mMetadataQuery = nil;
	}
}

- (void)dealloc
{
	[self destroy];
	[super dealloc];
}

@end
