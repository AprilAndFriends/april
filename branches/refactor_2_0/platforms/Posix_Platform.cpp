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
