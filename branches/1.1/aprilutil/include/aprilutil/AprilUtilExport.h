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
		#define AprilExport
	#else
		#ifdef _WIN32
			#ifdef APRILUTIL_EXPORTS
				#define AprilUtilExport __declspec(dllexport)
				#define AprilUtilFnExport __declspec(dllexport)
			#else
				#define AprilUtilExport __declspec(dllimport)
				#define AprilUtilFnExport __declspec(dllimport)
			#endif
		#else
			#define AprilUtilExport __attribute__ ((visibility("default")))
			#define AprilUtilFnExport
		#endif
	#endif

#endif

