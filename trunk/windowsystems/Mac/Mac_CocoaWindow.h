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

@interface AprilCocoaWindow : NSWindow<NSWindowDelegate>
{
@private
    NSTimer* mTimer;
}
@end


#endif
