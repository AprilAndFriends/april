/// @file
/// @author  Boris Mikic
/// @version 3.1
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
	void (*dialogCallback)(MessageBoxButton) = NULL; // defined here to avoid making a bunch of _OPENKODE #ifdefs in Android_Platform.cpp
	
	JNIEnv* getJNIEnv()
	{
		JNIEnv* env;
		return (((JavaVM*)april::javaVM)->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) == JNI_OK ? env : NULL);
	}
	
	jobject getActivity()
	{
		APRIL_GET_NATIVE_INTERFACE_CLASS(classNativeInterface);
		jfieldID fieldActivity = env->GetStaticFieldID(classNativeInterface, "Activity", _JCLASS("android/app/Activity"));
		return env->GetStaticObjectField(classNativeInterface, fieldActivity);
	}
	
	jobject getAprilActivity()
	{
		APRIL_GET_NATIVE_INTERFACE_CLASS(classNativeInterface);
		jfieldID fieldAprilActivity = env->GetStaticFieldID(classNativeInterface, "AprilActivity", _JCLASS("net/sourceforge/april/android/Activity"));
		return env->GetStaticObjectField(classNativeInterface, fieldAprilActivity);
	}
	
}
#endif
