/// @file
/// @version 4.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#if defined(_WIN32) && !defined(_OPENKODE) && !defined(_WINRT)
#include <stdio.h>

#define __HL_INCLUDE_PLATFORM_HEADERS
#include <gtypes/Vector2.h>
#include <hltypes/hdir.h>
#include <hltypes/hlog.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hmap.h>
#include <hltypes/hplatform.h>
#include <hltypes/hstring.h>

#define PSAPI_VERSION 1
// needed for GetProcessMemoryInfo()
#include <Psapi.h> // has to be here after hplatform.h that includes windows.h
#pragma comment(lib, "psapi.lib")

#include "april.h"
#include "Platform.h"
#include "RenderSystem.h"
#include "Window.h"

namespace april
{
	extern SystemInfo info;

	static hstr _getWindowsName()
	{
		OSVERSIONINFOEX osVersionInfo;
		memset(&osVersionInfo, 0, sizeof(OSVERSIONINFOEX));
		osVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
		osVersionInfo.dwMajorVersion = 5;
		osVersionInfo.dwMinorVersion = 0;
		DWORDLONG conditionMask = 0;
		VER_SET_CONDITION(conditionMask, VER_MAJORVERSION, VER_EQUAL);
		VER_SET_CONDITION(conditionMask, VER_MINORVERSION, VER_EQUAL);
		if (VerifyVersionInfoW(&osVersionInfo, VER_MAJORVERSION, conditionMask) != 0)
		{
			return " XP";
		}
		osVersionInfo.dwMajorVersion = 6;
		if (VerifyVersionInfoW(&osVersionInfo, VER_MAJORVERSION, conditionMask) != 0)
		{
			if (VerifyVersionInfoW(&osVersionInfo, VER_MINORVERSION, conditionMask) != 0)
			{
				return " Vista";
			}
			osVersionInfo.dwMinorVersion = 1;
			if (VerifyVersionInfoW(&osVersionInfo, VER_MINORVERSION, conditionMask) != 0)
			{
				return " 7";
			}
			osVersionInfo.dwMinorVersion = 2;
			if (VerifyVersionInfoW(&osVersionInfo, VER_MINORVERSION, conditionMask) != 0)
			{
				return " 8";
			}
			osVersionInfo.dwMinorVersion = 3;
			if (VerifyVersionInfoW(&osVersionInfo, VER_MINORVERSION, conditionMask) != 0)
			{
				return " 8.1";
			}
			// all future 6.x Windows versions will be labeled as "8.x" to avoid assumptions
			return " 8.x";
		}
		osVersionInfo.dwMajorVersion = 10;
		if (VerifyVersionInfoW(&osVersionInfo, VER_MAJORVERSION, conditionMask) != 0)
		{
			return " 10";
		}
		return "";
	}
	
	SystemInfo getSystemInfo()
	{
		if (info.locale == "")
		{
			info.name = "Windows" + _getWindowsName();
			info.deviceName = "WindowsDevice";
			info.architecture = "x86";
			// number of CPU cores
			SYSTEM_INFO w32info;
			GetNativeSystemInfo(&w32info);
			info.cpuCores = w32info.dwNumberOfProcessors;
			// RAM size
			MEMORYSTATUSEX status;
			status.dwLength = sizeof(status);
			GlobalMemoryStatusEx(&status);
			info.ram = (int)(status.ullTotalPhys / 1048576LL);
			// display resolution
			info.displayResolution.set((float)GetSystemMetrics(SM_CXSCREEN), (float)GetSystemMetrics(SM_CYSCREEN));
			// display DPI
			info.displayDpi = 96.0f;
			// other
			info.locale = "en"; // default is "en"
			wchar_t locale[LOCALE_NAME_MAX_LENGTH] = { 0 };
			int length = GetLocaleInfoW(LOCALE_USER_DEFAULT, LOCALE_SISO639LANGNAME, locale, (LOCALE_NAME_MAX_LENGTH - 1) * sizeof(wchar_t));
			if (length > 0)
			{
				info.locale = hstr::fromUnicode(locale);
			}
			length = GetLocaleInfoW(LOCALE_USER_DEFAULT, LOCALE_SISO3166CTRYNAME, locale, (LOCALE_NAME_MAX_LENGTH - 1) * sizeof(wchar_t));
			if (length > 0)
			{
				info.localeVariant = hstr::fromUnicode(locale);
			}
			info.locale = info.locale.lowered();
			info.localeVariant = info.localeVariant.uppered();
		}
		return info;
	}

	hstr getPackageName()
	{
		hlog::warn(logTag, "Cannot use getPackageName() on this platform.");
		return "";
	}

	hstr getUserDataPath()
	{
		return henv("APPDATA");
	}
	
	int64_t getRamConsumption()
	{
		int64_t result = 0LL;
		PROCESS_MEMORY_COUNTERS counters;
		if (GetProcessMemoryInfo(GetCurrentProcess(), &counters, sizeof(counters)))
		{
			result = (int64_t)counters.WorkingSetSize;
		}
		return result;
	}
	
	bool openUrl(chstr url)
	{
		hlog::write(logTag, "Opening URL: " + url);
		ShellExecuteW(NULL, L"open", url.wStr().c_str(), NULL, NULL, SW_SHOWNORMAL);
		return true;
	}

	static void (*currentCallback)(MessageBoxButton) = NULL;

	void _messageBoxResult(int button)
	{
		switch (button)
		{
		case IDOK:
			if (currentCallback != NULL)
			{
				(*currentCallback)(MessageBoxButton::Ok);
			}
			break;
		case IDYES:
			if (currentCallback != NULL)
			{
				(*currentCallback)(MessageBoxButton::Yes);
			}
			break;
		case IDNO:
			if (currentCallback != NULL)
			{
				(*currentCallback)(MessageBoxButton::No);
			}
			break;
		case IDCANCEL:
			if (currentCallback != NULL)
			{
				(*currentCallback)(MessageBoxButton::Cancel);
			}
			break;
		default:
			hlog::error(logTag, "Unknown message box callback: " + hstr(button));
			break;
		}
	}

	void messageBox_platform(chstr title, chstr text, MessageBoxButton buttons, MessageBoxStyle style,
		hmap<MessageBoxButton, hstr> customButtonTitles, void(*callback)(MessageBoxButton), bool modal)
	{
		currentCallback = callback;
		int type = 0;
		if (buttons == MessageBoxButton::OkCancel)
		{
			type |= MB_OKCANCEL;
		}
		else if (buttons == MessageBoxButton::YesNoCancel)
		{
			type |= MB_YESNOCANCEL;
		}
		else if (buttons == MessageBoxButton::Ok)
		{
			type |= MB_OK;
		}
		else if (buttons == MessageBoxButton::YesNo)
		{
			type |= MB_YESNO;
		}
		if (style == MessageBoxStyle::Info)
		{
			type |= MB_ICONINFORMATION;
		}
		else if (style == MessageBoxStyle::Warning)
		{
			type |= MB_ICONWARNING;
		}
		else if (style == MessageBoxStyle::Critical)
		{
			type |= MB_ICONSTOP;
		}
		else if (style == MessageBoxStyle::Question)
		{
			type |= MB_ICONQUESTION;
		}
		HWND hwnd = 0;
		if (april::window != NULL && modal)
		{
			hwnd = (HWND)april::window->getBackendId();
		}
		int button = MessageBoxW(hwnd, text.wStr().c_str(), title.wStr().c_str(), type);
		_messageBoxResult(button);
	}

}
#endif
