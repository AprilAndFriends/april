/// @file
/// @author  Boris Mikic
/// @version 1.4
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _ANDROID
#include <string.h>
#include <jni.h>

#include <hltypes/harray.h>
#include <hltypes/hstring.h>

#include "main.h"
#include "RenderSystem.h"
#include "Window.h"

jstring JNICALL __stringFromJNICPP(JNIEnv* env, jobject obj)
{
	return env->NewStringUTF("HAITHAR");
}

jint april_JNI_OnLoad(JavaVM* vm, void* reserved, const char* launchActivityName)
{
    JNIEnv* env;
    if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK)
	{
        return -1;
    }
	jclass classe = env->FindClass(launchActivityName);
	JNINativeMethod method;
	method.name = "stringFromJNICPP";
	method.signature = "()Ljava/lang/String;";
	method.fnPtr = (void*)&__stringFromJNICPP;
	int result = env->RegisterNatives(classe, &method, 1);
	if (result != 0)
	{
		return -1;
	}
    return JNI_VERSION_1_6;
}
#endif
