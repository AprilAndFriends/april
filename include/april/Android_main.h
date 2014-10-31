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

/// @brief This function is called when the .so file is loaded.
/// @param[in] vm A pointer to the Java VM.
/// @param[in] reserved Reserved data.
/// @return The Java/JNI version or -1 on failure.
/// @note Don't worry about this, this is how Android/Java/JNI has to initialize things.
extern "C" JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved)
{
	return april::__JNI_OnLoad(&april_init, &april_destroy, vm, reserved);
}
#endif
#endif
