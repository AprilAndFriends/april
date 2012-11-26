/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @author  Ivan Vucica
/// @version 2.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _WIN32
#include <hltypes/hplatform.h>
#if !_HL_WINRT

#include <gtypes/Vector2.h>
#include <hltypes/hlog.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hmap.h>
#include <hltypes/hplatform.h>
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
			GetNativeSystemInfo(&w32info);
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

	hstr getPackageName()
	{
		hlog::warn("Cannot use getPackageName() on this platform.");
		return "";
	}

	static void(*currentCallback)(MessageBoxButton) = NULL;

	// TODO - could be maybe put as a common function
	void _messageBoxResult(int button)
	{
		switch (button)
		{
		case IDOK:
			if (currentCallback != NULL)
			{
				(*currentCallback)(AMSGBTN_OK);
			}
			break;
		case IDYES:
			if (currentCallback != NULL)
			{
				(*currentCallback)(AMSGBTN_YES);
			}
			break;
		case IDNO:
			if (currentCallback != NULL)
			{
				(*currentCallback)(AMSGBTN_NO);
			}
			break;
		case IDCANCEL:
			if (currentCallback != NULL)
			{
				(*currentCallback)(AMSGBTN_CANCEL);
			}
			break;
		}
	}

	void messageBox_platform(chstr title, chstr text, MessageBoxButton buttonMask, MessageBoxStyle style,
		hmap<MessageBoxButton, hstr> customButtonTitles, void(*callback)(MessageBoxButton))
	{
		currentCallback = callback;
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

		HWND hwnd = 0;
		if (april::rendersys != NULL && april::window != NULL && (style & AMSGSTYLE_MODAL))
		{
			hwnd = (HWND)april::window->getBackendId();
		}
		int button = MessageBoxW(hwnd, text.w_str().c_str(), title.w_str().c_str(), type);
		_messageBoxResult(button);
	}

}
#endif
#endif
