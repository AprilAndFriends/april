/************************************************************************************\
This source file is part of the Awesome Portable Rendering Interface Library         *
For latest info, see http://libapril.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes                                                     *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#ifndef APRIL_EXPORT_H
#define APRIL_EXPORT_H

	#ifdef _STATICLIB
		#define aprilExport
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
			#define aprilFnExport
		#endif
	#endif
	#ifndef DEPRECATED_ATTRIBUTE
		#ifdef _MSC_VER
			#define DEPRECATED_ATTRIBUTE
		#else
			#define DEPRECATED_ATTRIBUTE __attribute__((deprecated))
		#endif
	#endif

#endif
