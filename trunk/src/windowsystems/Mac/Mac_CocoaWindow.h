/// @file
/// @version 3.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION

#ifndef APRIL_MAC_COCOA_WINDOW_H
#define APRIL_MAC_COCOA_WINDOW_H

#import <AppKit/NSWindow.h>
#import "Mac_OpenGLView.h"
#include "Keys.h"

@interface AprilCocoaWindow : NSWindow<NSWindowDelegate, NSMetadataQueryDelegate>
{
@public
	NSRect mWindowedRect;
	bool mCustomFullscreenExitAnimation;
@private
	NSMetadataQuery* mMetadataQuery;
	NSTimer* mTimer;
	AprilMacOpenGLView* mView;
}
- (void)startRenderLoop;
- (void)configure;
- (BOOL)isFullScreen;
- (void)platformToggleFullScreen;
- (void)enterFullScreen;
- (void)exitFullScreen;
- (void)setOpenGLView:(AprilMacOpenGLView*) view;
- (void)onWindowSizeChange;
- (void)setWindowedStyleMask;
- (void)destroy;
@end

#endif
