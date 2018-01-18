/// @file
/// @version 5.0
///
/// @section LICENSE
///
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#import <OpenGL/gl.h>
#include <Cocoa/Cocoa.h>

#include <hltypes/hlog.h>
#include <hltypes/hmutex.h>
#include <hltypes/hthread.h>

#import "Mac_OpenGLView.h"
#import "Mac_CocoaWindow.h"
#include "Application.h"
#include "april.h"
#include "Mac_Window.h"

// #define _OVERDRAW_DEBUG -- uncomment this to test overdraw response times

#define MAC_WINDOW ((april::Mac_Window*)april::window)

static AprilMacOpenGLView* gView;
extern AprilCocoaWindow* gWindow;

static CVReturn AprilDisplayLinkCallback(CVDisplayLinkRef displayLink, const CVTimeStamp* now, const CVTimeStamp* outputTime, CVOptionFlags flagsIn, CVOptionFlags* flagsOut, void* displayLinkContext)
{
	[gView draw];
	return kCVReturnSuccess;
}

@implementation AprilMacOpenGLView


-(void)cursorCheck:(NSTimer*) t
{
	// kspes@20150819 - Starting MacOS 10.10.5 I've noticed cursor rects getting messed up in fullscreen mode (windowed works fine). So this code here is a hack/workarround that problem
	// Analyzing the code everything seems to be in order, regardless if I use the old cursor rect logic or newer NSTrackingArea, the effect is the same. if this is a bug in apple or our code
	// I can't be sure, but in the lack of a better solution now, this is going to be used until and if that better solution comes.
	// update@20160119 - Had problems with cursors not showing up so I update the if below to force set the cursors in this case, while still not 100% perfect, it works better and more reliable
	// now I even got cursors not showing up properly in windowed mode, bah..
	if (april::window->getParam("disableCursorCheck") == "1")
	{
		return;
	}
	NSCursor* current = [NSCursor currentCursor];
	if (mCursor != NULL)
	{
		if (current != mCursor)
		{
			[gWindow invalidateCursorRectsForView:self];
			[mCursor set];
		}
	}
	else if (mUseBlankCursor)
	{
		if (current != mBlankCursor)
		{
			[gWindow invalidateCursorRectsForView:self];
			[mBlankCursor set];
		}
	}
	else if (mCursor == NULL)
	{
		if (current != [NSCursor arrowCursor])
		{
			[gWindow invalidateCursorRectsForView:self];
			[[NSCursor arrowCursor] set];
		}
	}
}

-(id) initWithFrame:(NSRect)frameRect
{
	gView = self;
	mDisplayLink = nil;
	mFrameRect = frameRect;
	mUseBlankCursor = false;
	mStartedDrawing = false;
	mDrawingFromMainThread = false;
	NSImage* image = [[NSImage alloc] initWithSize: NSMakeSize(8, 8)];
	NSBitmapImageRep* bmp = [[NSBitmapImageRep alloc] initWithBitmapDataPlanes: NULL pixelsWide: 1 pixelsHigh: 1 bitsPerSample: 8 samplesPerPixel: 4 hasAlpha: YES isPlanar: NO colorSpaceName: NSDeviceRGBColorSpace bytesPerRow: 0 bitsPerPixel: 0];
	[image addRepresentation: bmp];
	[image lockFocus];
	[[[NSColor blackColor] colorWithAlphaComponent:0] set];
	NSRectFill(NSMakeRect(0, 0, 1, 1));
	[image unlockFocus];
	mBlankCursor = [[NSCursor alloc] initWithImage:image hotSpot:NSMakePoint(0, 0)];
	[bmp release];
	[image release];
	mCursor = NULL;
	mTimer = [NSTimer timerWithTimeInterval:1 / 1000.0f target:self selector:@selector(cursorCheck:) userInfo:nil repeats:YES];
	[[NSRunLoop currentRunLoop] addTimer:mTimer forMode:NSDefaultRunLoopMode];
	return self;
}

-(void)initOpenGL
{
	// set up pixel format
	int n = 0;
	NSOpenGLPixelFormatAttribute a[64] = {0};
	a[n++] = NSOpenGLPFANoRecovery;
	a[n++] = NSOpenGLPFADoubleBuffer;
	a[n++] = NSOpenGLPFADepthSize; a[n++] = (NSOpenGLPixelFormatAttribute) 16;
	if (isLionOrNewer())
	{
		a[n++] = kCGLPFAOpenGLProfile; a[n++] = kCGLOGLPVersion_Legacy;
	}
	a[n++] = 0;
	NSOpenGLPixelFormat *pf = [[NSOpenGLPixelFormat alloc] initWithAttributes:a];
	if (pf == NULL)
	{
		hlog::error(april::logTag, "Unable to create requested OpenGL pixel format");
	}
	if (self = [super initWithFrame:mFrameRect pixelFormat:[pf autorelease]])
	{
		NSOpenGLContext* context = [self openGLContext];
		[context makeCurrentContext];
		// Synchronize buffer swaps with vertical refresh rate
		GLint swapInt = 1;
		if (april::rendersys->getOptions().vSync == false)
		{
			swapInt = 0;
		}
		[context setValues:&swapInt forParameter:NSOpenGLCPSwapInterval];
		if (april::isUsingCVDisplayLink())
		{
			CGLContextObj cglContext = [[self openGLContext] CGLContextObj];
			CGLPixelFormatObj cglPixelFormat = [[self pixelFormat] CGLPixelFormatObj];
			CVDisplayLinkCreateWithActiveCGDisplays(&mDisplayLink);
			CVDisplayLinkSetOutputCallback(mDisplayLink, &AprilDisplayLinkCallback, self);
			CVDisplayLinkSetCurrentCGDisplayFromOpenGLContext(mDisplayLink, cglContext, cglPixelFormat);
			CVDisplayLinkStart(mDisplayLink);
		}
	}
}

