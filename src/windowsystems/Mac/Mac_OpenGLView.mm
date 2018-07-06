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

namespace april
{
	AprilMacOpenGLView* macGlView = nil;
}

static CVReturn AprilDisplayLinkCallback(CVDisplayLinkRef displayLink, const CVTimeStamp* now, const CVTimeStamp* outputTime, CVOptionFlags flagsIn, CVOptionFlags* flagsOut, void* displayLinkContext)
{
	[april::macGlView draw];
	return kCVReturnSuccess;
}

@implementation AprilMacOpenGLView

-(void) cursorCheck:(NSTimer*) t
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
			[april::macCocoaWindow invalidateCursorRectsForView:self];
			[mCursor set];
		}
	}
	else if (mUseBlankCursor)
	{
		if (current != mBlankCursor)
		{
			[april::macCocoaWindow invalidateCursorRectsForView:self];
			[mBlankCursor set];
		}
	}
	else if (mCursor == NULL)
	{
		if (current != [NSCursor arrowCursor])
		{
			[april::macCocoaWindow invalidateCursorRectsForView:self];
			[[NSCursor arrowCursor] set];
		}
	}
}

-(id) initWithFrame:(NSRect)frameRect withPixels:(NSOpenGLPixelFormat*)pixelFormat
{
	self = [super initWithFrame:frameRect pixelFormat:pixelFormat];
	april::macGlView = self;
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

-(void) initApril
{
	int n = 0;
	NSOpenGLPixelFormatAttribute attributes[64] = {0};
	attributes[n++] = NSOpenGLPFANoRecovery;
	attributes[n++] = NSOpenGLPFADoubleBuffer;
	attributes[n++] = NSOpenGLPFADepthSize;
	attributes[n++] = (NSOpenGLPixelFormatAttribute)16;
	if (isLionOrNewer())
	{
		attributes[n++] = kCGLPFAOpenGLProfile;
		attributes[n++] = kCGLOGLPVersion_Legacy;
	}
	attributes[n++] = 0;
	NSOpenGLPixelFormat* pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attributes];
	if (pixelFormat == NULL)
	{
		hlog::error(april::logTag, "Unable to create requested OpenGL pixel format");
	}
	self = [super initWithFrame:mFrameRect pixelFormat:[pixelFormat autorelease]];
	if (self != NULL)
	{
		NSOpenGLContext* context = [self openGLContext];
		[context makeCurrentContext];
		GLint swapInterval = 1;
		if (april::rendersys->getOptions().vSync == false)
		{
			swapInterval = 0;
		}
		[context setValues:&swapInterval forParameter:NSOpenGLCPSwapInterval];
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

-(void) draw
{
	if (april::application != NULL && april::application->getState() == april::Application::State::Starting)
	{
		april::application->updateInitializing(true);
		return;
	}
	bool displayLink = april::isUsingCVDisplayLink();
	if (displayLink)
	{
		if (MAC_WINDOW->shouldIgnoreUpdate())
		{
			return;
		}
#ifdef _OVERDRAW_DEBUG
		unsigned int t1 = htickCount();
		unsigned int t2 = 0;
#endif
		hmutex::ScopeLock lock(&MAC_WINDOW->renderThreadSyncMutex);
		if (april::application != NULL)
		{
			NSOpenGLContext* context = [april::macGlView openGLContext];
			[context makeCurrentContext];
			CGLLockContext([context CGLContextObj]);
			if (april::application->getState() == april::Application::State::Running)
			{
				april::application->update();
			}
#ifdef  _OVERDRAW_DEBUG
			t2 = htickCount();
			if (t2 - t1 > 16) // 16 is max render time for 60 FPS
			{
				printf("overdraw: %d ms\n", t2 - t1);
			}
#endif
			CGLFlushDrawable([context CGLContextObj]);
			CGLUnlockContext([context CGLContextObj]);
		}
	}
	else
	{
		if (!MAC_WINDOW->shouldIgnoreUpdate())
		{
			NSOpenGLContext* context = [self openGLContext];
			[context makeCurrentContext];
			if (april::application != NULL && april::application->getState() == april::Application::State::Running)
			{
				april::application->update();
			}
		}
	}
	// check state again, don't cache!
	if (april::application != NULL && april::application->getState() != april::Application::State::Running)
	{
		[april::macCocoaWindow terminateMainLoop];
	}
	if (MAC_WINDOW->fpsCounter)
	{
		hstr newTitle = april::window->getTitle() + hsprintf(" [FPS: %d]", april::application->getFps());
		[april::macCocoaWindow _setTitle:[NSString stringWithUTF8String:newTitle.cStr()]];
	}
	else
	{
		[april::macCocoaWindow _setTitle:[NSString stringWithUTF8String:april::window->getTitle().cStr()]];
	}
	mStartedDrawing = false;
}

-(void)drawRect:(NSRect)dirtyRect
{
	[super drawRect:dirtyRect];
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
		NSOpenGLContext* context = [self openGLContext];
		[context flushBuffer];
		[context makeCurrentContext];
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
