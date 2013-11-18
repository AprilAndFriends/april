/// @file
/// @author  Boris Mikic
/// @version 3.2
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/// 
/// @section DESCRIPTION
/// 
/// Defines main for Android.

#ifdef _ANDROID
#ifndef APRIL_ANDROID_MAIN_H
#define APRIL_ANDROID_MAIN_H

#include <jni.h>
#include <string.h>

extern "C" JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved)
{
	return april::JNI_OnLoad(vm, reserved);
}
#endif
#endif
