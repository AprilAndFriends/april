/// @file
/// @version 5.1
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

#ifdef __ANDROID__
#include <jni.h>
#include <string.h>
#endif

#include <hltypes/harray.h>
#include <hltypes/hstring.h>

#include "aprilExport.h"

/// @brief Different platforms differently handle string transformations depending on locale. Which locale needs to be used for proper handling also depends on the platform.
#ifndef __APRIL_DEFAULT_LOCALE
#ifdef _WIN32
#define __APRIL_DEFAULT_LOCALE "en"
#else
#define __APRIL_DEFAULT_LOCALE "en_US.UTF-8"
#endif
#endif

/// @brief Define __APRIL_SINGLE_INSTANCE_NAME if you want an automated single-instance system.
/// @note Only pure Win32 supports and/or needs this.
#ifdef __APRIL_SINGLE_INSTANCE_NAME
#define __SINGLE_INSTANCE
#endif

/// @brief This function is called after the platform application has initialized. The developer implements this.
extern void __aprilApplicationInit();
/// @brief This function is called before the platform application exits. The developer implements this.
extern void __aprilApplicationDestroy();

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
#ifdef __ANDROID__
	/// @brief This function is called when the .so file is loaded.
	/// @param[in] aprilApplicationInit A pointer to __aprilApplicationInit().
	/// @param[in] aprilApplicationDestroy A pointer to __aprilApplicationDestroy().
	/// @param[in] vm A pointer to the Java VM.
	/// @param[in] reserved Reserved data.
	/// @note This is used internally only.
	aprilFnExport jint __JNI_OnLoad(void (*aprilApplicationInit)(), void (*aprilApplicationDestroy)(), JavaVM* vm, void* reserved);
#else
	/// @brief A special main loop function that serves as general entry point.
	/// @param[in] aprilApplicationInit A pointer to __aprilApplicationInit().
	/// @param[in] aprilApplicationDestroy A pointer to __aprilApplicationDestroy().
	/// @param[in] argc Number of arguments the application was run with.
	/// @param[in] argv The arguments the application was run with.
	/// @note This is used internally only.
	aprilExport int __mainStandard(void (*aprilApplicationInit)(), void (*aprilApplicationDestroy)(), int argc, char** argv);
#endif
}

#endif
