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

#if TARGET_OS_IPHONE
#import <UIKit/UIKit.h>

#include <gtypes/Vector2.h>
#include <hltypes/hltypesUtil.h>

#include "iOSWindow.h"
#include "Platform.h"

namespace april
{
	gvec2 getDisplayResolution()
	{
		UIScreen* mainScreen = [UIScreen mainScreen];
		float scale = 1.0f;
#if __IPHONE_3_2 //__IPHONE_OS_VERSION_MIN_REQUIRED >= 30200
		if ([mainScreen respondsToSelector:@selector(scale)])
		{
			scale = [mainScreen scale];
		}
#endif
		int w = mainScreen.bounds.size.width * scale;
		int h = mainScreen.bounds.size.height * scale;
		// forcing a w:h ratio where w > h
		return gvec2((float)hmax(w, h), (float)hmin(w, h));
	}

}
#endif
