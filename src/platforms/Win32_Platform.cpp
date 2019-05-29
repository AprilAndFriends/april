/// @file
/// @version 5.2
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#if defined(_WIN32) && !defined(_WINRT)
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

#include "Application.h"
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
			// MS completely ruined version detection from Win 8 onwards and requires a manifest to even
			// be able to tell what Windows version is running so let's just call it all Windows 10
			return " 10";
		}
		osVersionInfo.dwMajorVersion = 10;
		if (VerifyVersionInfoW(&osVersionInfo, VER_MAJORVERSION, conditionMask) != 0)
		{
			return " 10";
		}
		return "";
	}
	
	void _setupSystemInfo_platform(SystemInfo& info)
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
			info.displayResolution.set(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
			// display DPI
			info.displayDpi = 96.0f;
			if (info.displayResolution.y >= 1536)
			{
				// crude but works for now...
				info.displayScaleFactor = 2;
			}
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
	}

	hstr _getPackageName_platform()
	{
		hlog::warn(logTag, "Cannot use getPackageName() on this platform.");
		return "";
	}

	hstr _getUserDataPath_platform()
	{
		return henv("APPDATA");
	}

	int64_t _getRamConsumption_platform()
	{
		int64_t result = (int64_t)0;
		PROCESS_MEMORY_COUNTERS counters;
		if (GetProcessMemoryInfo(GetCurrentProcess(), &counters, sizeof(counters)))
		{
			result = (int64_t)counters.WorkingSetSize;
		}
		return result;
	}
	
	void _getNotchOffsets_platform(gvec2i& topLeft, gvec2i& bottomRight, bool landscape)
	{
		topLeft.set(0, 0);
		bottomRight.set(0, 0);
	}

	bool _openUrl_platform(chstr url)
	{
		ShellExecuteW(NULL, L"open", url.wStr().c_str(), NULL, NULL, SW_SHOWNORMAL);
		return true;
	}

	void _showMessageBox_platform(const MessageBoxData& data)
	{
		int type = 0;
		if (data.buttons == MessageBoxButton::OkCancel)
		{
			type |= MB_OKCANCEL;
		}
		else if (data.buttons == MessageBoxButton::YesNoCancel)
		{
			type |= MB_YESNOCANCEL;
		}
		else if (data.buttons == MessageBoxButton::Ok)
		{
			type |= MB_OK;
		}
		else if (data.buttons == MessageBoxButton::YesNo)
		{
			type |= MB_YESNO;
		}
		if (data.style == MessageBoxStyle::Info)
		{
			type |= MB_ICONINFORMATION;
		}
		else if (data.style == MessageBoxStyle::Warning)
		{
			type |= MB_ICONWARNING;
		}
		else if (data.style == MessageBoxStyle::Critical)
		{
			type |= MB_ICONSTOP;
		}
		else if (data.style == MessageBoxStyle::Question)
		{
			type |= MB_ICONQUESTION;
		}
		HWND hwnd = 0;
		if (april::window != NULL && data.modal)
		{
			hwnd = (HWND)april::window->getBackendId();
		}
		int button = MessageBoxW(hwnd, data.text.wStr().c_str(), data.title.wStr().c_str(), type);
		switch (button)
		{
		case IDOK:
			april::Application::messageBoxCallback(MessageBoxButton::Ok);
			break;
		case IDYES:
			april::Application::messageBoxCallback(MessageBoxButton::Yes);
			break;
		case IDNO:
			april::Application::messageBoxCallback(MessageBoxButton::No);
			break;
		case IDCANCEL:
			april::Application::messageBoxCallback(MessageBoxButton::Cancel);
			break;
		default:
			april::Application::messageBoxCallback(MessageBoxButton::Ok);
			break;
		}
	}

}
#endif
