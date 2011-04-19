/************************************************************************************\
This source file is part of the Awesome Portable Rendering Interface Library         *
For latest info, see http://libapril.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes                                                     *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#include <string.h>
#ifdef _WIN32
#include <IL/il.h>
#endif

#include <hltypes/util.h>

#include "ImageSource.h"
#include "RenderSystem.h"

namespace april
{
    Color ImageSource::getPixel(int x, int y)
	{
		x = hclamp(x, 0, w - 1);
		y = hclamp(y, 0, h - 1);
		Color c = APRIL_COLOR_WHITE;
		int index = x + y * w;
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
		int index = x + y * w;
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
	
	void ImageSource::copyPixels(void* output, ImageFormat _format)
	{
		// todo: hacky. input and output formats can be different, fix this in the future
		if (_format == AF_BGRA)
		{
			int x, y;
			unsigned char* o = (unsigned char*)output;
			unsigned char* i = data;
			for (y = 0; y < h; y++)
			{
				for (x = 0; x < w; x++, o += 4, i += 4)
				{
					o[0] = i[2];
					o[1] = i[1];
					o[2] = i[0];
					o[3] = i[3];
				}
			}
		}
		else if (_format == AF_BGR)
		{
			int x, y;
			unsigned char* o = (unsigned char*)output;
			unsigned char* i = data;
			for (y = 0; y < h; y++)
			{
				for (x = 0; x < w; x++, o += 4, i += 3)
				{
					o[0] = i[2];
					o[1] = i[1];
					o[2] = i[0];
				//	o[3] = i[3];
				}
			}
		}
		else
		{
			memcpy(output, this->data, this->w * this->h * this->bpp);
		}
	}
	
	void ImageSource::setPixels(int x, int y, int w, int h, Color c)
	{
		x = hclamp(x, 0, this->w - 1);
		y = hclamp(y, 0, this->h - 1);
		w = hclamp(w, 1, this->w - x);
		h = hclamp(h, 1, this->h - y);
		unsigned char* ptr;
		for (int j = 0; j < h; j++)
		{
			for (int i = 0; i < w; i++)
			{
				ptr = &this->data[((x + i) + (y + j) * this->w) * this->bpp];
				ptr[0] = c.r;
				ptr[1] = c.g;
				ptr[2] = c.b;
				ptr[3] = c.a;
			}
		}
	}

	void ImageSource::copyImage(ImageSource* other)
	{
		memcpy(this->data, other->data, this->w * this->h * this->bpp * sizeof(unsigned char));
	}

	void ImageSource::blit(int x, int y, ImageSource* other, int sx, int sy, int sw, int sh, unsigned char alpha)
	{
		x = hclamp(x, 0, this->w - 1);
		y = hclamp(y, 0, this->h - 1);
		sx = hclamp(sx, 0, other->w - 1);
		sy = hclamp(sy, 0, other->h - 1);
		sw = hmin(sw, hmin(this->w - x, other->w - sx));
		sh = hmin(sh, hmin(this->h - y, other->h - sy));
		unsigned char* c;
		unsigned char* sc;
		unsigned char a;
		for (int j = 0; j < sh; j++)
		{
			for (int i = 0; i < sw; i++)
			{
				c = &this->data[((x + i) + (y + j) * this->w) * this->bpp];
				sc = &other->data[((sx + i) + (sy + j) * other->w) * other->bpp];
				a = sc[3] * alpha / 255;
				c[0] = (sc[0] * a + (255 - a) * c[0]) / 255;
				c[1] = (sc[1] * a + (255 - a) * c[1]) / 255;
				c[2] = (sc[2] * a + (255 - a) * c[2]) / 255;
				c[3] = hmax(c[3], a);
			}
		}
	}

	ImageSource* createEmptyImage(int w, int h)
	{
		ImageSource* img = new ImageSource();
		unsigned char* data = new unsigned char[w * h * 4];
		memset(data, 0, w * h * 4 * sizeof(unsigned char));
		img->w = w;
		img->h = h;
		img->bpp = 4; // IL temp hack
		img->format = 6408; // IL temp hack
		img->data = data;
		return img;
	}
	
}
