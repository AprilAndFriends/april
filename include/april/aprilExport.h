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
/// Defines macros for DLL exports/imports.

#ifndef APRIL_EXPORT_H
#define APRIL_EXPORT_H

// TODOx - remove this once merged into trunk
#define __APRIL_5_x_API

	#ifdef _LIB
		#define aprilExport
		#define aprilFnExport
	#else
		#ifdef _WIN32
			#ifdef APRIL_EXPORTS
				#define aprilExport __declspec(dllexport)
				#define aprilFnExport __declspec(dllexport)
			#else
				#define aprilExport __declspec(dllimport)
				#define aprilFnExport __declspec(dllimport)
			#endif
		#else
			#define aprilExport __attribute__ ((visibility("default")))
			#define aprilFnExport __attribute__ ((visibility("default")))
		#endif
	#endif

#endif
