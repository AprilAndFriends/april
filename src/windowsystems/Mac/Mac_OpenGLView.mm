/// @file
/// @version 3.5
///
/// @section LICENSE
///
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <Cocoa/Cocoa.h>
#include <hltypes/hlog.h>
#include <hltypes/hmutex.h>
#include <hltypes/hthread.h>
#include "april.h"
#include "Mac_Window.h"
#import <OpenGL/gl.h>
#import "Mac_OpenGLView.h"

// #define _OVERDRAW_DEBUG -- uncomment this to test overdraw response times

static AprilMacOpenGLView* view;
extern bool gAppStarted;

static CVReturn AprilDisplayLinkCallback(CVDisplayLinkRef displayLink, const CVTimeStamp* now, const CVTimeStamp* outputTime, CVOptionFlags flagsIn, CVOptionFlags* flagsOut, void* displayLinkContext)
{
    [view draw];
    return kCVReturnSuccess;
}

@implementation AprilMacOpenGLView

- (id) initWithFrame:(NSRect)frameRect
{
    view = self;
    mDisplayLink = nil;
	mFrameRect = frameRect;
	mUseBlankCursor = false;
	mStartedDrawing = false;
	
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
    
	return self;
}

- (void)initOpenGL
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

	if (!pf)
    {
        hlog::error(logTag, "Unable to create requested OpenGL pixel format");
    }

	if (self = [super initWithFrame:mFrameRect pixelFormat:[pf autorelease]])
	{
		NSOpenGLContext* context = [self openGLContext];
		[context makeCurrentContext];
		// Synchronize buffer swaps with vertical refresh rate
		GLint swapInt = 1;
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

- (void)draw
{
    bool displayLink = april::isUsingCVDisplayLink();
    hmutex::ScopeLock lock;
    if (displayLink)
    {
		if (aprilWindow->shouldIgnoreUpdate())
		{
			return;
		}
#ifdef _OVERDRAW_DEBUG
        unsigned int t1, t2;
        t1 = htickCount();
#endif
        lock.acquire(&aprilWindow->renderThreadSyncMutex);
        if (gAppStarted)
        {
            NSOpenGLContext* context = [view openGLContext];
            [context makeCurrentContext];
            CGLLockContext([context CGLContextObj]);

            aprilWindow->dispatchQueuedEvents();
            aprilWindow->updateOneFrame();
            
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
        if (aprilWindow->shouldIgnoreUpdate())
        {
            mStartedDrawing = false;
            return;
        }
        NSOpenGLContext* context = [self openGLContext];
        [context makeCurrentContext];
        if (april::window != NULL)
        {
            aprilWindow->updateOneFrame();
            if (april::rendersys != NULL)
            {
                [[self openGLContext] makeCurrentContext];
                [[self openGLContext] flushBuffer];
            }
        }
        mStartedDrawing = false;
    }
}

- (void)drawRect:(NSRect)dirtyRect
{
    [self draw];
}

- (void) presentFrame
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

- (void)resetCursorRects
{
    
    hmutex::ScopeLock lock;
    
    if (april::isUsingCVDisplayLink())
    {
        lock.acquire(&aprilWindow->renderThreadSyncMutex);
    }
	if (mUseBlankCursor)
	{
		[self addCursorRect:[self bounds] cursor:mBlankCursor];
	}
	else if (mCursor != NULL)
	{
		[self addCursorRect:[self bounds] cursor:mCursor];
	}
}

// Apple doesn't forward rightmouse events on MacOS 10.6 and earlier for some reason, so we override the behaviour here
- (void)rightMouseDown:(NSEvent*) event
{
	[[self nextResponder] rightMouseDown:event];
}

- (void)rightMouseUp:(NSEvent*) event
{
	[[self nextResponder] rightMouseUp:event];
}

- (void) setUseBlankCursor:(BOOL)value
{
	mUseBlankCursor = value;
}

- (void) setCursor:(NSCursor*)cursor
{
	mCursor = (cursor == NULL) ? NULL : cursor;
}

- (void) destroy
{
    if (mDisplayLink)
    {
        CVDisplayLinkRelease(mDisplayLink);
        mDisplayLink = NULL;
    }

}

- (void) dealloc
{
    [self destroy];
	[mBlankCursor release];
	[super dealloc];
}

@end
