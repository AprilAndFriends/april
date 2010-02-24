/************************************************************************************
This source file is part of the Awesome Portable Rendering Interface Library
For latest info, see http://libatres.sourceforge.net/
*************************************************************************************
Copyright (c) 2010 Kresimir Spes (kreso@cateia.com)

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License (LGPL) as published by the
Free Software Foundation; either version 2 of the License, or (at your option)
any later version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.
*************************************************************************************/
#ifndef APRIL_EXPORT_H
#define APRIL_EXPORT_H

	#ifdef _STATICLIB
		#define AprilExport
	#else
		#ifdef _WIN32
			#ifdef APRIL_EXPORTS
				#define AprilExport __declspec(dllexport)
			#else
				#define AprilExport __declspec(dllimport)
			#endif
		#else
			#define AprilExport __attribute__ ((visibility("default")))
		#endif
	#endif

#endif

