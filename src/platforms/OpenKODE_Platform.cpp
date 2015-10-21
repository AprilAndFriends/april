/// @file
/// @version 3.7
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifdef _OPENKODE
#include <KD/kd.h>

#define __HL_INCLUDE_PLATFORM_HEADERS
#include <gtypes/Vector2.h>
#include <hltypes/hlog.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hmap.h>
#include <hltypes/hplatform.h>
#include <hltypes/hstring.h>

#include "april.h"

#ifdef _EGL
#include "egl.h"
#endif
#include "Platform.h"
#include "RenderSystem.h"
#include "Window.h"

#ifdef __APPLE__
	#include <TargetConditionals.h>
	#include <sys/sysctl.h>
	#ifdef _IOS
		void getStaticiOSInfo(chstr name, april::SystemInfo& info);
	#endif
#endif

#ifdef _ANDROID
#include <jni.h>
#define __NATIVE_INTERFACE_CLASS "com/april/NativeInterface"
#include "androidUtilJNI.h"
#include <unistd.h>
#endif

#ifdef _ANDROID
	#define debug_log(s) hlog::write(logTag, s)
#else
	#define debug_log(s) 
#endif

namespace april
{
	extern SystemInfo info;

	SystemInfo getSystemInfo()
	{
		if (info.locale == "")
		{
			info.name = "KD";
			debug_log("Fetching OpenKODE system info");
			// number of CPU cores
			info.cpuCores = 1;
			// display resolution
			int width = 0;
			int height = 0;
			
			debug_log("getting screen info");
			kdQueryAttribi(KD_ATTRIB_WIDTH, (KDint*)&width);
			kdQueryAttribi(KD_ATTRIB_HEIGHT, (KDint*)&height);
			info.displayResolution.set((float)hmax(width, height), (float)hmin(width, height));
			// display DPI
			int dpi = 0;
			kdQueryAttribi(KD_ATTRIB_DPI, (KDint*)&dpi);
			info.displayDpi = (float)dpi;
			// name
#ifdef _IOS // On iOS, april prefers to use hardcoded device info than OpenKODE's info, it's more accurate
			size_t size = 255;
			char cname[256] = {'\0'};
			sysctlbyname("hw.machine", cname, &size, NULL, 0);
			info.name = cname; // defaults for unknown devices
#elif defined(__APPLE__) && defined(_PC_INPUT) // mac
			info.name = "mac";
#elif defined(_WINRT)
			info.name = "winrt";
#elif defined(_ANDROID)
			info.name = "android";
#elif defined(_WIN32)
			info.name = "Windows";
#endif
			// misc
#ifdef _IOS // On iOS, april prefers to use hardcoded device info than OpenKODE's info, it's more accurate
			hstr model = kdQueryAttribcv(KD_ATTRIB_PLATFORM);
			if (model.contains("(") && model.contains(")"))
			{
				hstr a, b;
				model.split("(",a, b);
				b.split(")", model, a);
				getStaticiOSInfo(model, info);
			}
#elif defined(__APPLE__) && defined(_PC_INPUT) // mac
			info.cpuCores = sysconf(_SC_NPROCESSORS_ONLN);
#elif defined(_WINRT)
			SYSTEM_INFO w32info;
			GetNativeSystemInfo(&w32info);
			info.cpuCores = w32info.dwNumberOfProcessors;
			info.osVersion = 8.1f;
#elif defined(_ANDROID)
			debug_log("getting java stuff");
			APRIL_GET_NATIVE_INTERFACE_CLASS(classNativeInterface);
			// CPU cores
			debug_log("getting cpu cores");
			info.cpuCores = sysconf(_SC_NPROCESSORS_CONF);
			// OS version
			debug_log("getting os version");
			jmethodID methodGetOsVersion = env->GetStaticMethodID(classNativeInterface, "getOsVersion", _JARGS(_JSTR, ));
			harray<hstr> osVersions = _JSTR_TO_HSTR((jstring)env->CallStaticObjectMethod(classNativeInterface, methodGetOsVersion)).split('.');
//			use this block for debugging if this starts crashing again
//			jobject obj = env->CallStaticObjectMethod(classNativeInterface, methodGetOsVersion);
//			debug_log(hsprintf("getting os version - 2: %p", obj));
//			jstring jstr = (jstring) obj;
//			hstr str = _JSTR_TO_HSTR(jstr);
//			debug_log(hsprintf("getting os version - 3: %s", str.cStr()));
//			harray<hstr> osVersions = str.split('.');
	
			hstr majorVersion = osVersions.removeFirst();
			hstr minorVersion = osVersions.joined("");
			osVersions.clear();
			osVersions += majorVersion;
			osVersions += minorVersion;
			info.osVersion = (float)osVersions.joined('.');
#endif
			// RAM size
#if TARGET_IPHONE_SIMULATOR
			info.ram = 1024;
#elif defined(__APPLE__) && defined(_PC_INPUT) // mac
			int mib [] = {CTL_HW, HW_MEMSIZE};
			int64_t value = 0;
			size_t length = sizeof(value);
			if (sysctl(mib, 2, &value, &length, NULL, 0) != -1)
			{
				info.ram = value / (1024 * 1024);
			}
			else
			{
				info.ram = 2048;
			}
#else
			int pageSize;
			int pageCount;
			kdQueryAttribi(KD_ATTRIB_PAGESIZE, (KDint*)&pageSize);
			kdQueryAttribi(KD_ATTRIB_NUMPAGES, (KDint*)&pageCount);
#ifdef _WINRT
#ifndef _WINP8
			info.ram = (int)((int64)pageSize * pageCount / (1024 * 1024)); // in MB
#else
			// WinP8 reports 75% of the RAM's size as available
			info.ram = (int)(Windows::System::MemoryManager::AppMemoryUsageLimit / (1024 * 1024) * 4 / 3); // in MB
#endif
#else
			info.ram = (int)((int64_t)pageSize * pageCount / (1024 * 1024)); // in MB
#endif
#endif
			// other
			debug_log("getting locale");
			info.locale = "en"; // default is "en"
			hstr fullLocale = hstr(kdGetLocale());
			if (fullLocale.contains("-"))
			{
				fullLocale.split("-", info.locale, info.localeVariant);
			}
			else if (fullLocale.contains("_"))
			{
				fullLocale.split("_", info.locale, info.localeVariant);
			}
			else
			{
				info.locale = fullLocale;
			}
			info.locale = info.locale.lowered();
			info.localeVariant = info.localeVariant.uppered();
		}
		return info;
	}

