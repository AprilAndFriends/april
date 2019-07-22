/// @file
/// @version 5.2
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifdef __ANDROID__
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
	void (*dialogCallback)(const MessageBoxButton&) = NULL;
	jobject classLoader = NULL;

	hstr _jstringToHstr(JNIEnv* env, jstring string)
	{
		if (string == NULL)
		{
			return "";
		}
		const char* chars = env->GetStringUTFChars(string, NULL);
		hstr result(chars != NULL ? chars : "");
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
	
	jobject getActivity(JNIEnv* env)
	{
		jclass classNativeInterface = april::findJNIClass(env, __NATIVE_INTERFACE_CLASS);
		if (classNativeInterface == NULL)
		{
			hlog::error(APRIL_JNI_LOG_TAG, "Could not find native interface class: " + hstr(__NATIVE_INTERFACE_CLASS));
		}
		jfieldID fieldActivity = env->GetStaticFieldID(classNativeInterface, "activity", _JCLASS("android/app/Activity"));
		return env->GetStaticObjectField(classNativeInterface, fieldActivity);
	}
	
	jobject getAprilActivity(JNIEnv* env)
	{
		jclass classNativeInterface = april::findJNIClass(env, __NATIVE_INTERFACE_CLASS);
		if (classNativeInterface == NULL)
		{
			hlog::error(APRIL_JNI_LOG_TAG, "Could not find native interface class: " + hstr(__NATIVE_INTERFACE_CLASS));
		}
		jfieldID fieldAprilActivity = env->GetStaticFieldID(classNativeInterface, "aprilActivity", _JCLASS("com/april/Activity"));
		return env->GetStaticObjectField(classNativeInterface, fieldAprilActivity);
	}

	jclass findJNIClass(JNIEnv* env, chstr classPath)
	{
		if (april::classLoader == NULL)
		{
			return env->FindClass(classPath.cStr());
		}
		jclass classClassLoader = env->GetObjectClass(april::classLoader);
		jmethodID methodLoadClass = env->GetMethodID(classClassLoader, "loadClass", _JARGS(_JCLASS("java/lang/Class"), _JSTR _JBOOL));
		jstring jClassPath = env->NewStringUTF(classPath.cStr());
		jboolean jInitialize = JNI_TRUE;
		return (jclass)env->CallObjectMethod(april::classLoader, methodLoadClass, jClassPath, jInitialize);
	}

}
#endif
