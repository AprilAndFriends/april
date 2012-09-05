/// @file
/// @author  Boris Mikic
/// @version 2.14
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _ANDROID
#include <jni.h>

#include <gtypes/Vector2.h>

#include "Platform.h"
#include "RenderSystem.h"

// TODO - move this into a common Android platform header
#define _JARGS(returnType, arguments) "(" arguments ")" returnType
#define _JARR(str) "[" str
#define _JOBJ "Ljava/lang/Object;"
#define _JSTR "Ljava/lang/String;"
#define _JINT "I"
#define _JBOOL "Z"
#define _JFLOAT "F"
#define _JVOID "V"

namespace april
{
	JNIEnv* getJNIEnv();
	jobject getActivity();
	extern void (*dialogCallback)(MessageBoxButton);

	gvec2 getDisplayResolution()
	{
		JNIEnv* env = getJNIEnv();
		jclass classNativeInterface = env->FindClass("net/sourceforge/april/android/NativeInterface");
		jmethodID methodGetDisplayResolution = env->GetStaticMethodID(classNativeInterface, "getDisplayResolution", _JARGS(_JOBJ, ));
		jintArray jResolution = (jintArray)env->CallStaticObjectMethod(classNativeInterface, methodGetDisplayResolution);
		jint dimensions[2];
		env->GetIntArrayRegion(jResolution, 0, 2, dimensions);
		return gvec2((float)(int)dimensions[0], (float)(int)dimensions[1]);
	}

	SystemInfo getSystemInfo()
	{
		// TODO
		static SystemInfo info;
		if (info.locale == "")
		{
			info.cpu_cores = 1; // TODO
			info.ram = 256;
			info.max_texture_size = 0;
			info.locale = "en";
		}
		// TODO
		if (info.max_texture_size == 0 && april::rendersys != NULL)
		{
			info.max_texture_size = april::rendersys->_getMaxTextureSize();
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
		// Java Environment
		JNIEnv* env = getJNIEnv();
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
		jclass nativeInterface = env->FindClass("net/sourceforge/april/android/NativeInterface");
		jmethodID methodShowMessageBox = env->GetStaticMethodID(nativeInterface, "showMessageBox", _JARGS(_JVOID, _JSTR _JSTR _JSTR _JSTR _JSTR _JSTR _JINT));
		env->CallStaticVoidMethod(nativeInterface, methodShowMessageBox, jTitle, jText, jOk, jYes, jNo, jCancel, jIconId);
		return AMSGBTN_OK;
	}

}
#endif
