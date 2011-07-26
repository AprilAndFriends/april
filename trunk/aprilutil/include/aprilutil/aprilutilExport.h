/************************************************************************************\
This source file is part of the APRIL Utility library                                *
For latest info, see http://libapril.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes                                                     *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#ifndef APRILUTIL_EXPORT_H
#define APRILUTIL_EXPORT_H

	#ifdef _STATICLIB
		#define aprilutilExport
		#define aprilutilFnExport
	#else
		#ifdef _WIN32
			#ifdef APRILUTIL_EXPORTS
				#define aprilutilExport __declspec(dllexport)
				#define aprilutilFnExport __declspec(dllexport)
			#else
				#define aprilutilExport __declspec(dllimport)
				#define aprilutilFnExport __declspec(dllimport)
			#endif
		#else
			#define aprilutilExport __attribute__ ((visibility("default")))
			#define aprilutilFnExport
		#endif
	#endif
	#ifndef DEPRECATED_ATTRIBUTE
		#ifdef _MSC_VER
			#define DEPRECATED_ATTRIBUTE __declspec(deprecated("function is deprecated"))
		#else
			#define DEPRECATED_ATTRIBUTE __attribute__((deprecated))
		#endif
	#endif

#endif