	hstr getPackageName()
	{
#ifndef _WINRT
		return hstr(kdGetenv("KD_APP_ID"));
#else
		return _HL_PSTR_TO_HSTR(Windows::ApplicationModel::Package::Current->Id->FamilyName);
#endif
	}

	hstr getUserDataPath()
	{
		return "data";
	}
	
	int64_t getRamConsumption()
	{
		// TODOa
		return 0LL;
	}	
	
	void messageBox_platform(chstr title, chstr text, MessageBoxButton buttonMask, MessageBoxStyle style,
		hmap<MessageBoxButton, hstr> customButtonTitles, void(*callback)(MessageBoxButton))
	{
		hstr ok;
		hstr yes;
		hstr no;
		hstr cancel;
		_makeButtonLabels(&ok, &yes, &no, &cancel, buttonMask, customButtonTitles);
		const char* buttons[4] = {"", NULL, NULL, NULL};
		MessageBoxButton resultButtons[4] = {(MessageBoxButton)NULL, (MessageBoxButton)NULL, (MessageBoxButton)NULL, (MessageBoxButton)NULL};
		int indexCancel = -1;
		if ((buttonMask & MESSAGE_BUTTON_OK) && (buttonMask & MESSAGE_BUTTON_CANCEL))
		{
			// order is reversed because libKD prefers the colored button to be at place [1], at least on iOS
			// if this is going to be changed for a new platform, ifdef the button order for iOS
			buttons[1] = ok.cStr();
			buttons[0] = cancel.cStr();
			resultButtons[1] = MESSAGE_BUTTON_OK;
			resultButtons[0] = MESSAGE_BUTTON_CANCEL;
			indexCancel = 0;
		}
		else if ((buttonMask & MESSAGE_BUTTON_YES) && (buttonMask & MESSAGE_BUTTON_NO) && (buttonMask & MESSAGE_BUTTON_CANCEL))
		{
			buttons[1] = yes.cStr();
			buttons[0] = no.cStr();
			buttons[2] = cancel.cStr();
			resultButtons[1] = MESSAGE_BUTTON_YES;
			resultButtons[0] = MESSAGE_BUTTON_NO;
			resultButtons[2] = MESSAGE_BUTTON_CANCEL;
			indexCancel = 2;
		}
		else if (buttonMask & MESSAGE_BUTTON_OK)
		{
			buttons[0] = ok.cStr();
			resultButtons[0] = MESSAGE_BUTTON_OK;
			indexCancel = 0;
		}
		else if ((buttonMask & MESSAGE_BUTTON_YES) && (buttonMask & MESSAGE_BUTTON_NO))
		{
			buttons[1] = yes.cStr();
			buttons[0] = no.cStr();
			resultButtons[1] = MESSAGE_BUTTON_YES;
			resultButtons[0] = MESSAGE_BUTTON_NO;
			indexCancel = 1;
		}
		int index = kdShowMessage(title.cStr(), text.cStr(), buttons);
		if (index == -1)
		{
			index = indexCancel;
		}
		if (callback != NULL && index >= 0)
		{
			(*callback)(resultButtons[index]);
		}
	}

}
#endif
