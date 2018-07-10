/// @file
/// @version 5.0
///
/// @section LICENSE
///
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#import <Cocoa/Cocoa.h>

#include <hltypes/hthread.h>

#import "Mac_LoadingOverlay.h"
#include "Window.h"

NSWindow* gOverlayWindow = nil;
NSImageView* gImageView = nil;
static float fadeOutDelay = -1;

static void updateLoadingOverlaySize(NSWindow* parent, bool check)
{
	NSRect windowFrame = parent.frame;
	NSRect frame = [parent.contentView bounds];
	if (!check || !NSEqualSizes(frame.size, gOverlayWindow.frame.size))
	{
		frame.origin = windowFrame.origin;
		NSSize imgSize = [gImageView image].size;
		NSRect imgFrame = frame;
		if ((float) imgSize.width / imgSize.height > (float) frame.size.width / frame.size.height)
		{
			float w = frame.size.width;
			imgFrame.size.width = frame.size.height * imgSize.width / imgSize.height;
			imgFrame.origin.y = 0;
			imgFrame.origin.x = -(imgFrame.size.width - w) / 2;
		}
		else
		{
			imgFrame.origin.x = imgFrame.origin.y = 0;
		}
		[gImageView setFrame:imgFrame];
		[gOverlayWindow setFrame:frame display:YES];
	}
}

void createLoadingOverlay(NSWindow* parent)
{
	NSScreen* mainScreen = parent.screen;
	if (mainScreen == nil)
	{
		mainScreen = [NSScreen mainScreen];
	}
	float scalingFactor = mainScreen.backingScaleFactor;
	NSString* imgName = scalingFactor > 1 ? @"Default@2x" : @"Default";
	NSString* path = [[NSBundle mainBundle] pathForResource:imgName ofType:@"png"];
	bool found = [[NSFileManager defaultManager] fileExistsAtPath:path];
	if (found)
	{
		NSRect windowFrame = parent.frame;
		NSRect frame = [parent.contentView bounds];
		frame.origin = windowFrame.origin;
		gOverlayWindow = [[NSWindow alloc] initWithContentRect:frame styleMask:NSBorderlessWindowMask backing: NSBackingStoreBuffered defer:false];
		[gOverlayWindow setBackgroundColor:[NSColor blackColor]];
		[gOverlayWindow setOpaque:YES];
		NSImage* image = [[NSImage alloc] initWithContentsOfFile:path];
		gImageView = [[NSImageView alloc] init];
		[gImageView setImage:image];
		updateLoadingOverlaySize(parent, 0);
		[gImageView setImageScaling:NSImageScaleAxesIndependently];
		[gOverlayWindow.contentView addSubview: gImageView];
		[parent addChildWindow:gOverlayWindow ordered:NSWindowAbove];
		[gOverlayWindow makeKeyWindow];
		[image release];
	}
}

void reattachLoadingOverlay()
{
	if (gOverlayWindow != nil)
	{
		NSWindow* wnd = gOverlayWindow.parentWindow;
		[wnd removeChildWindow:gOverlayWindow];
		[wnd addChildWindow:gOverlayWindow ordered:NSWindowAbove];
	}
}

void updateLoadingOverlay(float timeDelta)
{
	static float alpha = 505;
	if (fadeOutDelay > 0)
	{
		fadeOutDelay -= timeDelta;
		if (fadeOutDelay < 0)
		{
			fadeOutDelay = 0;
		}
		return;
	}
	else if (fadeOutDelay == -1)
	{
		hstr delay = april::window->getParam("delay_splash");
		if (delay != "")
		{
			fadeOutDelay = delay;
			return;
		}
	}
	if (alpha == 505)
	{
		alpha = april::window->getParam("splashscreen_fadeout") == "0" ? -1 : 1.5f;
	}
	if (gOverlayWindow)
	{
		if (april::window->getParam("fasthide_loading_overlay") == "1")
		{
			alpha -= timeDelta * 3; // If you don't want to wait for the loading overlay every time, set this param. eg on debug mode
		}
		else
		{
			alpha -= timeDelta;
		}
		if (april::window->getParam("retain_loading_overlay") == "1")
		{
			alpha = 1.5f;
		}
		if (alpha < 0.0f)
		{
			[gImageView removeFromSuperview];
			[gImageView release];
			gImageView = nil;
			[gOverlayWindow.parentWindow removeChildWindow:gOverlayWindow];
			[gOverlayWindow release];
			gOverlayWindow = nil;
		}
		else
		{
			gOverlayWindow.alphaValue = alpha;
			updateLoadingOverlaySize([gOverlayWindow parentWindow], 1);
		}
	}
	
}
