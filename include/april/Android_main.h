/// @file
/// @version 3.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Defines main for Android.

#ifdef _ANDROID
#ifndef APRIL_ANDROID_MAIN_H
#define APRIL_ANDROID_MAIN_H

#include <jni.h>
#include <string.h>

#include "aprilExport.h"

extern "C" JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved)
{
	return april::JNI_OnLoad(&april_init, &april_destroy, vm, reserved);
}
#endif
#endif
