/// @file
/// @author  Kresimir Spes
/// @version 3.0
///
/// @section LICENSE
///
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#import <Cocoa/Cocoa.h>
#import "Mac_LoadingOverlay.h"
#include "Window.h"

NSWindow* mOverlayWindow = nil;
NSImageView* mImageView = nil;

void createLoadingOverlay(NSWindow* parent)
{
	NSString* path = [[NSBundle mainBundle] pathForResource:@"Default" ofType:@"png"];
	if ([[NSFileManager defaultManager] fileExistsAtPath:path])
	{
		NSRect windowFrame = parent.frame;
		NSRect frame = [parent.contentView bounds];
		
		frame.origin = windowFrame.origin;
		mOverlayWindow = [[NSWindow alloc] initWithContentRect:frame styleMask:NSBorderlessWindowMask backing: NSBackingStoreBuffered defer:false];

		NSImage* image = [[NSImage alloc] initWithContentsOfFile:path];
		NSSize imgSize = image.size;
		mImageView = [[NSImageView alloc] init];
		[mImageView setImage:image];

		if ((float) imgSize.width / imgSize.height > (float) frame.size.width / frame.size.height)
		{
			float w = frame.size.width;
			frame.size.width = frame.size.height * imgSize.width / imgSize.height;
			frame.origin.y = 0;
			frame.origin.x = -(frame.size.width - w) / 2;
		}
		else
		{
			frame.origin.y = 0;
		}
		
		[mImageView setImageScaling:NSImageScaleAxesIndependently];
		[mImageView setFrame:frame];
		
		[mOverlayWindow.contentView addSubview: mImageView];
		[parent addChildWindow:mOverlayWindow ordered:NSWindowAbove];
		[mOverlayWindow makeKeyWindow];
	}
}

void reattachLoadingOverlay()
{
	if (mOverlayWindow != nil)
	{
		NSWindow* wnd = mOverlayWindow.parentWindow;
		[wnd removeChildWindow:mOverlayWindow];
		[wnd addChildWindow:mOverlayWindow ordered:NSWindowAbove];
	}
}

void updateLoadingOverlay(float k)
{
	static float alpha = 1.5f;
	if (mOverlayWindow)
	{
		alpha -= k;
		if (april::window->getParam("retain_loading_overlay") == "1") alpha = 1.5f;

		if (alpha < 0)
		{
			[mImageView removeFromSuperview];
			[mImageView release];
			mImageView = nil;
			[mOverlayWindow.parentWindow removeChildWindow:mOverlayWindow];
			[mOverlayWindow release];
			mOverlayWindow = nil;
		}
		else
			mOverlayWindow.alphaValue = alpha;
	}
}
