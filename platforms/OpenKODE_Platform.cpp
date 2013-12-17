/// @file
/// @author  Boris Mikic
/// @version 3.14
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _OPENKODE
#include <KD/kd.h>

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
	#if TARGET_OS_IPHONE
		void getStaticiOSInfo(chstr name, april::SystemInfo& info);
	#elif TARGET_OS_MAC
		#include <sys/sysctl.h>
	#endif
#endif

#if defined(_ANDROID)
#include <jni.h>
#define __NATIVE_INTERFACE_CLASS "net/sourceforge/april/android/NativeInterface"
#include "androidUtilJNI.h"
#endif

namespace april
{
	static SystemInfo info;
	SystemInfo getSystemInfo()
	{
		if (info.locale == "")
		{
			// number of CPU cores
			info.cpuCores = 1;
			// display resolution
			int width = 0;
			int height = 0;
			kdQueryAttribi(KD_ATTRIB_WIDTH, (KDint*)&width);
			kdQueryAttribi(KD_ATTRIB_HEIGHT, (KDint*)&height);
			info.displayResolution.set((float)hmax(width, height), (float)hmin(width, height));
			// display DPI
			kdQueryAttribi(KD_ATTRIB_DPI, (KDint*)&info.displayDpi);
#if TARGET_OS_IPHONE // On iOS, april prefers to use hardcoded device info than OpenKODE's info, it's more accurate
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
#elif defined(_ANDROID)
			APRIL_GET_NATIVE_INTERFACE_CLASS(classNativeInterface);
			// CPU cores
			jmethodID methodGetCpuCores = env->GetStaticMethodID(classNativeInterface, "getCpuCores", _JARGS(_JINT, ));
			info.cpuCores = (int)env->CallStaticIntMethod(classNativeInterface, methodGetCpuCores);
#endif
			// RAM size
#if TARGET_IPHONE_SIMULATOR
			info.ram = 1024;
#elif defined(__APPLE__) && defined(_PC_INPUT) // mac
			int mib [] = { CTL_HW, HW_MEMSIZE };
			int64_t value = 0;
			size_t length = sizeof(value);
			
			if (sysctl(mib, 2, &value, &length, NULL, 0) == -1)
			{
				info.ram = 2048;
			}
			else
			{
				info.ram = value / (1024 * 1024);
			}
#else
			int pageSize;
			int pageCount;
			kdQueryAttribi(KD_ATTRIB_PAGESIZE, (KDint*)&pageSize);
			kdQueryAttribi(KD_ATTRIB_NUMPAGES, (KDint*)&pageCount);
			info.ram = hmax((int)((int64)pageSize * pageCount / 1048576), 1024); // in MB, min being 1 GB
#endif
			// other
			info.locale = hstr(kdGetLocale());
			if (info.locale == "")
			{
				info.locale = "en"; // default is "en"
			}
			else if (info.locale == "pt_PT")
			{
				info.locale = "pt-PT";
			}
			else if (info.locale.utf8_size() > 2 && info.locale != "pt-PT")
			{
				info.locale = info.locale.utf8_substr(0, 2);
			}
		}
		if (info.maxTextureSize == 0 && april::rendersys != NULL)
		{
			info.maxTextureSize = april::rendersys->getMaxTextureSize();
		}
		return info;
	}

	DeviceType getDeviceType()
	{
		return DEVICE_OPENKODE;
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
	
	void messageBox_platform(chstr title, chstr text, MessageBoxButton buttonMask, MessageBoxStyle style,
		hmap<MessageBoxButton, hstr> customButtonTitles, void(*callback)(MessageBoxButton))
	{
		hstr ok;
		hstr yes;
		hstr no;
		hstr cancel;
		_makeButtonLabels(&ok, &yes, &no, &cancel, buttonMask, customButtonTitles);
		const char* buttons[4] = {"", NULL, NULL, NULL};
		MessageBoxButton resultButtons[4] = {AMSGBTN_NULL, AMSGBTN_NULL, AMSGBTN_NULL, AMSGBTN_NULL};
		int indexCancel = -1;
		if ((buttonMask & AMSGBTN_OK) && (buttonMask & AMSGBTN_CANCEL))
		{
			// order is reversed because libKD prefers the colored button to be at place [1], at least on iOS
			// if this is going to be changed for a new platform, ifdef the button order for iOS
			buttons[1] = ok.c_str();
			buttons[0] = cancel.c_str();
			resultButtons[1] = AMSGBTN_OK;
			resultButtons[0] = AMSGBTN_CANCEL;
			indexCancel = 0;
		}
		else if ((buttonMask & AMSGBTN_YES) && (buttonMask & AMSGBTN_NO) && (buttonMask & AMSGBTN_CANCEL))
		{
			buttons[1] = yes.c_str();
			buttons[0] = no.c_str();
			buttons[2] = cancel.c_str();
			resultButtons[1] = AMSGBTN_YES;
			resultButtons[0] = AMSGBTN_NO;
			resultButtons[2] = AMSGBTN_CANCEL;
			indexCancel = 2;
		}
		else if (buttonMask & AMSGBTN_OK)
		{
			buttons[0] = ok.c_str();
			resultButtons[0] = AMSGBTN_OK;
			indexCancel = 0;
		}
		else if ((buttonMask & AMSGBTN_YES) && (buttonMask & AMSGBTN_NO))
		{
			buttons[1] = yes.c_str();
			buttons[0] = no.c_str();
			resultButtons[1] = AMSGBTN_YES;
			resultButtons[0] = AMSGBTN_NO;
			indexCancel = 1;
		}
		int index = kdShowMessage(title.c_str(), text.c_str(), buttons);
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
