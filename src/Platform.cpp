/// @file
/// @version 5.2
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

#include "Application.h"
#include "april.h"
#include "main_base.h"
#include "Platform.h"
#include "Window.h"

namespace april
{
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

	MessageBoxData::MessageBoxData(chstr title, chstr text, MessageBoxButton buttons, MessageBoxStyle style, hmap<MessageBoxButton, hstr> customButtonTitles,
		void (*callback)(const MessageBoxButton&), bool modal, bool applicationFinishAfterDisplay)
	{
		this->title = title;
		this->text = text;
		this->buttons = buttons;
		this->style = style;
		this->customButtonTitles = customButtonTitles;
		this->callback = callback;
		this->modal = modal;
		this->applicationFinishAfterDisplay = applicationFinishAfterDisplay;
	}

	extern void _setupSystemInfo_platform(SystemInfo& info);
	extern hstr _getPackageName_platform();
	extern hstr _getUserDataPath_platform();
	extern int64_t _getRamConsumption_platform();
	extern void _getNotchOffsets_platform(gvec2i& topLeft, gvec2i& bottomRight, bool landscape = true);
	extern bool _openUrl_platform(chstr url);
	extern void _showMessageBox_platform(const MessageBoxData&);

	void (*_setupSystemInfo)(SystemInfo& info) = &_setupSystemInfo_platform;
	hstr (*_getPackageName)() = &_getPackageName_platform;
	hstr (*_getUserDataPath)() = &_getUserDataPath_platform;
	int64_t (*_getRamConsumption)() = &_getRamConsumption_platform;
	void (*_getNotchOffsets)(gvec2i&, gvec2i&, bool) = &_getNotchOffsets_platform;
	bool (*_openUrl)(chstr) = &_openUrl_platform;
	void (*_showMessageBox)(const MessageBoxData&) = &_showMessageBox_platform;

	SystemInfo info;
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
		this->displayScaleFactor = 1.0f;
		this->locale = "";
	}
	
	/*
	grecti SystemInfo::getNotchedRect(bool landscape) const
	{
		if (this->name == "iPhone X")
		{
			gvec2i size = this->displayResolution;
			int notchMargin = 132; // Apple's 44pt @3x
			if (landscape)
			{
				int homeButtonMargin = 69; // Apple's 23pt @3x
				size.x -= notchMargin * 2;
				size.y -= homeButtonMargin;
				return grecti(notchMargin, 0, size);
			}
			hswap(size.x, size.y);
			size.y -= notchMargin * 2;
			return grecti(0, notchMargin, size);
		}
		return grecti(0, 0, this->displayResolution);
	}
	*/

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

	void getNotchOffsets(gvec2i& topLeft, gvec2i& bottomRight, bool landscape)
	{
		if (_getNotchOffsets != NULL)
		{
			(*_getNotchOffsets)(topLeft, bottomRight, landscape);
			return;
		}
		topLeft.set(0, 0);
		bottomRight.set(0, 0);
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

	void showMessageBox(chstr title, chstr text, MessageBoxButton buttons, MessageBoxStyle style, hmap<MessageBoxButton, hstr> customButtonTitles,
		void (*callback)(const MessageBoxButton&), bool modal, bool applicationFinishAfterDisplay)
	{
		if (_showMessageBox != NULL)
		{
			april::application->queueMessageBox(MessageBoxData(title, text, buttons, style, customButtonTitles, callback, modal, applicationFinishAfterDisplay));
		}
		else
		{
			hlog::warn(logTag, "Cannot use showMessageBox() on this platform.");
		}
	}

	void _processMessageBox(const MessageBoxData& data)
	{
		(*_showMessageBox)(data);
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
