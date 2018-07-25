/// @file
/// @version 5.2
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Defines main functions. Keep in mind following:
///
/// - You MUST include this header somewhere in your project (typically main.cpp).
/// - This header must only be included ONCE in the entire project.
/// - When using OpenKODE, include KD/kd.h before including this header.
/// 
/// Following preprocessors must be defined before including this header:
///
/// - Windows native: _WIN32
/// - Windows with console: _WIN32 _CONSOLE
/// - UWP: _UWP
/// - WinRT: _WINRT
/// - Android: __ANDROID__
/// - Mac: __APPLE__
/// - iOS: __APPLE__
/// - Linux: _UNIX

#ifndef APRIL_MAIN_H
#define APRIL_MAIN_H

#ifdef __APPLE__
#include <TargetConditionals.h>
#endif

#define __HL_INCLUDE_PLATFORM_HEADERS
#include <hltypes/hplatform.h>
#include <hltypes/harray.h>
#include <hltypes/hstring.h>

#include "main_base.h"
#include "aprilExport.h"

#ifdef __ANDROID__
	#include "Android_main.h"
#elif defined(_UWP)
	#include "UWP_main.h"
#elif defined(_WINRT)
	#include "WinRT_main.h"
#elif defined(_WIN32) && !defined(_CONSOLE)
	#include "Win32_main.h"
#else
	#include "Standard_main.h"
#endif

#endif