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
/// Defines main for Win32 "Windows" system.

#if defined(_WIN32) && !defined(_CONSOLE) && !defined(_WINRT)
#ifndef APRIL_WIN32_MAIN_H
#define APRIL_WIN32_MAIN_H

#include <stdio.h>
#include <shellapi.h>

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
	// extract arguments
	int argc = 0;
	wchar_t** wArgv = CommandLineToArgvW(wCmdLine, &argc);
#ifdef __SINGLE_INSTANCE
	if (!april::__lockSingleInstanceMutex(hstr::from_unicode(__APRIL_SINGLE_INSTANCE_NAME), hstr::from_unicode(wArgv[0])))
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
	int result = __april_main(april_init, april_destroy, argc, argv);
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
