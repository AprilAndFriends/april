/// @file
/// @author  Boris Mikic
/// @version 3.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _OPENKODE
#include <KD/kd.h>

#include <gtypes/Vector2.h>
#include <hltypes/hlog.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hmap.h>
#include <hltypes/hplatform.h>
#include <hltypes/hstring.h>

#include "april.h"
#ifdef _EGL
#include "egl.h"
#endif
#include "Platform.h"
#include "RenderSystem.h"
#include "Window.h"

namespace april
{
	SystemInfo getSystemInfo()
	{
		static SystemInfo info;
		if (info.displayResolution.x == 0.0f)
		{
			// display resolution and DPI
			int width = 0;
			int height = 0;
			april::egl.getSystemParameters(&width, &height, &info.displayDpi);
			info.displayResolution.set((float)width, (float)height);
		}
		if (info.locale == "")
		{
			// TODO
			// number of CPU cores
			info.cpuCores = 1;
			// RAM size
			info.ram = 1024;
			// other
			info.locale = hstr(kdGetLocale());
			if (info.locale == "")
			{
				info.locale = "en"; // default is "en"
			}
			else if (info.locale.utf8_size() > 2)
			{
				info.locale = info.locale.utf8_substr(0, 2);
			}
		}
		if (info.maxTextureSize == 0 && april::rendersys != NULL)
		{
			info.maxTextureSize = april::rendersys->getMaxTextureSize();
		}
		return info;
	}

	DeviceType getDeviceType()
	{
		return DEVICE_OPENKODE;
	}

	hstr getPackageName()
	{
		hlog::warn(april::logTag, "Cannot use getPackageName() on this platform.");
		return "";
	}

	hstr getUserDataPath()
	{
		hlog::warn(april::logTag, "Cannot use getUserDataPath() on this platform.");
		return ".";
	}
	
	void messageBox_platform(chstr title, chstr text, MessageBoxButton buttonMask, MessageBoxStyle style,
		hmap<MessageBoxButton, hstr> customButtonTitles, void(*callback)(MessageBoxButton))
	{
		hlog::warn(april::logTag, "Cannot use messageBox() on this platform.");
	}

}
#endif