-(void)draw
{
	bool displayLink = april::isUsingCVDisplayLink();
	if (displayLink)
	{
		if (MAC_WINDOW->shouldIgnoreUpdate())
		{
			return;
		}
#ifdef _OVERDRAW_DEBUG
		unsigned int t1, t2;
		t1 = htickCount();
#endif
		hmutex::ScopeLock lock(&MAC_WINDOW->renderThreadSyncMutex);
		if (april::application != NULL && april::application->getState() == april::Application::State::Running)
		{
			NSOpenGLContext* context = [gView openGLContext];
			[context makeCurrentContext];
			CGLLockContext([context CGLContextObj]);
			april::application->update();
#ifdef  _OVERDRAW_DEBUG
			t2 = htickCount();
			if (t2 - t1 > 16) // 16 is max render time for 60 FPS
			{
				printf("overdraw: %d ms\n", t2 - t1);
			}
#endif
			CGLFlushDrawable([context CGLContextObj]);
			CGLUnlockContext([context CGLContextObj]);
			if (april::application->getState() != april::Application::State::Running)
			{
				[gWindow terminateMainLoop];
			}
		}
	}
	else
	{
		if (MAC_WINDOW->shouldIgnoreUpdate())
		{
			mStartedDrawing = false;
			return;
		}
		NSOpenGLContext* context = [self openGLContext];
		[context makeCurrentContext];
		if (april::application != NULL && april::application->getState() == april::Application::State::Running)
		{
			april::application->update();
			[context flushBuffer];
			if (april::application->getState() != april::Application::State::Running)
			{
				[gWindow terminateMainLoop];
			}
		}
		mStartedDrawing = false;
	}
	if (MAC_WINDOW->messageBoxQueue.size() > 0)
	{
#define ns(s) [NSString stringWithUTF8String:s.cStr()]
		april::Mac_Window::MessageBoxData data = MAC_WINDOW->messageBoxQueue.removeFirst();
		[AprilCocoaWindow showAlertView:ns(data.title) button1:ns(data.buttons[0]) button2:ns(data.buttons[1]) button3:ns(data.buttons[2]) btn1_t:data.buttonTypes[0] btn2_t:data.buttonTypes[1] btn3_t:data.buttonTypes[2] text:ns(data.text) callback:data.callback];
	}
}

-(void)drawRect:(NSRect)dirtyRect
{
	bool displayLink = april::isUsingCVDisplayLink();
	if (displayLink)
	{
		if (april::window->getParam("displayLinkIgnoreSystemRedraw") != "1")
		{
			glClear(GL_COLOR_BUFFER_BIT);
			if (MAC_WINDOW->shouldIgnoreUpdate())
			{
				return;
			}
			hmutex::ScopeLock lock;
			lock.acquire(&MAC_WINDOW->renderThreadSyncMutex);
			mDrawingFromMainThread = true;
			lock.release();
			[self draw];
			mDrawingFromMainThread = false;
		}
	}
	else
	{
		[self draw];
	}
}

-(void)presentFrame
{
	if (april::isUsingCVDisplayLink())
	{
		CGLFlushDrawable([[self openGLContext] CGLContextObj]);
	}
	else
	{
		[[self openGLContext] makeCurrentContext];
		[[self openGLContext] flushBuffer];
	}
}

-(void)resetCursorRects
{
	hmutex::ScopeLock lock;
	if (april::isUsingCVDisplayLink() && !mDrawingFromMainThread)
	{
		lock.acquire(&MAC_WINDOW->renderThreadSyncMutex);
	}
	if (mUseBlankCursor)
	{
		[self addCursorRect:[self bounds] cursor:mBlankCursor];
	}
	else if (mCursor != NULL)
	{
		[self addCursorRect:[self bounds] cursor:mCursor];
	}
	else
	{
		[self addCursorRect:[self bounds] cursor:[NSCursor arrowCursor]];
	}
}

// Apple doesn't forward rightmouse events on MacOS 10.6 and earlier for some reason, so we override the behaviour here
-(void)rightMouseDown:(NSEvent*) event
{
	[[self nextResponder] rightMouseDown:event];
}

-(void)rightMouseUp:(NSEvent*) event
{
	[[self nextResponder] rightMouseUp:event];
}

-(void) setUseBlankCursor:(BOOL)value
{
	mUseBlankCursor = value;
}

-(void) setCursor:(NSCursor*)cursor
{
	mCursor = (cursor == NULL) ? NULL : cursor;
}

-(void) destroy
{
	if (mDisplayLink)
	{
		CVDisplayLinkRelease(mDisplayLink);
		mDisplayLink = NULL;
	}
}

-(void) dealloc
{
	[self destroy];
	[mBlankCursor release];
	[super dealloc];
}

@end
