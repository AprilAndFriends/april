/// @file
/// @version 3.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <hltypes/hstring.h>

#include "main_base.h"
#include "Platform.h"

namespace april
{
#if defined(_WIN32) && !defined(_WINRT)
	static HANDLE lockMutex;
#endif

	bool __lockSingleInstanceMutex(hstr instanceName, chstr fallbackName)
	{
#if defined(_WIN32) && !defined(_WINRT)
		if (instanceName == "")
		{
			instanceName = fallbackName;
		}
		instanceName = instanceName.replaced("\\", "/");
		lockMutex = CreateMutexW(NULL, true, instanceName.wStr().c_str());
		if (lockMutex != 0 && GetLastError() == ERROR_ALREADY_EXISTS)
		{
			CloseHandle(lockMutex);
			april::messageBox("Warning", "Cannot launch '" + instanceName + "', already running!", april::MESSAGE_BUTTON_OK, april::MESSAGE_STYLE_WARNING);
			return false;
		}
#endif
		return true;
	}

	void __unlockSingleInstanceMutex()
	{
#if defined(_WIN32) && !defined(_WINRT)
		if (lockMutex != 0)
		{
			CloseHandle(lockMutex);
		}
#endif
	}

}

