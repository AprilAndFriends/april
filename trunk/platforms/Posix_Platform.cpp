/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @version 2.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _UNIX
#include <gtypes/Vector2.h>
#include <hltypes/hlog.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hstring.h>

#include "april.h"
#include "Platform.h"

namespace april
{
	SystemInfo getSystemInfo()
	{
		// TODO
		static SystemInfo info;
		if (info.locale == "")
		{
			info.cpuCores = 1; // TODO
			info.displayResolution.set(1024.0f, 768.0f); // TODO
			info.displayDpi = 96; // TODO
			//info.ram = 1024; // TODO
			info.locale = "en"; // TODO
		}
		if (info.maxTextureSize == 0 && april::rendersys != NULL)
		{
			info.maxTextureSize = april::rendersys->_getMaxTextureSize();
		}
		return info;
	}

	DeviceType getDeviceType()
	{
		return DEVICE_LINUX_PC;
	}
	
	hstr getPackageName()
	{
		hlog::warn(april::logTag, "Cannot use getPackageName() on this platform.");
		return "";
	}

	MessageBoxButton messageBox_platform(chstr title, chstr text, MessageBoxButton buttonMask, MessageBoxStyle style, hmap<MessageBoxButton, hstr> customButtonTitles, void(*callback)(MessageBoxButton))
	{
		// TODO
		return AMSGBTN_OK;
	}

}
#endif
