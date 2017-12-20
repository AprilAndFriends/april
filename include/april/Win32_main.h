/// @file
/// @version 4.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Defines main for Win32 "Windows" system.

#if defined(_WIN32) && !defined(_CONSOLE) && !defined(_WINRT)
#ifndef APRIL_WIN32_MAIN_H
#define APRIL_WIN32_MAIN_H

#include <locale.h>
#include <stdio.h>
#include <shellapi.h>

#define __HL_INCLUDE_PLATFORM_HEADERS
#include <hltypes/harray.h>
#include <hltypes/hplatform.h>
#include <hltypes/hstring.h>

#include "main_base.h"
#include "aprilExport.h"

/// @brief Defines the main function in Win32 when not using a console application.
/// @param[in] hInstance Handle to the instance within Windows.
/// @param[in] hPrevInstance Handle to the instance of the calling application.
/// @param[in] wCmdLine Command line that called this application.
/// @param[in] wCmdShow Whether the console is displayed.
/// @return Application result code.
/// @note This is used internally only.
int CALLBACK wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, wchar_t* wCmdLine, int nCmdShow)
{
	setlocale(LC_ALL, __APRIL_DEFAULT_LOCALE); // make sure the app uses a neutral locale that includes all specifics for all locales
	// extract arguments
	int argc = 0;
	wchar_t** wArgv = CommandLineToArgvW(wCmdLine, &argc);
#ifdef __SINGLE_INSTANCE
	if (!april::__lockSingleInstanceMutex(hstr::fromUnicode(__APRIL_SINGLE_INSTANCE_NAME), hstr::fromUnicode(wArgv[0])))
	{
		LocalFree(wArgv);
		return 0;
	}
#endif
	char** argv = new char*[argc];
	hstr arg;
	for_iter (i, 0, argc)
	{
		arg = hstr::fromUnicode(wArgv[i]);
		argv[i] = new char[arg.size() + 1];
		memcpy(argv[i], arg.cStr(), sizeof(char) * (arg.size() + 1));
	}
	LocalFree(wArgv);
	// call the user specified main function
	int result = april::__mainStandard(&__aprilApplicationInit, &__aprilApplicationDestroy, argc, argv);
	// free allocated memory for arguments
	for_iter (i, 0, argc)
	{
		delete[] argv[i];
	}
	delete[] argv;
#ifdef __SINGLE_INSTANCE
	april::__unlockSingleInstanceMutex();
#endif
	return result;
}
#endif
#endif
