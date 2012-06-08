/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @version 2.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _UNIX
#include <gtypes/Vector2.h>

#include "Platform.h"

namespace april
{
	gvec2 getDisplayResolution()
	{
		// TODO
		return gvec2();
	}

	SystemInfo getSystemInfo()
	{
		// TODO
		static SystemInfo info;
		if (info.locale == "")
		{
			info.cpu_cores = 1; // TODO
			info.ram = 1024;
			info.max_texture_size = 0;
			info.locale = "en";
		}
		// TODO
		if (info.max_texture_size == 0 && april::rendersys != NULL)
		{
			info.max_texture_size = april::rendersys->_getMaxTextureSize();
		}
		return info;
	}

	DeviceType getDeviceType()
	{
		return DEVICE_LINUX_PC;
	}
	
	MessageBoxButton messageBox_platform(chstr title, chstr text, MessageBoxButton buttonMask, MessageBoxStyle style, hmap<MessageBoxButton, hstr> customButtonTitles, void(*callback)(MessageBoxButton))
	{
		// TODO
		return AMSGBTN_OK;
	}

}
#endif
