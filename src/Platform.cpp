/// @file
/// @author  Ivan Vucica
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @version 3.2
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
	/*
	MessageBoxButton AMSGBTN_NULL = MESSAGE_BUTTON_OK; // DEPRECATED
	MessageBoxButton MESSAGE_BUTTON_OK = MESSAGE_BUTTON_OK; // DEPRECATED
	MessageBoxButton MESSAGE_BUTTON_CANCEL = MESSAGE_BUTTON_CANCEL; // DEPRECATED
	MessageBoxButton MESSAGE_BUTTON_YES = MESSAGE_BUTTON_YES; // DEPRECATED
	MessageBoxButton MESSAGE_BUTTON_NO = MESSAGE_BUTTON_NO; // DEPRECATED
	MessageBoxButton MESSAGE_BUTTON_OKCANCEL = MESSAGE_BUTTON_OK_CANCEL; // DEPRECATED
	MessageBoxButton MESSAGE_BUTTON_YESNO = MESSAGE_BUTTON_YES_NO; // DEPRECATED
	MessageBoxButton MESSAGE_BUTTON_YESNOCANCEL = MESSAGE_BUTTON_YES_NO_CANCEL; // DEPRECATED

	MessageBoxStyle AMSGSTYLE_PLAIN = MESSAGE_STYLE_NORMAL; // DEPRECATED
	MessageBoxStyle AMSGSTYLE_INFORMATION = MESSAGE_STYLE_INFO; // DEPRECATED
	MessageBoxStyle AMSGSTYLE_WARNING = MESSAGE_STYLE_WARNING; // DEPRECATED
	MessageBoxStyle AMSGSTYLE_CRITICAL = MESSAGE_STYLE_CRITICAL; // DEPRECATED
	MessageBoxStyle AMSGSTYLE_QUESTION = MESSAGE_STYLE_QUESTION; // DEPRECATED
	MessageBoxStyle AMSGSTYLE_MODAL = MESSAGE_STYLE_MODAL; // DEPRECATED
	MessageBoxStyle AMSGSTYLE_TERMINATEAPPONDISPLAY = MESSAGE_STYLE_TERMINATE_ON_DISPLAY; // DEPRECATED
	*/

	SystemInfo::SystemInfo()
	{
		this->name = "";
#ifdef _ARM
		this->architecture = "ARM";
#elif defined(_X64)
		this->architecture = "x64";
#else
		this->architecture = "x86";
#endif
		this->osVersion = 1.0f;
		this->cpuCores = 1;
		this->ram = 256;
		this->maxTextureSize = 0;
		this->displayDpi = 0;
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
				// TODOa - move this in their appropriate classes
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
			*ok = customButtonTitles.try_get_by_key(MESSAGE_BUTTON_OK, "OK");
			*cancel = customButtonTitles.try_get_by_key(MESSAGE_BUTTON_CANCEL, "Cancel");
		}
		else if ((buttonMask & MESSAGE_BUTTON_YES) && (buttonMask & MESSAGE_BUTTON_NO) && (buttonMask & MESSAGE_BUTTON_CANCEL))
		{
			*yes = customButtonTitles.try_get_by_key(MESSAGE_BUTTON_YES, "Yes");
			*no = customButtonTitles.try_get_by_key(MESSAGE_BUTTON_NO, "No");
			*cancel = customButtonTitles.try_get_by_key(MESSAGE_BUTTON_CANCEL, "Cancel");
		}
		else if (buttonMask & MESSAGE_BUTTON_OK)
		{
			*ok = customButtonTitles.try_get_by_key(MESSAGE_BUTTON_OK, "OK");
		}
		else if ((buttonMask & MESSAGE_BUTTON_YES) && (buttonMask & MESSAGE_BUTTON_NO))
		{
			*yes = customButtonTitles.try_get_by_key(MESSAGE_BUTTON_YES, "Yes");
			*no = customButtonTitles.try_get_by_key(MESSAGE_BUTTON_NO, "No");
		}
	}

}
