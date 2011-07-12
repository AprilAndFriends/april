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

	void ImageSource::clear()
	{
		memset(this->data, 0, this->w * this->h * this->bpp * sizeof(unsigned char));
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

	void ImageSource::stretchBlit(int x, int y, int w, int h, ImageSource* other, int sx, int sy, int sw, int sh, unsigned char alpha)
	{
		x = hclamp(x, 0, this->w - 1);
		y = hclamp(y, 0, this->h - 1);
		w = hmin(w, this->w - x);
		h = hmin(h, this->h - y);
		sx = hclamp(sx, 0, other->w - 1);
		sy = hclamp(sy, 0, other->h - 1);
		sw = hmin(sw, other->w - sx);
		sh = hmin(sh, other->h - sy);
		float fw = (float)sw / w;
		float fh = (float)sh / h;
		unsigned char* c;
		unsigned char* sc;
		int a0;
		int a1;
		unsigned char color[4] = {0};
		unsigned char* ctl;
		unsigned char* ctr;
		unsigned char* cbl;
		unsigned char* cbr;
		float cx;
		float cy;
		float rx0;
		float ry0;
		float rx1;
		float ry1;
		int x0;
		int y0;
		int x1;
		int y1;
		for (int j = 0; j < h; j++)
		{
			for (int i = 0; i < w; i++)
			{
				c = &this->data[((x + i) + (y + j) * this->w) * this->bpp];
				cx = sx + i * fw;
				cy = sy + j * fh;
				x0 = (int)cx;
				y0 = (int)cy;
				x1 = hmin((int)cx + 1, other->w - 1);
				y1 = hmin((int)cy + 1, other->h - 1);
				rx0 = cx - x0;
				ry0 = cy - y0;
				rx1 = 1.0f - rx0;
				ry1 = 1.0f - ry0;
				if (rx0 != 0.0f && ry0 != 0.0f)
				{
					ctl = &other->data[(x0 + y0 * other->w) * other->bpp];
					ctr = &other->data[(x1 + y0 * other->w) * other->bpp];
					cbl = &other->data[(x0 + y1 * other->w) * other->bpp];
					cbr = &other->data[(x1 + y1 * other->w) * other->bpp];
					color[0] = (unsigned char)((ctl[0] * ry1 + cbl[0] * ry0) * rx1 + (ctr[0] * ry1 + cbr[0] * ry0) * rx0);
					color[1] = (unsigned char)((ctl[1] * ry1 + cbl[1] * ry0) * rx1 + (ctr[1] * ry1 + cbr[1] * ry0) * rx0);
					color[2] = (unsigned char)((ctl[2] * ry1 + cbl[2] * ry0) * rx1 + (ctr[2] * ry1 + cbr[2] * ry0) * rx0);
					color[3] = (unsigned char)((ctl[3] * ry1 + cbl[3] * ry0) * rx1 + (ctr[3] * ry1 + cbr[3] * ry0) * rx0);
					sc = color;
				}
				else if (rx0 != 0.0f)
				{
					ctl = &other->data[(x0 + y0 * other->w) * other->bpp];
					ctr = &other->data[(x1 + y0 * other->w) * other->bpp];
					color[0] = (unsigned char)(ctl[0] * rx1 + ctr[0] * rx0);
					color[1] = (unsigned char)(ctl[1] * rx1 + ctr[1] * rx0);
					color[2] = (unsigned char)(ctl[2] * rx1 + ctr[2] * rx0);
					color[3] = (unsigned char)(ctl[3] * rx1 + ctr[3] * rx0);
					sc = color;
				}
				else if (ry0 != 0.0f)
				{
					ctl = &other->data[(x0 + y0 * other->w) * other->bpp];
					cbl = &other->data[(x0 + y1 * other->w) * other->bpp];
					color[0] = (unsigned char)(ctl[0] * ry1 + cbl[0] * ry0);
					color[1] = (unsigned char)(ctl[1] * ry1 + cbl[1] * ry0);
					color[2] = (unsigned char)(ctl[2] * ry1 + cbl[2] * ry0);
					color[3] = (unsigned char)(ctl[3] * ry1 + cbl[3] * ry0);
					sc = color;
				}
				else
				{
					sc = &other->data[(x0 + y0 * other->w) * other->bpp];
				}
				a0 = sc[3] * (int)alpha / 255;
				a1 = 255 - a0;
				c[0] = (unsigned char)((sc[0] * a0 + c[0] * a1) / 255);
				c[1] = (unsigned char)((sc[1] * a0 + c[1] * a1) / 255);
				c[2] = (unsigned char)((sc[2] * a0 + c[2] * a1) / 255);
				c[3] = (unsigned char)hmax((int)c[3], a0);
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
	
	ImageSource* createBlankImage(int w, int h)
	{
		ImageSource* img = new ImageSource();
		unsigned char* data = new unsigned char[w * h * 4];
		memset(data, 0, w * h * 4 * sizeof(unsigned char));
		img->w = w;
		img->h = h;
		img->bpp = 4; // IL temp hack
		img->format = 6408; // IL temp hack
		img->data = data;
		img->setPixels(0, 0, w, h, april::Color(APRIL_COLOR_WHITE, 0));
		return img;
	}
	
}
