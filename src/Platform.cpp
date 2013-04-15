/// @file
/// @author  Ivan Vucica
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @version 3.0
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

	void messageBox(chstr title, chstr text, MessageBoxButton buttonMask, MessageBoxStyle style,
		hmap<MessageBoxButton, hstr> customButtonTitles, void(*callback)(MessageBoxButton))
	{
		MessageBoxStyle passedStyle = style;
		if (style & AMSGSTYLE_TERMINATEAPPONDISPLAY) 
		{
			// TODO - move somewhere else, this file should be completely platform independent
#if !defined(_IOS) && !defined(_COCOA)
			window->terminateMainLoop();
			window->destroy();
#endif
#if defined(_COCOA)
			window->destroy();
#endif
			passedStyle = (MessageBoxStyle)(passedStyle | AMSGSTYLE_MODAL);
		}
		messageBox_platform(title, text, buttonMask, passedStyle, customButtonTitles, callback);
		if (style & AMSGSTYLE_TERMINATEAPPONDISPLAY)
		{
			exit(1);
		}
	}

	void _makeButtonLabels(hstr* ok, hstr* yes, hstr* no, hstr* cancel,
		MessageBoxButton buttonMask, hmap<MessageBoxButton, hstr> customButtonTitles)
	{
		if ((buttonMask & AMSGBTN_OK) && (buttonMask & AMSGBTN_CANCEL))
		{
			*ok = customButtonTitles.try_get_by_key(AMSGBTN_OK, "OK");
			*cancel = customButtonTitles.try_get_by_key(AMSGBTN_CANCEL, "Cancel");
		}
		else if ((buttonMask & AMSGBTN_YES) && (buttonMask & AMSGBTN_NO && buttonMask & AMSGBTN_CANCEL))
		{
			*yes = customButtonTitles.try_get_by_key(AMSGBTN_YES, "Yes");
			*no = customButtonTitles.try_get_by_key(AMSGBTN_NO, "No");
			*cancel = customButtonTitles.try_get_by_key(AMSGBTN_CANCEL, "Cancel");
		}
		else if (buttonMask & AMSGBTN_OK)
		{
			*ok = customButtonTitles.try_get_by_key(AMSGBTN_OK, "OK");
		}
		else if ((buttonMask & AMSGBTN_YES) && (buttonMask & AMSGBTN_NO))
		{
			*yes = customButtonTitles.try_get_by_key(AMSGBTN_YES, "Yes");
			*no = customButtonTitles.try_get_by_key(AMSGBTN_NO, "No");
		}
	}

}
