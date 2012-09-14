/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @author  Ivan Vucica
/// @version 2.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _WIN32
#include <windows.h>

#include <gtypes/Vector2.h>
#include <hltypes/hmap.h>
#include <hltypes/hstring.h>

#include "Platform.h"
#include "RenderSystem.h"
#include "Window.h"

namespace april
{
	SystemInfo getSystemInfo()
	{
		// TODO
		static SystemInfo info;
		if (info.locale == "")
		{
			// number of CPU cores
			SYSTEM_INFO w32info;
			GetSystemInfo(&w32info);
			info.cpuCores = w32info.dwNumberOfProcessors;
			// RAM size
			MEMORYSTATUSEX status;
			status.dwLength = sizeof(status);
			GlobalMemoryStatusEx(&status);
			info.ram = (int)(status.ullTotalPhys / 1048576);
			// display resolution
			info.displayResolution.set((float)GetSystemMetrics(SM_CXSCREEN), (float)GetSystemMetrics(SM_CYSCREEN));
			// display DPI
			info.displayDpi = 96;
			// other
			info.locale = "en"; // TODO
		}
		// TODO
		if (info.maxTextureSize == 0 && april::rendersys != NULL)
		{
			info.maxTextureSize = april::rendersys->_getMaxTextureSize();
		}
		return info;
	}

	DeviceType getDeviceType()
	{
		return DEVICE_WINDOWS_PC;
	}

	MessageBoxButton messageBox_platform(chstr title, chstr text, MessageBoxButton buttonMask, MessageBoxStyle style,
		hmap<MessageBoxButton, hstr> customButtonTitles, void(*callback)(MessageBoxButton))
	{
		HWND hwnd = 0;
		if (april::rendersys != NULL && april::window != NULL && (style & AMSGSTYLE_MODAL))
		{
			hwnd = (HWND)april::window->getBackendId();
		}
		int type = 0;
		if ((buttonMask & AMSGBTN_OK) && (buttonMask & AMSGBTN_CANCEL))
		{
			type |= MB_OKCANCEL;
		}
		else if ((buttonMask & AMSGBTN_YES) && (buttonMask & AMSGBTN_NO) && (buttonMask & AMSGBTN_CANCEL))
		{
			type |= MB_YESNOCANCEL;
		}
		else if (buttonMask & AMSGBTN_OK)
		{
			type |= MB_OK;
		}
		else if ((buttonMask & AMSGBTN_YES) && (buttonMask & AMSGBTN_NO))
		{
			type |= MB_YESNO;
		}
		
		if (style & AMSGSTYLE_INFORMATION)
		{
			type |= MB_ICONINFORMATION;
		}
		else if (style & AMSGSTYLE_WARNING)
		{
			type |= MB_ICONWARNING;
		}
		else if (style & AMSGSTYLE_CRITICAL)
		{
			type |= MB_ICONSTOP;
		}
		else if (style & AMSGSTYLE_QUESTION)
		{
			type |= MB_ICONQUESTION;
		}
		
		int button = MessageBox(hwnd, text.c_str(), title.c_str(), type);
		switch (button)
		{
		case IDOK:
			if (callback != NULL)
			{
				(*callback)(AMSGBTN_OK);
			}
			return AMSGBTN_OK;
		case IDYES:
			if (callback != NULL)
			{
				(*callback)(AMSGBTN_YES);
			}
			return AMSGBTN_YES;
		case IDNO:
			if (callback != NULL)
			{
				(*callback)(AMSGBTN_NO);
			}
			return AMSGBTN_NO;
		case IDCANCEL:
			if (callback != NULL)
			{
				(*callback)(AMSGBTN_CANCEL);
			}
			return AMSGBTN_CANCEL;
		}
		return AMSGBTN_OK;
	}

}
#endif
