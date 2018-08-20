/// @file
/// @version 5.2
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#define __HL_INCLUDE_PLATFORM_HEADERS
#include <hltypes/hplatform.h>
#include <hltypes/hstring.h>

#include "main_base.h"
#include "Platform.h"

namespace april
{
#if defined(_WIN32) && !defined(_UWP)
	static HANDLE lockMutex;
#endif

	bool __lockSingleInstanceMutex(hstr instanceName, chstr fallbackName)
	{
#if defined(_WIN32) && !defined(_UWP)
		if (instanceName == "")
		{
			instanceName = fallbackName;
		}
		instanceName.replace("\\", "/");
		lockMutex = CreateMutexW(NULL, true, instanceName.wStr().c_str());
		if (lockMutex != 0 && GetLastError() == ERROR_ALREADY_EXISTS)
		{
			CloseHandle(lockMutex);
			april::showMessageBox("Warning", "Cannot launch '" + instanceName + "', already running!", april::MessageBoxButton::Ok, april::MessageBoxStyle::Warning);
			return false;
		}
#endif
		return true;
	}

	void __unlockSingleInstanceMutex()
	{
#if defined(_WIN32) && !defined(_UWP)
		if (lockMutex != 0)
		{
			CloseHandle(lockMutex);
		}
#endif
	}

}
