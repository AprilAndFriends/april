/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @author  Ivan Vucica
/// @version 1.52
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#if TARGET_OS_MAC
#import <AppKit/NSApplication.h>
#import <AppKit/NSCursor.h>
#import <AppKit/NSEvent.h>
#import <AppKit/NSPanel.h>
#import <AppKit/NSScreen.h>
#import <AppKit/NSWindow.h>
#import <Foundation/NSString.h>

#include <gtypes/Vector2.h>

#include "Platform.h"

namespace april
{
	gvec2 getDisplayResolution()
	{
		NSScreen* mainScreen = [NSScreen mainScreen];
		NSRect rect = [mainScreen frame];
		return gvec2(rect.size.width, rect.size.height);
	}

}
#endif
