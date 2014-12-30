/// @file
/// @version 3.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <hltypes/hmap.h>
#include <hltypes/hstring.h>
#include <stdlib.h>
#include "Platform.h"
#include "Window.h"
#include "main_base.h"

namespace april
{
	SystemInfo info;

	SystemInfo::SystemInfo()
	{
		this->name = "";
#ifdef _ARM
		this->architecture = "ARM";
#else
		this->architecture = "x86";
#endif
		// __LP64__ - apple specific, applies to both iOS and Mac
		// _X64     - manual override for other platforms
#if defined(__LP64__) || defined(_X64)
		this->architectureBits = 64;
#else
		this->architectureBits = 32;
#endif
		this->osVersion = 1.0f;
		this->cpuCores = 1;
		this->ram = 256;
		this->displayDpi = 0.0f;
		this->locale = "";
	}
	
	SystemInfo::~SystemInfo()
	{
	}

	void messageBox(chstr title, chstr text, MessageBoxButton buttonMask, MessageBoxStyle style,
		hmap<MessageBoxButton, hstr> customButtonTitles, void(*callback)(MessageBoxButton))
	{
		MessageBoxStyle passedStyle = style;
		if (style & MESSAGE_STYLE_TERMINATE_ON_DISPLAY)
		{
			if (window != NULL)
			{
#if !defined(_IOS) && !defined(_COCOA_WINDOW)
				window->terminateMainLoop();
				window->destroy();
#endif
#ifdef _COCOA_WINDOW
				window->destroy();
#endif
			}
			passedStyle = (MessageBoxStyle)(passedStyle | MESSAGE_STYLE_MODAL);
		}
		messageBox_platform(title, text, buttonMask, passedStyle, customButtonTitles, callback);
		if (style & MESSAGE_STYLE_TERMINATE_ON_DISPLAY)
		{
			exit(0);
		}
	}

	void _makeButtonLabels(hstr* ok, hstr* yes, hstr* no, hstr* cancel,
		MessageBoxButton buttonMask, hmap<MessageBoxButton, hstr> customButtonTitles)
	{
		if ((buttonMask & MESSAGE_BUTTON_OK) && (buttonMask & MESSAGE_BUTTON_CANCEL))
		{
			*ok = customButtonTitles.tryGet(MESSAGE_BUTTON_OK, "OK");
			*cancel = customButtonTitles.tryGet(MESSAGE_BUTTON_CANCEL, "Cancel");
		}
		else if ((buttonMask & MESSAGE_BUTTON_YES) && (buttonMask & MESSAGE_BUTTON_NO) && (buttonMask & MESSAGE_BUTTON_CANCEL))
		{
			*yes = customButtonTitles.tryGet(MESSAGE_BUTTON_YES, "Yes");
			*no = customButtonTitles.tryGet(MESSAGE_BUTTON_NO, "No");
			*cancel = customButtonTitles.tryGet(MESSAGE_BUTTON_CANCEL, "Cancel");
		}
		else if (buttonMask & MESSAGE_BUTTON_OK)
		{
			*ok = customButtonTitles.tryGet(MESSAGE_BUTTON_OK, "OK");
		}
		else if ((buttonMask & MESSAGE_BUTTON_YES) && (buttonMask & MESSAGE_BUTTON_NO))
		{
			*yes = customButtonTitles.tryGet(MESSAGE_BUTTON_YES, "Yes");
			*no = customButtonTitles.tryGet(MESSAGE_BUTTON_NO, "No");
		}
	}

}
