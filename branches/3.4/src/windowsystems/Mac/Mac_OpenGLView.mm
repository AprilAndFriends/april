/// @file
/// @version 3.4
///
/// @section LICENSE
///
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <Cocoa/Cocoa.h>
#include <hltypes/hlog.h>
#include "april.h"
#include "Mac_Window.h"
#import <OpenGL/gl.h>
#import "Mac_OpenGLView.h"

@implementation AprilMacOpenGLView

- (id) initWithFrame:(NSRect)frameRect
{
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

    if (!pf) hlog::error(april::logTag, "Unable to create requested OpenGL pixel format");

    if (self = [super initWithFrame:mFrameRect pixelFormat:[pf autorelease]])
	{
        NSOpenGLContext* context = [self openGLContext];
        [context makeCurrentContext];
        // Synchronize buffer swaps with vertical refresh rate
        GLint swapInt = 1;
        [context setValues:&swapInt forParameter:NSOpenGLCPSwapInterval];
	}	
}

- (void)updateGLViewport
{
    float w = aprilWindow->getWidth(),
          h = aprilWindow->getHeight();
	glViewport(0, 0, w, h);
}

- (void)drawRect:(NSRect)dirtyRect
{
	if (aprilWindow->ignoreUpdate)
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
			[self presentFrame];
		}
	}
	mStartedDrawing = false;
}

- (void) presentFrame
{
	[[self openGLContext] makeCurrentContext];
	[[self openGLContext] flushBuffer];
}

- (void)resetCursorRects
{
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

- (void) dealloc
{
	[mBlankCursor release];
	[super dealloc];
}

@end
