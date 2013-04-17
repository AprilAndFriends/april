/// @file
/// @author  Kresimir Spes
/// @version 3.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/// 
/// @section DESCRIPTION

#ifndef APRIL_MAC_COCOA_WINDOW_H
#define APRIL_MAC_COCOA_WINDOW_H

#import <AppKit/NSWindow.h>
#import "Mac_OpenGLView.h"
#include "Keys.h"

@interface AprilCocoaWindow : NSWindow<NSWindowDelegate>
{
@public
	NSRect mWindowedRect;
@private
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
