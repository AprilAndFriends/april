/// @file
/// @version 4.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <stdlib.h>

#include <hltypes/harray.h>
#include <hltypes/hlog.h>
#include <hltypes/hmap.h>
#include <hltypes/hstring.h>

#include "april.h"
#include "main_base.h"
#include "Platform.h"
#include "Window.h"

namespace april
{
	// DEPRECATED
	void messageBox(chstr title, chstr text, MessageBoxButton buttons, MessageBoxStyle style,
		hmap<MessageBoxButton, hstr> customButtonTitles, void(*callback)(MessageBoxButton), bool modal, bool terminateOnDisplay)
	{
		showMessageBox(title, text, buttons, style, customButtonTitles, callback, modal, terminateOnDisplay);
	}


	HL_ENUM_CLASS_DEFINE(MessageBoxButton,
	(
		HL_ENUM_DEFINE(MessageBoxButton, Ok);
		HL_ENUM_DEFINE(MessageBoxButton, Cancel);
		HL_ENUM_DEFINE(MessageBoxButton, Yes);
		HL_ENUM_DEFINE(MessageBoxButton, No);
		HL_ENUM_DEFINE(MessageBoxButton, OkCancel);
		HL_ENUM_DEFINE(MessageBoxButton, YesNo);
		HL_ENUM_DEFINE(MessageBoxButton, YesNoCancel);

		bool MessageBoxButton::hasOk() const
		{
			return ((*this) == Ok || (*this) == OkCancel);
		}

		bool MessageBoxButton::hasCancel() const
		{
			return ((*this) == Cancel || (*this) == OkCancel || (*this) == YesNoCancel);
		}

		bool MessageBoxButton::hasYes() const
		{
			return ((*this) == Yes || (*this) == YesNo || (*this) == YesNoCancel);
		}

		bool MessageBoxButton::hasNo() const
		{
			return ((*this) == No || (*this) == YesNo || (*this) == YesNoCancel);
		}

	));

	HL_ENUM_CLASS_DEFINE(MessageBoxStyle,
	(
		HL_ENUM_DEFINE(MessageBoxStyle, Normal);
		HL_ENUM_DEFINE(MessageBoxStyle, Info);
		HL_ENUM_DEFINE(MessageBoxStyle, Warning);
		HL_ENUM_DEFINE(MessageBoxStyle, Critical);
		HL_ENUM_DEFINE(MessageBoxStyle, Question);
	));

	extern void _setupSystemInfo_platform(SystemInfo& info);
	extern hstr _getPackageName_platform();
	extern hstr _getUserDataPath_platform();
	extern int64_t _getRamConsumption_platform();
	extern bool _openUrl_platform(chstr url);
	extern void _showMessageBox_platform(chstr, chstr, MessageBoxButton, MessageBoxStyle, hmap<MessageBoxButton, hstr>, void(*)(MessageBoxButton), bool);

	void (*_setupSystemInfo)(SystemInfo& info) = &_setupSystemInfo_platform;
	hstr (*_getPackageName)() = &_getPackageName_platform;
	hstr (*_getUserDataPath)() = &_getUserDataPath_platform;
	int64_t (*_getRamConsumption)() = &_getRamConsumption_platform;
	bool (*_openUrl)(chstr) = &_openUrl_platform;
	void (*_showMessageBox)(chstr, chstr, MessageBoxButton, MessageBoxStyle, hmap<MessageBoxButton, hstr>, void (*)(MessageBoxButton), bool) = &_showMessageBox_platform;

	static SystemInfo info;
	harray<hstr> args;

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
		this->osVersion.set(1);
		this->cpuCores = 1;
		this->ram = 256;
		this->displayDpi = 0.0f;
		this->locale = "";
	}
	
	SystemInfo::~SystemInfo()
	{
	}

	harray<hstr> getArgs()
	{
		return args;
	}

	SystemInfo getSystemInfo()
	{
		if (_setupSystemInfo != NULL)
		{
			(*_setupSystemInfo)(info);
		}
		else if (info.locale == "")
		{
			hlog::warn(logTag, "_setupSystemInfo() has not been set up on this platform.");
		}
		return info;
	}

	hstr getPackageName()
	{
		if (_getPackageName != NULL)
		{
			return (*_getPackageName)();
		}
		hlog::warn(logTag, "Cannot use getPackageName() on this platform.");
		return "";
	}

	hstr getUserDataPath()
	{
		if (_getUserDataPath != NULL)
		{
			return (*_getUserDataPath)();
		}
		hlog::warn(logTag, "Cannot use getUserDataPath() on this platform.");
		return ".";
	}

	int64_t getRamConsumption()
	{
		if (_getRamConsumption != NULL)
		{
			return (*_getRamConsumption)();
		}
		hlog::warn(logTag, "Cannot use getRamConsumption() on this platform.");
		return 0LL;
	}

	bool openUrl(chstr url)
	{
		bool result = false;
		hlog::write(logTag, "Opening URL: " + url);
		if (_openUrl != NULL)
		{
			result = (*_openUrl)(url);
			if (!result)
			{
				hlog::warn(logTag, "Could not open URL!");
			}
		}
		else
		{
			hlog::warn(logTag, "Cannot use openUrl() on this platform.");
		}
		return result;
	}

	void showMessageBox(chstr title, chstr text, MessageBoxButton buttons, MessageBoxStyle style,
		hmap<MessageBoxButton, hstr> customButtonTitles, void (*callback)(MessageBoxButton), bool modal, bool terminateOnDisplay)
	{
		if (terminateOnDisplay)
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
			modal = true;
		}
		if (_showMessageBox != NULL)
		{
			(*_showMessageBox)(title, text, buttons, style, customButtonTitles, callback, modal);
		}
		else
		{
			hlog::warn(logTag, "Cannot use showMessageBox() on this platform.");
		}
		if (terminateOnDisplay)
		{
			exit(0);
		}
	}

	void _makeButtonLabels(hstr* ok, hstr* yes, hstr* no, hstr* cancel, MessageBoxButton buttons, hmap<MessageBoxButton, hstr> customButtonTitles)
	{
		if (buttons.hasOk())
		{
			*ok = customButtonTitles.tryGet(MessageBoxButton::Ok, "OK");
		}
		if (buttons.hasCancel())
		{
			*cancel = customButtonTitles.tryGet(MessageBoxButton::Cancel, "Cancel");
		}
		if (buttons.hasYes())
		{
			*yes = customButtonTitles.tryGet(MessageBoxButton::Yes, "Yes");
		}
		if (buttons.hasNo())
		{
			*no = customButtonTitles.tryGet(MessageBoxButton::No, "No");
		}
	}

}
