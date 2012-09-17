/// @file
/// @author  Boris Mikic
/// @version 2.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _ANDROID
#include <jni.h>

#include <gtypes/Vector2.h>
#include <hltypes/hmap.h>
#include <hltypes/hstring.h>

#define __NATIVE_INTERFACE_CLASS "net/sourceforge/april/android/NativeInterface"
#include "androidUtilJNI.h"
#include "Platform.h"
#include "RenderSystem.h"

namespace april
{
	void* javaVM = NULL;
	void (*dialogCallback)(MessageBoxButton) = NULL;

	JNIEnv* getJNIEnv()
	{
		JNIEnv* env;
		return (((JavaVM*)april::javaVM)->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) == JNI_OK ? env : NULL);
	}

	jobject getActivity()
	{
		APRIL_GET_NATIVE_INTERFACE_CLASS(classNativeInterface);
		jfieldID fieldActivity = env->GetStaticFieldID(classNativeInterface, "Activity", _JCLASS("net/sourceforge/april/android/Activity"));
		return env->GetStaticObjectField(classNativeInterface, fieldActivity);
	}

	SystemInfo getSystemInfo()
	{
		static SystemInfo info;
		if (info.locale == "")
		{
			APRIL_GET_NATIVE_INTERFACE_CLASS(classNativeInterface);
			// CPU cores
			jmethodID methodGetCpuCores = env->GetStaticMethodID(classNativeInterface, "getCpuCores", _JARGS(_JINT, ));
			info.cpuCores = (int)env->CallStaticObjectMethod(classNativeInterface, methodGetCpuCores);
			// RAM
			jmethodID methodGetDeviceRam = env->GetStaticMethodID(classNativeInterface, "getDeviceRam", _JARGS(_JINT, ));
			info.ram = (int)env->CallStaticObjectMethod(classNativeInterface, methodGetDeviceRam);
			// display resolution
			jmethodID methodGetDisplayResolution = env->GetStaticMethodID(classNativeInterface, "getDisplayResolution", _JARGS(_JOBJ, ));
			jintArray jResolution = (jintArray)env->CallStaticObjectMethod(classNativeInterface, methodGetDisplayResolution);
			jint dimensions[2];
			env->GetIntArrayRegion(jResolution, 0, 2, dimensions);
			info.displayResolution.set((float)(int)dimensions[0], (float)(int)dimensions[1]);
			// display DPI
			jmethodID methodGetDisplayDpi = env->GetStaticMethodID(classNativeInterface, "getDisplayDpi", _JARGS(_JINT, ));
			info.displayDpi = (int)env->CallStaticObjectMethod(classNativeInterface, methodGetDisplayDpi);
			// locale
			jmethodID methodGetLocale = env->GetStaticMethodID(classNativeInterface, "getLocale", _JARGS(_JSTR, ));
			info.locale = _JSTR_TO_HSTR((jstring)env->CallStaticObjectMethod(classNativeInterface, methodGetLocale));
		}
		if (info.maxTextureSize == 0 && april::rendersys != NULL)
		{
			info.maxTextureSize = april::rendersys->_getMaxTextureSize();
		}
		return info;
	}

	DeviceType getDeviceType()
	{
		// TODO
		return DEVICE_ANDROID_PHONE;
	}

	MessageBoxButton messageBox_platform(chstr title, chstr text, MessageBoxButton buttonMask, MessageBoxStyle style,
		hmap<MessageBoxButton, hstr> customButtonTitles, void(*callback)(MessageBoxButton))
	{
		APRIL_GET_NATIVE_INTERFACE_METHOD(classNativeInterface, methodShowMessageBox, "showMessageBox", _JARGS(_JVOID, _JSTR _JSTR _JSTR _JSTR _JSTR _JSTR _JINT));
		// determine ok/yes/no/cancel texts
		hstr ok;
		hstr yes;
		hstr no;
		hstr cancel;
		if ((buttonMask & AMSGBTN_OK) && (buttonMask & AMSGBTN_CANCEL))
		{
			ok = customButtonTitles.try_get_by_key(AMSGBTN_OK, "OK");
			cancel = customButtonTitles.try_get_by_key(AMSGBTN_CANCEL, "Cancel");
		}
		else if ((buttonMask & AMSGBTN_YES) && (buttonMask & AMSGBTN_NO && buttonMask & AMSGBTN_CANCEL))
		{
			yes = customButtonTitles.try_get_by_key(AMSGBTN_YES, "Yes");
			no = customButtonTitles.try_get_by_key(AMSGBTN_NO, "No");
			cancel = customButtonTitles.try_get_by_key(AMSGBTN_CANCEL, "Cancel");
		}
		else if (buttonMask & AMSGBTN_OK)
		{
			ok = customButtonTitles.try_get_by_key(AMSGBTN_OK, "OK");
		}
		else if ((buttonMask & AMSGBTN_YES) && (buttonMask & AMSGBTN_NO))
		{
			yes = customButtonTitles.try_get_by_key(AMSGBTN_YES, "Yes");
			no = customButtonTitles.try_get_by_key(AMSGBTN_NO, "No");
		}
		// create Java strings from hstr
		jstring jTitle = (title != "" ? env->NewStringUTF(title.c_str()) : NULL);
		jstring jText = (text != "" ? env->NewStringUTF(text.c_str()) : NULL);
		jstring jOk = (ok != "" ? env->NewStringUTF(ok.c_str()) : NULL);
		jstring jYes = (yes != "" ? env->NewStringUTF(yes.c_str()) : NULL);
		jstring jNo = (no != "" ? env->NewStringUTF(no.c_str()) : NULL);
		jstring jCancel = (cancel != "" ? env->NewStringUTF(cancel.c_str()) : NULL);
		jint jIconId = 0;
		if ((style & AMSGSTYLE_INFORMATION) || (style & AMSGSTYLE_QUESTION))
		{
			jIconId = 1;
		}
		else if ((style & AMSGSTYLE_WARNING) || (style & AMSGSTYLE_CRITICAL))
		{
			jIconId = 2;
		}
		april::dialogCallback = callback;
		// call Java AprilJNI
		env->CallStaticVoidMethod(classNativeInterface, methodShowMessageBox, jTitle, jText, jOk, jYes, jNo, jCancel, jIconId);
		return AMSGBTN_OK;
	}

}
#endif
