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
		#define aprilExport
	#else
		#ifdef _WIN32
			#ifdef APRILUTIL_EXPORTS
				#define aprilUtilExport __declspec(dllexport)
				#define aprilUtilFnExport __declspec(dllexport)
			#else
				#define aprilUtilExport __declspec(dllimport)
				#define aprilUtilFnExport __declspec(dllimport)
			#endif
		#else
			#define aprilUtilExport __attribute__ ((visibility("default")))
			#define aprilUtilFnExport
		#endif
	#endif

#endif

