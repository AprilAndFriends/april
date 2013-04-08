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
	if (april::window) april::window->updateOneFrame();
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
	mTimer = [NSTimer timerWithTimeInterval:1/60.0f target:self selector:@selector(timerEvent:) userInfo:nil repeats:YES];
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
	april::window->handleKeyEvent(april::Window::AKEYEVT_DOWN, april::getAprilMacKeyCode(keyCode), unichr);
}

- (void)onKeyUp:(unsigned int) keyCode
{
	april::window->handleKeyEvent(april::Window::AKEYEVT_UP, april::getAprilMacKeyCode(keyCode), 0);
}

- (void)keyDown:(NSEvent*) event
{
	[self onKeyDown:[event keyCode] unicode:[event characters]];
	[super keyDown:event];
}

- (void)keyUp:(NSEvent*) event
{
	[self onKeyUp:[event keyCode]];
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
		onKeyDown:[event keyCode];
    }
    else
    {
		onKeyUp:[event keyCode];
    }
}

- (void)windowDidResignKey:(NSNotification*) notification
{
	if (gReattachLoadingOverlay)
	{
		gReattachLoadingOverlay = false;
		reattachLoadingOverlay();
	}
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
