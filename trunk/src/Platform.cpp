/// @file
/// @author  Ivan Vucica
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @version 2.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <hltypes/hmap.h>
#include <hltypes/hstring.h>

#include "Platform.h"
#include "Window.h"

namespace april
{
	gvec2 getDisplayResolution()
	{
		return getSystemInfo().displayResolution;
	}

	MessageBoxButton messageBox(chstr title, chstr text, MessageBoxButton buttonMask, MessageBoxStyle style,
		hmap<MessageBoxButton, hstr> customButtonTitles, void(*callback)(MessageBoxButton))
	{
		MessageBoxStyle passedStyle = style;
		if (style & AMSGSTYLE_TERMINATEAPPONDISPLAY) 
		{
			// TODO - move somewhere else, this file should be completely platform independent
#if !TARGET_OS_IPHONE
			window->terminateMainLoop();
			window->destroy();
#endif
			passedStyle = (MessageBoxStyle)(passedStyle & AMSGSTYLE_MODAL);
		}
		MessageBoxButton returnValue = messageBox_platform(title, text, buttonMask, passedStyle, customButtonTitles, callback);
		if (style & AMSGSTYLE_TERMINATEAPPONDISPLAY)
		{
			exit(1);
		}
		return returnValue;
	}

}
