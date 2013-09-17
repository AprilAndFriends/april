/// @file
/// @author  Ivan Vucica
/// @author  Boris Mikic
/// @version 3.1
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
#endif
#ifdef _ANDROID
#include <jni.h>
#include <string.h>
#endif

#ifdef _ANDROID
namespace april
{
	aprilExport jint JNI_OnLoad(JavaVM* vm, void* reserved);
}
#endif
#if !defined(_ANDROID) || defined(_OPENKODE)
aprilExport int april_main(void (*anAprilInit)(const harray<hstr>&), void (*anAprilDestroy)(), int argc, char** argv);
#endif

extern void april_init(const harray<hstr>& args);
extern void april_destroy();

#ifndef BUILDING_APRIL
#if defined(_WIN32) && !defined(_WINRT) && defined(__APRIL_SINGLE_INSTANCE_NAME)
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
#endif
#ifdef _OPENKODE
KDint KD_APIENTRY kdMain(KDint argc, const KDchar* const* argv)
{
	april_main(april_init, april_destroy, (int)argc, (char**)argv);
	return 0;
}
#elif !defined(_ANDROID)
#if !defined(_WIN32) || defined(_CONSOLE) && !defined(_WINRT)
int main(int argc, char** argv)
{
#ifdef __SINGLE_INSTANCE
	if (!__lockSingleInstanceMutex(hstr::from_unicode(__APRIL_SINGLE_INSTANCE_NAME), argv[0]))
	{
		return 0;
	}
#endif
	int result = april_main(april_init, april_destroy, argc, argv);
#ifdef __SINGLE_INSTANCE
	__unlockSingleInstanceMutex();
#endif
	return result;
}
#else
#ifndef _WINRT
#include <stdio.h>
#include <shellapi.h>
int CALLBACK wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, wchar_t* wCmdLine, int nCmdShow)
{
	// extract arguments
	int argc = 0;
	wchar_t** wArgv = CommandLineToArgvW(wCmdLine, &argc);
#ifdef __SINGLE_INSTANCE
	if (!__lockSingleInstanceMutex(hstr::from_unicode(__APRIL_SINGLE_INSTANCE_NAME), hstr::from_unicode(wArgv[0])))
	{
		LocalFree(wArgv);
		return 0;
	}
#endif
	char** argv = new char*[argc];
	hstr arg;
	for_iter (i, 0, argc)
	{
		arg = hstr::from_unicode(wArgv[i]);
		argv[i] = new char[arg.size() + 1];
		memcpy(argv[i], arg.c_str(), sizeof(char) * (arg.size() + 1));
	}
	LocalFree(wArgv);
	// call the user specified main function
	april_main(april_init, april_destroy, argc, argv);
	// free allocated memory for arguments
	for_iter (i, 0, argc)
	{
		delete [] argv[i];
	}
	delete [] argv;
#ifdef __SINGLE_INSTANCE
	__unlockSingleInstanceMutex();
#endif
	return 0;
}
#else
[Platform::MTAThread]
int main(Platform::Array<Platform::String^>^ args)
{
#ifdef __SINGLE_INSTANCE
	if (!__lockSingleInstanceMutex(hstr::from_unicode(__APRIL_SINGLE_INSTANCE_NAME), hstr::from_unicode(args[0]->Data())))
	{
		return 0;
	}
#endif
	// extract arguments
	int argc = 0;
	char** argv = new char*[argc];
	hstr arg;
	for_iter (i, 0, argc)
	{
		arg = hstr::from_unicode(args[i]->Data());
		argv[i] = new char[arg.size() + 1];
		memcpy(argv[i], arg.c_str(), sizeof(char) * (arg.size() + 1));
	}
	// call the user specified main function
	april_main(april_init, april_destroy, argc, argv);
	// free allocated memory for arguments
	for_iter (i, 0, argc)
	{
		delete [] argv[i];
	}
	delete [] argv;
#ifdef __SINGLE_INSTANCE
	__unlockSingleInstanceMutex();
#endif
	return 0;
}
#endif
#endif
#endif
#define main __ STOP_USING_MAIN___DEPRECATED_IN_APRIL
#endif

#define APRIL_NO_MAIN 1

#endif
