/// @file
/// @author  Boris Mikic
/// @version 2.4
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/// 
/// @section DESCRIPTION
/// 
/// Defines platform specific functionality.

#ifndef APRIL_PLATFORM_H
#define APRIL_PLATFORM_H

#include <gtypes/Vector2.h>
#include <hltypes/hmap.h>
#include <hltypes/hstring.h>

#include "aprilExport.h"

namespace april
{
	struct SystemInfo
	{
		hstr name;
		int cpuCores; // number of CPU cores or separate CPU units
		int ram; // how many MB of RAM does the host system have in total
		int maxTextureSize;
		gvec2 displayResolution;
		int displayDpi;
		hstr locale; // current system locale code
		SystemInfo() : name(""), cpuCores(1), ram(256), maxTextureSize(0), displayDpi(0), locale("") { }
	};
	
	enum DeviceType
	{
		DEVICE_IPHONE = 0,
		DEVICE_IPAD,
		DEVICE_ANDROID_PHONE,
		DEVICE_ANDROID_TABLET,
		DEVICE_WINDOWS_PC,
		DEVICE_LINUX_PC,
		DEVICE_MAC_PC,
		DEVICE_WINDOWS_PHONE,
		DEVICE_WINDOWS_TABLET,
		DEVICE_UNKNOWN_LARGE,
		DEVICE_UNKNOWN_MEDIUM,
		DEVICE_UNKNOWN_SMALL,
		DEVICE_UNKNOWN
	};

	enum MessageBoxButton
	{
		AMSGBTN_NULL = 0,

		AMSGBTN_OK = 1,
		AMSGBTN_CANCEL = 2,
		AMSGBTN_YES = 4,
		AMSGBTN_NO = 8,
			
		AMSGBTN_OKCANCEL = AMSGBTN_OK | AMSGBTN_CANCEL,
		AMSGBTN_YESNO = AMSGBTN_YES | AMSGBTN_NO,
		AMSGBTN_YESNOCANCEL = AMSGBTN_YESNO | AMSGBTN_CANCEL

	};
	
	enum MessageBoxStyle
	{
		AMSGSTYLE_PLAIN = 0,
			
		AMSGSTYLE_INFORMATION = 1,
		AMSGSTYLE_WARNING = 2,
		AMSGSTYLE_CRITICAL = 3,
		AMSGSTYLE_QUESTION = 4,
			
		AMSGSTYLE_MODAL = 8,
		AMSGSTYLE_TERMINATEAPPONDISPLAY = 16

	};
		
	aprilFnExport SystemInfo getSystemInfo();
	aprilFnExport DeviceType getDeviceType();
	aprilFnExport MessageBoxButton messageBox(chstr title, chstr text, MessageBoxButton buttonMask = AMSGBTN_OK, MessageBoxStyle style = AMSGSTYLE_PLAIN,
		hmap<MessageBoxButton, hstr> customButtonTitles = hmap<MessageBoxButton, hstr>(), void(*callback)(MessageBoxButton) = NULL);

	MessageBoxButton messageBox_platform(chstr title, chstr text, MessageBoxButton buttonMask = AMSGBTN_OK, MessageBoxStyle style = AMSGSTYLE_PLAIN,
		hmap<MessageBoxButton, hstr> customButtonTitles = hmap<MessageBoxButton, hstr>(), void(*callback)(MessageBoxButton) = NULL);

}

#endif
