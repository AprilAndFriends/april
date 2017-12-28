/// @file
/// @version 5.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Defines main for standard C/C++.

#if !defined(_WIN32) || defined(_CONSOLE) && !defined(_WINRT)
#ifndef APRIL_STANDARD_MAIN_H
#define APRIL_STANDARD_MAIN_H

#include <locale.h>

#include <hltypes/harray.h>
#include <hltypes/hstring.h>

#include "main_base.h"
#include "aprilExport.h"

/// @brief Defines the main function C-standard applications.
/// @param[in] argc Number of arguments
/// @param[in] argv Arguments.
/// @return Application result code.
/// @note This is used internally only.
int main(int argc, char** argv)
{
	setlocale(LC_ALL, __APRIL_DEFAULT_LOCALE); // make sure the app uses a neutral locale that includes all specifics for all locales
#ifdef __SINGLE_INSTANCE
	if (!april::__lockSingleInstanceMutex(hstr::fromUnicode(__APRIL_SINGLE_INSTANCE_NAME), argv[0]))
	{
		return 0;
	}
#endif
	int result = april::__mainStandard(&__aprilApplicationInit, &__aprilApplicationDestroy, argc, argv);
#ifdef __SINGLE_INSTANCE
	april::__unlockSingleInstanceMutex();
#endif
	return result;
}
#endif
#endif
