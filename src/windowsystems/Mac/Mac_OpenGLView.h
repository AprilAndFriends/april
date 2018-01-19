/// @file
/// @version 5.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION

#ifndef APRIL_MAC_OPENGL_VIEW_H
#define APRIL_MAC_OPENGL_VIEW_H

#import <AppKit/NSOpenGLView.h>

@interface AprilMacOpenGLView : NSOpenGLView
{
@public
	bool mStartedDrawing;
	bool mUseBlankCursor;
    bool mDrawingFromMainThread;
	NSCursor* mBlankCursor;
	NSCursor* mCursor;
	NSRect mFrameRect;
    CVDisplayLinkRef mDisplayLink;
	NSTimer* mTimer;

}

- (void) initApril;
- (void) draw;
- (void) presentFrame;
- (void) setUseBlankCursor:(BOOL)value;
- (void) setCursor:(NSCursor*)cursor;
- (void) destroy;

@end

namespace april
{
	extern AprilMacOpenGLView* macGlView;
}
#endif
