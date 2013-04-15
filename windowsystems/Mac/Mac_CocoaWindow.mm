/// @file
/// @author  Kresimir Spes
/// @version 3.0
///
/// @section LICENSE
///
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#import <Cocoa/Cocoa.h>
#import "Mac_CocoaWindow.h"
#include "Mac_LoadingOverlay.h"
#include "Mac_Window.h"
#include "Mac_Keyboard.h"

extern bool gReattachLoadingOverlay;

@implementation AprilCocoaWindow

- (void)timerEvent:(NSTimer*) t
{
	[self.contentView setNeedsDisplay:YES];
}

- (void)configure
{
	april::initMacKeyMap();
	
	[self setBackgroundColor:[NSColor blackColor]];
	[self setOpaque:YES];
	[self setDelegate:self];
	[self setAcceptsMouseMovedEvents:YES];
}

- (void)startRenderLoop
{
	mTimer = [NSTimer timerWithTimeInterval:0.00000001f target:self selector:@selector(timerEvent:) userInfo:nil repeats:YES];
	[[NSRunLoop currentRunLoop] addTimer:mTimer forMode:NSDefaultRunLoopMode];
	[[NSRunLoop currentRunLoop] addTimer:mTimer forMode:NSEventTrackingRunLoopMode];
}

- (BOOL)windowShouldClose:(NSWindow*) sender
{
	if (april::window->handleQuitRequest(true))
	{
		[NSApp terminate:nil];
		return YES;
	}
	else
	{
		NSLog(@"Aborting window close request per app's request.");
	}
	return NO;
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

- (void)mouseDragged:(NSEvent*) event
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
	april::window->handleKeyEvent(april::Window::AKEYEVT_DOWN, (april::Key) keyCode, unichr);
}

- (void)onKeyUp:(unsigned int) keyCode
{
	april::window->handleKeyEvent(april::Window::AKEYEVT_UP, (april::Key) keyCode, 0);
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

- (void)keyDown:(NSEvent*) event
{
	[self onKeyDown:[self processKeyCode:event] unicode:[event characters]];
	[super keyDown:event];
}

- (void)keyUp:(NSEvent*) event
{
	[self onKeyUp:[self processKeyCode:event]];
	[super keyUp:event];
}

- (void)scrollWheel:(NSEvent*) event
{
	gvec2 vec([event deltaX], -[event deltaY]);
	april::window->handleMouseEvent(april::Window::AMOUSEEVT_SCROLL, vec, april::AK_NONE);
}

- (void)flagsChanged:(NSEvent*) event // special NSWindow function for modifier keys
{
    if ([event modifierFlags] > (1 << 15)) // DOWN
    {
		onKeyDown:april::getAprilMacKeyCode([event keyCode]);
    }
    else
    {
		onKeyUp:april::getAprilMacKeyCode([event keyCode]);
    }
}

- (void)windowDidResignKey:(NSNotification*) notification
{
	if (gReattachLoadingOverlay)
	{
		gReattachLoadingOverlay = false;
		reattachLoadingOverlay();
	}
	else april::window->handleFocusChangeEvent(0);
}

- (void)windowDidBecomeKey:(NSNotification*) notification
{
	if (!gReattachLoadingOverlay)
	{
		static bool first = 0;
		if (!first) first = 1; // ignore initialization time focus gain
		else april::window->handleFocusChangeEvent(1);
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

- (void) dealloc
{
    [mTimer release];
	[super dealloc];
}

@end
