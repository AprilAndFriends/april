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
/// Defines basic functions required in main startup procedure.

#ifndef APRIL_MAIN_BASE_H
#define APRIL_MAIN_BASE_H

#ifdef _ANDROID
#include <jni.h>
#include <string.h>
#endif

#include <hltypes/harray.h>
#include <hltypes/hstring.h>

#include "aprilExport.h"

/// @brief Define __APRIL_SINGLE_INSTANCE_NAME if you want an automated single-instance system.
/// @note Only pure Win32 supports and/or needs this.
#ifdef __APRIL_SINGLE_INSTANCE_NAME
#define __SINGLE_INSTANCE
#endif

/// @brief This function is called after the platform application has initialized. The developer implements this.
/// @param[in] args An array of arguments.
extern void april_init(const harray<hstr>& args);
/// @brief This function is called before the platform application exits. The developer implements this.
extern void april_destroy();

namespace april
{
	/// @brief Locks a mutex to prevent multiple instances of the application running.
	/// @param[in] instanceName Unique name of the application.
	/// @param[in] fallbackName If no instance name is specified, tries to use an alternative (e.g. executable name).
	/// @return True if successfully locked. False if another instance is already running.
	/// @note This is used internally only.
	aprilFnExport bool __lockSingleInstanceMutex(hstr instanceName, chstr fallbackName);
	/// @brief Unlocks the single-instance mutex.
	/// @note This is used internally only.
	aprilFnExport void __unlockSingleInstanceMutex();
#ifdef _ANDROID
	/// @brief This function is called when the .so file is loaded.
	/// @param[in] anAprilInit A pointer to april_init().
	/// @param[in] anAprilDestroy A pointer to april_destroy().
	/// @param[in] vm A pointer to the Java VM.
	/// @param[in] reserved Reserved data.
	/// @note This is used internally only.
	aprilFnExport jint __JNI_OnLoad(void(*anAprilInit)(const harray<hstr>&), void(*anAprilDestroy)(), JavaVM* vm, void* reserved);
#endif
}

#if !defined(_ANDROID) || defined(_OPENKODE)
/// @brief A special main loop function that serves as general entry point.
/// @param[in] A special main loop function that serves as general entry point.
/// @note This is used internally only.
aprilExport int __april_main(void(*anAprilInit)(const harray<hstr>&), void(*anAprilDestroy)(), int argc, char** argv);
#endif

#endif
