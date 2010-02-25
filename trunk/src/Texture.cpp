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
#include "Texture.h"
#include <IL/ilut.h>

namespace April
{
	ImageSource::ImageSource()
	{
		ilGenImages(1, &mImageId);
	}
	
	ImageSource::~ImageSource()
	{
		ilDeleteImages(1, &mImageId);
	}
	
	ImageSource* loadTexture(std::string filename)
	{
		ImageSource* img=new ImageSource();
		ilBindImage(img->getImageId());

		int success = ilLoadImage(filename.c_str());
		if (!success) { delete img; return 0; }
		img->w=ilGetInteger(IL_IMAGE_WIDTH);
		img->h=ilGetInteger(IL_IMAGE_HEIGHT);
		img->bpp=ilGetInteger(IL_IMAGE_BPP);
		img->format=ilGetInteger(IL_IMAGE_FORMAT);
		img->data=ilGetData();
		
		return img;
	}	
}