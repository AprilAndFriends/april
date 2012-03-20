/// @file
/// @author  Boris Mikic
/// @version 1.51
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _ANDROID
#include <gtypes/Vector2.h>

#include "Platform.h"

namespace april
{
	extern gvec2 androidResolution; // TODO

	gvec2 getDisplayResolution()
	{
		// TODO
		return april::androidResolution;
	}

	SystemInfo getSystemInfo()
	{
		// TODO
		static SystemInfo info;
		return info;
	}

	DeviceType getDeviceType()
	{
		// TODO
		return DEVICE_ANDROID_PHONE;
	}

	MessageBoxButton messageBox_platform(chstr title, chstr text, MessageBoxButton buttonMask, MessageBoxStyle style,
		hmap<MessageBoxButton, hstr> customButtonTitles, void(*callback)(MessageBoxButton))
	{
		// TODO
		return AMSGBTN_OK;
	}

}
#endif
