/// @file
/// @author  Ivan Vucica
/// @author  Boris Mikic
/// @version 3.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/// 
/// @section DESCRIPTION
/// 
/// Defines main functions.

#ifndef APRIL_MAIN_H
#define APRIL_MAIN_H

#include <hltypes/harray.h>
#include <hltypes/hstring.h>

#include "aprilExport.h"

/**
 * @file main.h
 *
 * This file is primarily used to change entry and exit points
 * from main() to april_init() and april_destroy(). These will
 * be called transparently from main() which we will guarantee
 * to define correctly in platform-specific way. Then, we will
 * call your april_init() and april_destroy().
 * 
 * Correct usage of main.h is:
 * - include it in exactly one file in your project
 *   (typically main.cpp); it MUST be included somewhere
 * - define just april_init() and april_destroy()
 * - do not worry about other functionality in this header
 * - when using the OpenKODE standard, kdMain is defined here
 *   and you should include KD/kd.h before this file
 *
 * No other functionality defined in main.h should be seen as
 * being publicly available, and other functions (such as
 * april_main()) are actually for internal use only.
 **/

#ifndef BUILDING_APRIL
#include <hltypes/hplatform.h>
#ifdef __APPLE__
#include <TargetConditionals.h>
#endif
#ifdef _ANDROID
#include <jni.h>
#include <string.h>
#endif
#endif

#ifndef _ANDROID
aprilExport int april_main(void (*anAprilInit)(const harray<hstr>&), void (*anAprilDestroy)(), const harray<hstr>& args, int argc, char** argv);
#else
namespace april
{
	aprilExport jint JNI_OnLoad(JavaVM* vm, void* reserved);
}
#endif

extern void april_init(const harray<hstr>& args);
extern void april_destroy();

#ifndef BUILDING_APRIL
#if defined(_WIN32) && !_HL_WINRT && defined(__APRIL_SINGLE_INSTANCE_NAME)
#define __SINGLE_INSTANCE
#include <april/Platform.h>
static HANDLE lockMutex;
bool __lockSingleInstanceMutex(hstr instanceName, chstr fallbackName)
{
	if (instanceName == "")
	{
		instanceName = fallbackName;
	}
	instanceName = instanceName.replace("\\", "/");
	lockMutex = CreateMutexW(NULL, true, instanceName.w_str().c_str());
	if (lockMutex != 0 && GetLastError() == ERROR_ALREADY_EXISTS)
	{
		CloseHandle(lockMutex);
		april::messageBox("Warning", "Cannot launch " + instanceName + ", already running!", april::AMSGBTN_OK, april::AMSGSTYLE_WARNING);
		return false;
	}
	return true;
}

void __unlockSingleInstanceMutex()
{
	if (lockMutex != 0)
	{
		CloseHandle(lockMutex);
	}
}
#endif
#ifdef _ANDROID
extern "C" JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved)
{
	return april::JNI_OnLoad(vm, reserved);
}
#elif defined(_OPENKODE)
KDint KD_APIENTRY kdMain(KDint argc, const KDchar* const* argv)
{
	harray<hstr> args;
	if (argv != NULL && argv[0] != NULL)
	{
		for_iter (i, 0, argc)
		{
			args += argv[i];
		}
	}
	april_main(april_init, april_destroy, args, (int)argc, (char**)argv);
	return 0;
}
#elif !defined(_WIN32) || defined(_CONSOLE) && !_HL_WINRT
int main(int argc, char** argv)
{
#ifdef __SINGLE_INSTANCE
	if (!__lockSingleInstanceMutex(hstr::from_unicode(__APRIL_SINGLE_INSTANCE_NAME), argv[0]))
	{
		return 0;
	}
#endif
	harray<hstr> args;
	if (argv != NULL)
	{
		for_iter (i, 0, argc)
		{
			args += argv[i];
		}
	}
	int result = april_main(april_init, april_destroy, args, argc, argv);
#ifdef __SINGLE_INSTANCE
	__unlockSingleInstanceMutex();
#endif
	return result;
}
#else
#if !_HL_WINRT
#include <stdio.h>
#include <shellapi.h>
int CALLBACK wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, wchar_t* wCmdLine, int nCmdShow)
#else
[Platform::MTAThread]
int main(Platform::Array<Platform::String^>^ args)
#endif
{
#if _HL_WINRT && defined(__SINGLE_INSTANCE)
	if (!__lockSingleInstanceMutex(hstr::from_unicode(__APRIL_SINGLE_INSTANCE_NAME), hstr::from_unicode(args[0]->Data())))
	{
		return 0;
	}
#endif
	// extract arguments
	int argc = 0;
#if !_HL_WINRT
	wchar_t** wArgv = CommandLineToArgvW(wCmdLine, &argc);
#ifdef __SINGLE_INSTANCE
	if (!__lockSingleInstanceMutex(hstr::from_unicode(__APRIL_SINGLE_INSTANCE_NAME), hstr::from_unicode(wArgv[0])))
	{
		LocalFree(wArgv);
		return 0;
	}
#endif
#endif
	harray<hstr> args;
	for_iter (i, 0, argc)
	{
#if !_HL_WINRT
		args += hstr::from_unicode(wArgv[i]);
#else
		args += hstr::from_unicode(args[i]->Data());
#endif
	}
#if !_HL_WINRT
	LocalFree(wArgv);
#endif
	// call the user specified main function
	april_main(april_init, april_destroy, args, 0, NULL);
#ifdef __SINGLE_INSTANCE
	__unlockSingleInstanceMutex();
#endif
	return 0;
}
#endif
#define main __ STOP_USING_MAIN___DEPRECATED_IN_APRIL
#endif

#define APRIL_NO_MAIN 1

#endif
