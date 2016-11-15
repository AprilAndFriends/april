/// @file
/// @version 4.2
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifdef _ANDROID
#include <jni.h>

#include <gtypes/Vector2.h>
#include <hltypes/hmap.h>
#include <hltypes/hstring.h>

#define __NATIVE_INTERFACE_CLASS "com/april/NativeInterface"
#include "androidUtilJNI.h"
#include "Platform.h"
#include "RenderSystem.h"

namespace april
{
	void* javaVM = NULL;
	void (*dialogCallback)(MessageBoxButton) = NULL; // defined here to avoid making a bunch of _OPENKODE #ifdefs in Android_Platform.cpp
	jobject classLoader = NULL;

	hstr _jstringToHstr(JNIEnv* env, jstring string)
	{
		const char* chars = env->GetStringUTFChars(string, NULL);
		hstr result(chars);
		env->ReleaseStringUTFChars(string, chars);
		return result;
	}
	
	JNIEnv* getJNIEnv()
	{
		JNIEnv* env = NULL;
		if (((JavaVM*)april::javaVM)->AttachCurrentThread(&env, NULL) == JNI_OK)
		{
			env->PushLocalFrame(APRIL_JNI_DEFAULT_LOCAL_FRAME_SIZE);
		}
		return env;
	}
	
	jobject getActivity()
	{
		jobject result = NULL;
		APRIL_GET_NATIVE_INTERFACE_CLASS(classNativeInterface);
		jfieldID fieldActivity = env->GetStaticFieldID(classNativeInterface, "activity", _JCLASS("android/app/Activity"));
		result = env->GetStaticObjectField(classNativeInterface, fieldActivity);
		env->PopLocalFrame(NULL);
		return result;
	}
	
	jobject getAprilActivity()
	{
		jobject result = NULL;
		APRIL_GET_NATIVE_INTERFACE_CLASS(classNativeInterface);
		jfieldID fieldAprilActivity = env->GetStaticFieldID(classNativeInterface, "aprilActivity", _JCLASS("com/april/Activity"));
		result = env->GetStaticObjectField(classNativeInterface, fieldAprilActivity);
		env->PopLocalFrame(NULL);
		return result;
	}

	jclass findJNIClass(JNIEnv* env, chstr classPath)
	{
		if (april::classLoader == NULL)
		{
			return env->FindClass(classPath.cStr());
		}
		jclass result = NULL;
		env->PushLocalFrame(APRIL_JNI_DEFAULT_LOCAL_FRAME_SIZE);
		jclass classClassLoader = env->GetObjectClass(april::classLoader);
		jmethodID methodLoadClass = env->GetMethodID(classClassLoader, "loadClass", _JARGS(_JCLASS("java/lang/Class"), _JSTR _JBOOL));
		jstring jClassPath = env->NewStringUTF(classPath.cStr());
		jboolean jInitialize = JNI_TRUE;
		result = (jclass)env->CallObjectMethod(april::classLoader, methodLoadClass, jClassPath, jInitialize);
		env->PopLocalFrame(NULL);
		return result;
	}

}
#endif
