/// @file
/// @author  Kresimir Spes
/// @version 3.0
///
/// @section LICENSE
///
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
#include <Cocoa/Cocoa.h>
#import "Mac_OpenGLView.h"
#include "Window.h"

@implementation AprilMacOpenGLView

- (id) initWithFrame:(NSRect)frameRect
{
	mUseBlankCursor = false;
	
	NSImage* image = [[NSImage alloc] initWithSize: NSMakeSize(8, 8)];
	
	NSBitmapImageRep* bmp = [[NSBitmapImageRep alloc] initWithBitmapDataPlanes: NULL pixelsWide: 1 pixelsHigh: 1 bitsPerSample: 8 samplesPerPixel: 4 hasAlpha: YES isPlanar: NO colorSpaceName: NSDeviceRGBColorSpace bytesPerRow: 0 bitsPerPixel: 0];
	[image addRepresentation: bmp];
	[image lockFocus];
	
	[[NSColor colorWithSRGBRed:0 green:0 blue:0 alpha:0] set];
	NSRectFill(NSMakeRect(0, 0, 1, 1));
	
	[image unlockFocus];

	mBlankCursor = [[NSCursor alloc] initWithImage:image hotSpot:NSMakePoint(0, 0)];
	[bmp release];
	[image release];

    NSOpenGLPixelFormatAttribute attrs[] =
    {
		NSOpenGLPFAAccelerated,
		NSOpenGLPFANoRecovery,
		NSOpenGLPFADoubleBuffer,
		NSOpenGLPFADepthSize, 24,
		0
    };
	
    NSOpenGLPixelFormat *pf = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
	
    if (!pf)
		NSLog(@"No OpenGL pixel format");
	
    if (self = [super initWithFrame:frameRect pixelFormat:[pf autorelease]])
	{
		[self initGL];
	}
	
	return self;
}

- (void) initGL
{
	[[self openGLContext] makeCurrentContext];
	
	// Synchronize buffer swaps with vertical refresh rate
	GLint swapInt = 1;
	[[self openGLContext] setValues:&swapInt forParameter:NSOpenGLCPSwapInterval];
}

- (void) presentFrame
{
	[[self openGLContext] makeCurrentContext];
	    
	[[self openGLContext] flushBuffer];
}

- (void)resetCursorRects
{
	if (mUseBlankCursor)
		[self addCursorRect:[self bounds] cursor:mBlankCursor];
}

- (void) setDefaultCursor
{
	mUseBlankCursor = false;
}

- (void) setBlankCursor
{
	mUseBlankCursor = true;
	[self resetCursorRects];
}

- (BOOL) isBlankCursorUsed
{
	return mUseBlankCursor;
}

- (void) dealloc
{
	[mBlankCursor release];
	[super dealloc];
}

@end
