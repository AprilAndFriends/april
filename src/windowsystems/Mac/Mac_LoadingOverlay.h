/// @file
/// @author  Kresimir Spes
/// @version 3.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 

#ifndef APRIL_MAC_LOADING_OVERLAY_H
#define APRIL_MAC_LOADING_OVERLAY_H

#import <AppKit/NSWindow.h>

void createLoadingOverlay(NSWindow* parent);
void updateLoadingOverlay(float timeDelta);
void reattachLoadingOverlay();

extern NSWindow* mOverlayWindow;

#endif
