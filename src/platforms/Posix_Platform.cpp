/// @file
/// @version 4.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifdef _UNIX
#include <gtypes/Vector2.h>
#include <hltypes/hlog.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hstring.h>

#include "april.h"
#include "Platform.h"

namespace april
{
	extern SystemInfo info;
	
	void _setupSystemInfo_platform(SystemInfo& info)
	{
		info.cpuCores = 1; // TODO
		info.displayResolution.set(1024.0f, 768.0f); // TODO
		info.displayDpi = 96.0f; // TODO
		//info.ram = 1024; // TODO
		info.locale = "en"; // TODO
	}

	hstr _getPackageName_platform()
	{
		hlog::warn(logTag, "Cannot use getPackageName() on this platform.");
		return "";
	}

	hstr _getUserDataPath_platform()
	{
		hlog::warn(logTag, "Cannot use getUserDataPath() on this platform.");
		return ".";
	}
	
	int64_t _getRamConsumption_platform()
	{
		hlog::warn(logTag, "Cannot use getRamConsumption() on this platform.");
		return 0LL;
	}
	
	bool _openUrl_platform(chstr url)
	{
		hlog::warn(logTag, "Cannot use openUrl() on this platform.");
		return false;
	}
	
	void _showMessageBox_platform(chstr title, chstr text, MessageBoxButton buttons, MessageBoxStyle style, hmap<MessageBoxButton, hstr> customButtonTitles, void (*callback)(const MessageBoxButton&), bool modal)
	{
		hlog::warn(logTag, "Cannot use showMessageBox() on this platform.");
	}

}
#endif
