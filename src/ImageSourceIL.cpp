/************************************************************************************\
This source file is part of the Awesome Portable Rendering Interface Library         *
For latest info, see http://libapril.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes                                                     *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#include "ImageSource.h"
#include "RenderSystem.h"
#include <IL/il.h>

namespace april
{
	ImageSource::ImageSource()
	{
		ilGenImages(1, &mImageId);
		this->compressedLength = 0;
	}
	
	ImageSource::~ImageSource()
	{
		ilDeleteImages(1, &mImageId);
	}
	
	Color ImageSource::getPixel(int x,int y)
	{
		if (x < 0) x=0;
		if (y < 0) y=0;
		if (x > w-1) x=w-1;
		if (y > h-1) y=h-1;
		
		Color c;
		int index=(y*this->w+x);
		if (this->bpp == 3) // RGB
		{
			c.r=this->data[index*3];
			c.g=this->data[index*3+1];
			c.b=this->data[index*3+2];
			c.a=255;
		}
		else if (this->bpp == 4) // RGBA
		{
			c.r=this->data[index*4];
			c.g=this->data[index*4+1];
			c.b=this->data[index*4+2];
			c.a=this->data[index*4+3];
		}
		
		return c;
	}
	
	void ImageSource::setPixel(int x,int y,Color c)
	{
		if (x < 0) x=0;
		if (y < 0) y=0;
		if (x > w-1) x=w-1;
		if (y > h-1) y=h-1;
		
		int index=(y*this->w+x);
		if (this->bpp == 3) // RGB
		{
			this->data[index*3]=c.r;
			this->data[index*3+1]=c.g;
			this->data[index*3+2]=c.b;
		}
		else if (this->bpp == 4) // RGBA
		{
			this->data[index*4]=c.r;
			this->data[index*4+1]=c.g;
			this->data[index*4+2]=c.b;
			this->data[index*4+3]=c.a;
		}
	}
	
	Color ImageSource::getInterpolatedPixel(float x,float y)
	{
		return getPixel((int)x,(int)y);
	}
	
	void ImageSource::copyPixels(void* output,int format)
	{
		ilCopyPixels(0,0,0,w,h,1,format,IL_UNSIGNED_BYTE,output);
	}
	
	ImageSource* loadImage(chstr filename)
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
