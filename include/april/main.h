/************************************************************************************\
 This source file is part of the Awesome Portable Rendering Interface Library         *
 For latest info, see http://libapril.sourceforge.net/                                *
 **************************************************************************************
 Copyright (c) 2010 Ivan Vucica                                                       *
 *                                                                                    *
 * This program is free software; you can redistribute it and/or modify it under      *
 * the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
 \************************************************************************************/
#ifndef APRIL_MAIN_H
#define APRIL_MAIN_H

#include <hltypes/hstring.h>
#include <hltypes/harray.h>

#include "aprilExport.h"

/**
 * \file main.h
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
 *
 * No other functionality defined in main.h should be seen as
 * being publicly available, and other functions (such as
 * april_main()) are actually for internal use only.
 **/

#ifndef BUILDING_APRIL
#ifdef __APPLE__
#include <TargetConditionals.h>
#endif
#endif

extern void april_init(const harray<hstr>& argv);
extern void april_destroy();

aprilExport int april_main (void (*anAprilInit)(const harray<hstr>&), void (*anAprilDestroy)(), int argc, char **argv);

#ifndef BUILDING_APRIL
//{
#if !defined(_WIN32) || defined(_CONSOLE) || defined(HAVE_MARMELADE)
int main (int argc, char **argv)
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
	if (argc >= 2 && strcmp(argv[1], "-valgrind") == 0) {
		printf("Relaunching with valgrind\n");
		execl(VALGRIND, VALGRIND, "--leak-check=full", "--error-limit=no", argv[0],
			  NULL);
	}
#endif
	
    april_main(april_init, april_destroy, argc, argv);
    return 0;
}
#else
#include <windows.h>
int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    april_main(april_init, april_destroy, 0, NULL);
    return 0;
}
#endif
#define main __ STOP_USING_MAIN___DEPRECATED_IN_APRIL
//}
#endif

#define APRIL_NO_MAIN 1

#endif
