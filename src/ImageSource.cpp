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
#include "ImageSource.h"
#include "RenderSystem.h"
#include <IL/il.h>

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
	
	Color ImageSource::getPixel(int x,int y)
	{
		if (x < 0 || y < 0 || x > this->w || y > this->h) return Color(1,1,1,1);
		
		Color c;
		int index=(y*this->w+x);
		if (this->bpp == 3) // RGB
		{
			c.r=this->data[index*3];
			c.g=this->data[index*3+1];
			c.b=this->data[index*3+2];
			c.a=1;
		}
		else if (this->bpp == 4) // RGBA
		{
			c.r=this->data[index*4];
			c.g=this->data[index*4+1];
			c.b=this->data[index*4+2];
			c.a=this->data[index*4+3];;
		}
		
		return c;
	}
	
	Color ImageSource::getInterpolatedPixel(float x,float y)
	{
		return getPixel(x,y);
	}
	
	ImageSource* loadImage(std::string filename)
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
