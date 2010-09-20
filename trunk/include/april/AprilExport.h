/************************************************************************************\
This source file is part of the Awesome Portable Rendering Interface Library         *
For latest info, see http://libapril.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes (kreso@cateia.com)                                  *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#ifndef APRIL_EXPORT_H
#define APRIL_EXPORT_H

	#ifdef _STATICLIB
		#define AprilExport
	#else
		#ifdef _WIN32
			#ifdef APRIL_EXPORTS
				#define AprilExport __declspec(dllexport)
				#define AprilFnExport __declspec(dllexport)
			#else
				#define AprilExport __declspec(dllimport)
				#define AprilFnExport __declspec(dllimport)
			#endif
		#else
			#define AprilExport __attribute__ ((visibility("default")))
			#define AprilFnExport
		#endif
	#endif
	#ifndef DEPRECATED_ATTRIBUTE
		#ifdef _MSCVER
			#define DEPRECATED_ATTRIBUTE
		#else
			#define DEPRECATED_ATTRIBUTE __attribute__((deprecated))
		#endif
	#endif

#endif