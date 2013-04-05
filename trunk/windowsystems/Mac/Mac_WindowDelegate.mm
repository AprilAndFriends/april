/// @file
/// @author  Kresimir Spes
/// @version 3.0
///
/// @section LICENSE
///
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#import <Foundation/Foundation.h>
#import "Mac_WindowDelegate.h"
#include "Mac_LoadingOverlay.h"
#include "Window.h"

extern bool gReattachLoadingOverlay;

@implementation AprilMacWindowDelegate

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

- (BOOL)windowShouldClose:(NSWindow *)sender
{
	return YES;
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
