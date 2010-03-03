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
#ifndef APRIL_TEXTURE_H
#define APRIL_TEXTURE_H

#include <string>
#include "AprilExport.h"

namespace April
{
	class Color;
	
	class AprilExport ImageSource
	{
		unsigned int mImageId;
	public:
		ImageSource();
		~ImageSource();
		unsigned int getImageId() { return mImageId; };
		
		Color getColorAtPoint(float x,float y);
		
		unsigned char* data;
		int w,h,bpp,format;
		
	};
	
	ImageSource* loadTexture(std::string filename);
}

#endif
