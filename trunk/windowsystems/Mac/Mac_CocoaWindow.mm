/// @file
/// @author  Kresimir Spes
/// @version 3.0
///
/// @section LICENSE
///
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#import <Foundation/Foundation.h>
#import "Mac_CocoaWindow.h"
#include "Mac_LoadingOverlay.h"
#include "Mac_Window.h"
#include "Keys.h"

extern bool gReattachLoadingOverlay;

@implementation AprilCocoaWindow

- (void)timerEvent:(NSTimer *)t
{
	if (april::window) april::window->updateOneFrame();
    //[self setNeedsDisplay:YES];
}

- (void)windowDidBecomeMain:(NSNotification *)notification
{
    mTimer = [NSTimer timerWithTimeInterval:1/60.0f
									 target:self
								   selector:@selector(timerEvent:)
								   userInfo:nil
									repeats:YES];
    
    [[NSRunLoop mainRunLoop] addTimer:mTimer forMode:NSDefaultRunLoopMode];
}

- (BOOL)windowShouldClose:(NSWindow*) sender
{
	return YES;
}

- (gvec2)transformCocoaPoint:(NSPoint) point
{
	// TODO: optimize
	gvec2 pt(point.x, april::window->getHeight() - point.y);
	return pt;
}

- (void)mouseDown:(NSEvent*) event
{	
	gvec2 pos = [self transformCocoaPoint:[event locationInWindow]];
	((april::Mac_Window*) april::window)->updateCursorPosition(pos);
	april::window->handleMouseEvent(april::Window::AMOUSEEVT_DOWN, pos, april::AK_LBUTTON);
}

- (void)mouseUp:(NSEvent*) event
{
	gvec2 pos = [self transformCocoaPoint:[event locationInWindow]];
	((april::Mac_Window*) april::window)->updateCursorPosition(pos);
	april::window->handleMouseEvent(april::Window::AMOUSEEVT_UP, pos, april::AK_LBUTTON);	
}

- (void)mouseMoved:(NSEvent*) event
{
	gvec2 pos = [self transformCocoaPoint:[event locationInWindow]];
	((april::Mac_Window*) april::window)->updateCursorPosition(pos);
	april::window->handleMouseEvent(april::Window::AMOUSEEVT_MOVE, pos, april::AK_NONE);
}

- (void)windowDidResignKey:(NSNotification*) notification
{
	if (gReattachLoadingOverlay)
	{
		gReattachLoadingOverlay = false;
		reattachLoadingOverlay();
	}
}

- (void) dealloc
{
    [mTimer release];
	[super dealloc];
}

@end
