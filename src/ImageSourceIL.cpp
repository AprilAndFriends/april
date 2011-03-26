/************************************************************************************\
This source file is part of the Awesome Portable Rendering Interface Library         *
For latest info, see http://libapril.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes                                                     *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#include <IL/il.h>

#include <hltypes/util.h>

#include "ImageSource.h"
#include "RenderSystem.h"

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
	
	Color ImageSource::getPixel(int x, int y)
	{
		x = hclamp(x, 0, w - 1);
		y = hclamp(y, 0, h - 1);
		Color c;
		int index = y * w + x;
		c.r = this->data[index * this->bpp];
		c.g = this->data[index * this->bpp + 1];
		c.b = this->data[index * this->bpp + 2];
		if (this->bpp == 4) // RGBA
		{
			c.a = this->data[index * this->bpp + 3];
		}
		return c;
	}
	
	void ImageSource::setPixel(int x, int y, Color c)
	{
		x = hclamp(x, 0, w - 1);
		y = hclamp(y, 0, h - 1);
		int index = y * w + x;
		this->data[index * this->bpp] = c.r;
		this->data[index * this->bpp + 1] = c.g;
		this->data[index * this->bpp + 2] = c.b;
		if (this->bpp == 4) // RGBA
		{
			this->data[index * 4 + 3] = c.a;
		}
	}

	Color ImageSource::getInterpolatedPixel(float x, float y)
	{
		return getPixel((int)x, (int)y); // TODO
	}
	
	void ImageSource::copyPixels(void* output, int format)
	{
		ilCopyPixels(0, 0, 0, w, h, 1, format, IL_UNSIGNED_BYTE, output);
	}
	
	void ImageSource::setPixels(int x, int y, int w, int h, Color c)
	{
		int size = w * h;
		unsigned char color[4] = {c.r, c.g, c.b, c.a};
		unsigned char* data = new unsigned char[size * this->bpp];
		for (int i = 0; i < size; i++)
		{
			memcpy(&data[i * this->bpp], color, this->bpp * sizeof(unsigned char));
		}
		ilSetPixels(x, y, 0, w, h, 1, IL_RGBA, IL_UNSIGNED_BYTE, data);
		delete [] data;
	}

	ImageSource* loadImage(chstr filename)
	{
		ImageSource* img = new ImageSource();
		ilBindImage(img->getImageId());
		int success = ilLoadImage(filename.c_str());
		if (!success)
		{
			delete img;
			return NULL;
		}
		img->w = ilGetInteger(IL_IMAGE_WIDTH);
		img->h = ilGetInteger(IL_IMAGE_HEIGHT);
		img->bpp = ilGetInteger(IL_IMAGE_BPP);
		img->format = ilGetInteger(IL_IMAGE_FORMAT);
		img->data = ilGetData();
		return img;
	}

	ImageSource* createEmptyImage(int w, int h)
	{
		ImageSource* img = new ImageSource();
		ilBindImage(img->getImageId());
		int size = w * h * 4 * sizeof(unsigned char);
		unsigned char* data = new unsigned char[size];
		memset(data, 0, size);
		ilTexImage(w, h, 1, 4, IL_RGBA, IL_UNSIGNED_BYTE, data);
		delete [] data;
		img->w = ilGetInteger(IL_IMAGE_WIDTH);
		img->h = ilGetInteger(IL_IMAGE_HEIGHT);
		img->bpp = ilGetInteger(IL_IMAGE_BPP);
		img->format = ilGetInteger(IL_IMAGE_FORMAT);
		img->data = ilGetData();
		return img;
	}

}
