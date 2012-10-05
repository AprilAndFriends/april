/// @file
/// @author  Ivan Vucica
/// @author  Boris Mikic
/// @version 2.0
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
 * - if you are using Android with JNI, make sure to define
 *   APRIL_ANDROID_ACTIVITY_NAME as a fully-qualified
 *   name of the launch activity of implement JNI_OnLoad
 *   yourself
 *
 * No other functionality defined in main.h should be seen as
 * being publicly available, and other functions (such as
 * april_main()) are actually for internal use only.
 **/

#ifndef BUILDING_APRIL
#ifdef __APPLE__
#include <TargetConditionals.h>
#endif
#ifdef _ANDROID
#include <jni.h>
#include <string.h>
#endif
#endif

#ifndef _ANDROID
aprilExport int april_main(void (*anAprilInit)(const harray<hstr>&), void (*anAprilDestroy)(), int argc, char** argv);
#else
namespace april
{
	aprilExport jint JNI_OnLoad(JavaVM* vm, void* reserved);
}
#endif

extern void april_init(const harray<hstr>& args);
extern void april_destroy();

#ifndef BUILDING_APRIL
//{
#ifdef _ANDROID
extern "C" JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved)
{
	return april::JNI_OnLoad(vm, reserved);
}
#elif !defined(_WIN32) || defined(_CONSOLE)
int main(int argc, char** argv)
{
#if TARGET_IPHONE_SIMULATOR
	/* trick for running valgrind in iphone simulator
	 * http://landonf.bikemonkey.org/code/iphone/iPhone_Simulator_Valgrind.20081224.html
	 * original code requires that code is not executed with -valgrind.
	 * since we're a lib, we'll include the reexec code,
	 * allow redefining of valgrind path and force using -valgrind
	 * to specify relaunching requirement. 
	 * we'll also include this only on iPhone Simulator code,
	 * instead of requiring manually defining that we want this code.
	 */
#ifndef VALGRIND
#define VALGRIND "/usr/local/bin/valgrind"
#endif
	/* Using the valgrind build config, reexec this program
	 * in valgrind */
	if (argc >= 2 && strcmp(argv[1], "-valgrind") == 0)
	{
		printf("Relaunching with valgrind\n");
		execl(VALGRIND, VALGRIND, "--leak-check=full", "--error-limit=no", argv[0], NULL);
	}
#endif
	
	return april_main(april_init, april_destroy, argc, argv);
}
#else
#include <windows.h>
#include <stdio.h>
#include <shellapi.h>
int CALLBACK wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, wchar_t* wCmdLine, int nCmdShow)
{
#if !_HLWINRT
	// extract arguments
	int argc = 0;
	wchar_t** wArgv = CommandLineToArgvW(wCmdLine, &argc);
	char** argv = new char*[argc];
	hstr arg;
	for_iter (i, 0, argc)
	{
		arg = unicode_to_utf8(wArgv[i]);
		argv[i] = new char[arg.size() + 1];
		memset(argv[i], 0, arg.size() + 1);
		memcpy(argv[i], arg.c_str(), sizeof(char) * arg.size());
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
#else
	char* test = ".\\win8app";
	april_main(april_init, april_destroy, 1, &test);
#endif
	return 0;
}
#endif
#define main __ STOP_USING_MAIN___DEPRECATED_IN_APRIL
//}
#endif

#define APRIL_NO_MAIN 1

#endif
