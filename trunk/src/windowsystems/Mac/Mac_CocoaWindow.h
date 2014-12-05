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
#include "Platform.h"

typedef void (*MessageBoxCallback)(april::MessageBoxButton);
struct MessageBoxParams
{
    hstr title, button1, button2, button3, text;
    harray<april::MessageBoxButton> btnTypes;
    MessageBoxCallback callback;
};

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
- (void)_preLionToggleFullscreen:(NSValue*) param;
- (void)platformToggleFullScreen;
- (void)enterFullScreen;
- (void)exitFullScreen;
- (void)setOpenGLView:(AprilMacOpenGLView*) view;
- (void)onWindowSizeChange;
- (void)setWindowedStyleMask;
- (void)_terminateMainLoop:(void*) param;
- (void)terminateMainLoop;
- (void)destroy;
- (void)_setTitle:(NSString*) title;
- (void)_showAlertView:(NSValue*) _params;
- (void)showAlertView:(NSString*) title button1:(NSString*) btn1 button2:(NSString*) btn2 button3:(NSString*) btn3 btn1_t:(april::MessageBoxButton) btn1_t btn2_t:(april::MessageBoxButton) btn2_t btn3_t:(april::MessageBoxButton) btn3_t text:(NSString*) text callback:(MessageBoxCallback) callback;

@end

#endif
