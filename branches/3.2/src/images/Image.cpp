/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @version 3.2
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <string.h>

#include <hltypes/hltypesUtil.h>
#include <hltypes/hresource.h>

#include "Image.h"
#include "RenderSystem.h"

#ifdef __APPLE__
#include <TargetConditionals.h>
#endif

namespace april
{
#if TARGET_OS_IPHONE
	Image* _tryLoadingPVR(chstr filename);
#endif

	Image::Image()
	{
		this->data = NULL;
		this->w = 0;
		this->h = 0;
		this->bpp = 0;
		this->format = april::Image::FORMAT_UNDEFINED;
		this->compressedSize = 0;
	}
	
	Image::~Image()
	{
		if (this->data != NULL)
		{
			delete [] this->data;
		}
	}

	Color Image::getPixel(int x, int y)
	{
		x = hclamp(x, 0, w - 1);
		y = hclamp(y, 0, h - 1);
		Color c = Color::White;
		int index = x + y * w;
		if (this->bpp >= 3)
		{
			c.r = this->data[index * this->bpp];
			c.g = this->data[index * this->bpp + 1];
			c.b = this->data[index * this->bpp + 2];
			if (this->bpp == 4) // RGBA
			{
				c.a = this->data[index * this->bpp + 3];
			}
		}
		else
		{
			c.r = c.g = c.b = this->data[index * this->bpp];
		}
		return c;
	}
	
	void Image::setPixel(int x, int y, Color c)
	{
		x = hclamp(x, 0, w - 1);
		y = hclamp(y, 0, h - 1);
		int index = x + y * w;
		if (this->bpp >= 3)
		{

			this->data[index * this->bpp] = c.r;
			this->data[index * this->bpp + 1] = c.g;
			this->data[index * this->bpp + 2] = c.b;
			if (this->bpp == 4) // RGBA
			{
				this->data[index * 4 + 3] = c.a;
			}
		}
		else
		{
			this->data[index * this->bpp] = (c.r + c.g + c.b) / 3;
		}
	}

	Color Image::getInterpolatedPixel(float x, float y)
	{
		return getPixel((int)x, (int)y); // TODO
	}
	
	void Image::copyImage(Image* source, bool fillAlpha)
	{
		if (fillAlpha && this->bpp == 4 && source->bpp < 4)
		{
			memset(this->data, 255, this->w * this->h * this->bpp);
		}
		if ((this->bpp == 4 || source->bpp == 4) && this->bpp != source->bpp)
		{
			unsigned char* o = this->data;
			unsigned char* i = source->data;
			int x;
			for_iter (y, 0, this->h)
			{
				for (x = 0; x < this->w; x++, o += this->bpp, i += source->bpp)
				{
					o[0] = i[0];
					o[1] = i[1];
					o[2] = i[2];
				}
			}
		}
		else if (this->bpp == source->bpp)
		{
			memcpy(this->data, source->data, this->w * this->h * this->bpp * sizeof(unsigned char));
		}
		else
		{
			// not good, BPP differ too much (one might be 4 or 3 while the other is less than 3)
			Color c;
			unsigned char* o = this->data;
			unsigned char* i = source->data;
			int x;
			for_iter (y, 0, this->h)
			{
				for (x = 0; x < this->w; x++, o += this->bpp, i += source->bpp)
				{
					c = source->getPixel(x, y);
					o[0] = c.r;
					o[1] = c.g;
					o[2] = c.b;
				}
			}
		}

		if ((this->bpp == 4 || source->bpp == 4) && this->bpp != source->bpp)
		{
			if (this->bpp == 4)
			{
				memset(this->data, 255, this->w * this->h * this->bpp);
			}
			unsigned char* o = this->data;
			unsigned char* i = source->data;
			int x;
			for_iter (y, 0, this->h)
			{
				for (x = 0; x < this->w; x++, o += this->bpp, i += source->bpp)
				{
					o[0] = i[0];
					o[1] = i[1];
					o[2] = i[2];
				}
			}
		}
		else if (this->bpp == source->bpp)
		{
			memcpy(this->data, source->data, this->w * this->h * this->bpp * sizeof(unsigned char));
		}
		else
		{
			// not good, BPP differ too much (one might be 4 or 3 while the other is less than 3)
			if (this->bpp == 4)
			{
				memset(this->data, 255, this->w * this->h * this->bpp);
			}
			Color c;
			unsigned char* o = this->data;
			unsigned char* i = source->data;
			int x;
			for_iter (y, 0, this->h)
			{
				for (x = 0; x < this->w; x++, o += this->bpp, i += source->bpp)
				{
					c = source->getPixel(x, y);
					o[0] = c.r;
					o[1] = c.g;
					o[2] = c.b;
				}
			}
		}
	}

	void Image::copyPixels(void* output, Format _format)
	{
		// TODO - hacky. input and output formats can be different, fix this in the future
		if (_format == Image::FORMAT_BGRA)
		{
			unsigned char* o = (unsigned char*)output;
			unsigned char* i = data;
			int x;
			for_iter (y, 0, h)
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
		else if (_format == Image::FORMAT_BGR)
		{
			unsigned char* o = (unsigned char*)output;
			unsigned char* i = data;
			int x;
			for_iter (y, 0, h)
			{
				for (x = 0; x < w; x++, o += 4, i += 3)
				{
					o[0] = i[2];
					o[1] = i[1];
					o[2] = i[0];
				}
			}
		}
		else
		{
			memcpy(output, this->data, this->w * this->h * this->bpp);
		}
	}
	
	void Image::insertAsAlphaMap(Image* source)
	{
		if (this->bpp < 4)
		{
			return;
		}
		if (source->bpp == 4 || source->bpp == 3)
		{
			unsigned char* o = this->data;
			unsigned char* i = source->data;
			int x;
			for_iter (y, 0, this->h)
			{
				for (x = 0; x < this->w; x++, o += this->bpp, i += source->bpp)
				{
					o[3] = i[2]; // takes actually only the R component
				}
			}
		}
		else
		{
			unsigned char* o = this->data;
			int x;
			int w = source->w;
			for_iter (y, 0, this->h)
			{
				for (x = 0; x < this->w; x++, o += this->bpp)
				{
					o[3] = source->data[x + y * w];
				}
			}
		}
	}

	void Image::setPixels(int x, int y, int w, int h, Color c)
	{
		x = hclamp(x, 0, this->w - 1);
		y = hclamp(y, 0, this->h - 1);
		w = hclamp(w, 1, this->w - x);
		h = hclamp(h, 1, this->h - y);
		unsigned char* ptr;
		int i;
		for_iter (j, 0, h)
		{
			for_iterx (i, 0, w)
			{
				ptr = &this->data[((x + i) + (y + j) * this->w) * this->bpp];
				ptr[0] = c.r;
				ptr[1] = c.g;
				ptr[2] = c.b;
				ptr[3] = c.a;
			}
		}
	}

	void Image::clear()
	{
		memset(this->data, 0, this->w * this->h * this->bpp * sizeof(unsigned char));
	}

	void Image::blit(int x, int y, Image* source, int sx, int sy, int sw, int sh, unsigned char alpha)
	{
		x = hclamp(x, 0, this->w - 1);
		y = hclamp(y, 0, this->h - 1);
		sx = hclamp(sx, 0, source->w - 1);
		sy = hclamp(sy, 0, source->h - 1);
		sw = hmin(sw, hmin(this->w - x, source->w - sx));
		sh = hmin(sh, hmin(this->h - y, source->h - sy));
		unsigned char* c;
		unsigned char* sc;
		unsigned char a;
		int i;
		for_iter (j, 0, sh)
		{
			for_iterx (i, 0, sw)
			{
				c = &this->data[((x + i) + (y + j) * this->w) * this->bpp];
				sc = &source->data[((sx + i) + (sy + j) * source->w) * source->bpp];
				a = sc[3] * alpha / 255;
				c[0] = (sc[0] * a + (255 - a) * c[0]) / 255;
				c[1] = (sc[1] * a + (255 - a) * c[1]) / 255;
				c[2] = (sc[2] * a + (255 - a) * c[2]) / 255;
				c[3] = a + (255 - a) * c[3] / 255;
			}
		}
	}

	void Image::stretchBlit(int x, int y, int w, int h, Image* source, int sx, int sy, int sw, int sh, unsigned char alpha)
	{
		x = hclamp(x, 0, this->w - 1);
		y = hclamp(y, 0, this->h - 1);
		w = hmin(w, this->w - x);
		h = hmin(h, this->h - y);
		sx = hclamp(sx, 0, source->w - 1);
		sy = hclamp(sy, 0, source->h - 1);
		sw = hmin(sw, source->w - sx);
		sh = hmin(sh, source->h - sy);
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
		int i;
		for_iter (j, 0, h)
		{
			for_iterx (i, 0, w)
			{
				c = &this->data[((x + i) + (y + j) * this->w) * this->bpp];
				cx = sx + i * fw;
				cy = sy + j * fh;
				x0 = (int)cx;
				y0 = (int)cy;
				x1 = hmin((int)cx + 1, source->w - 1);
				y1 = hmin((int)cy + 1, source->h - 1);
				rx0 = cx - x0;
				ry0 = cy - y0;
				rx1 = 1.0f - rx0;
				ry1 = 1.0f - ry0;
				if (rx0 != 0.0f && ry0 != 0.0f)
				{
					ctl = &source->data[(x0 + y0 * source->w) * source->bpp];
					ctr = &source->data[(x1 + y0 * source->w) * source->bpp];
					cbl = &source->data[(x0 + y1 * source->w) * source->bpp];
					cbr = &source->data[(x1 + y1 * source->w) * source->bpp];
					color[0] = (unsigned char)((ctl[0] * ry1 + cbl[0] * ry0) * rx1 + (ctr[0] * ry1 + cbr[0] * ry0) * rx0);
					color[1] = (unsigned char)((ctl[1] * ry1 + cbl[1] * ry0) * rx1 + (ctr[1] * ry1 + cbr[1] * ry0) * rx0);
					color[2] = (unsigned char)((ctl[2] * ry1 + cbl[2] * ry0) * rx1 + (ctr[2] * ry1 + cbr[2] * ry0) * rx0);
					color[3] = (unsigned char)((ctl[3] * ry1 + cbl[3] * ry0) * rx1 + (ctr[3] * ry1 + cbr[3] * ry0) * rx0);
					sc = color;
				}
				else if (rx0 != 0.0f)
				{
					ctl = &source->data[(x0 + y0 * source->w) * source->bpp];
					ctr = &source->data[(x1 + y0 * source->w) * source->bpp];
					color[0] = (unsigned char)(ctl[0] * rx1 + ctr[0] * rx0);
					color[1] = (unsigned char)(ctl[1] * rx1 + ctr[1] * rx0);
					color[2] = (unsigned char)(ctl[2] * rx1 + ctr[2] * rx0);
					color[3] = (unsigned char)(ctl[3] * rx1 + ctr[3] * rx0);
					sc = color;
				}
				else if (ry0 != 0.0f)
				{
					ctl = &source->data[(x0 + y0 * source->w) * source->bpp];
					cbl = &source->data[(x0 + y1 * source->w) * source->bpp];
					color[0] = (unsigned char)(ctl[0] * ry1 + cbl[0] * ry0);
					color[1] = (unsigned char)(ctl[1] * ry1 + cbl[1] * ry0);
					color[2] = (unsigned char)(ctl[2] * ry1 + cbl[2] * ry0);
					color[3] = (unsigned char)(ctl[3] * ry1 + cbl[3] * ry0);
					sc = color;
				}
				else
				{
					sc = &source->data[(x0 + y0 * source->w) * source->bpp];
				}
				a0 = sc[3] * (int)alpha / 255;
				a1 = 255 - a0;
				c[0] = (unsigned char)((sc[0] * a0 + c[0] * a1) / 255);
				c[1] = (unsigned char)((sc[1] * a0 + c[1] * a1) / 255);
				c[2] = (unsigned char)((sc[2] * a0 + c[2] * a1) / 255);
				c[3] = (unsigned char)(a0 + c[3] * a1 / 255);
			}
		}
	}

	Image* Image::load(chstr filename)
	{
		Image* img = NULL;
		
		if (filename.lower().ends_with(".png"))
		{
			hresource res(filename);
			img = Image::_loadPng(res);
		}
		else if (filename.lower().ends_with(".jpg") || filename.lower().ends_with(".jpeg"))
		{
			hresource res(filename);
			img = Image::_loadJpg(res);
		}
		else if (filename.lower().ends_with(".jpt"))
		{
			hresource res(filename);
			img = Image::_loadJpt(res);
		}
#if TARGET_OS_IPHONE
		else if (filename.lower().ends_with(".pvr"))
		{
			img = _tryLoadingPVR(filename);
		}
#endif
		return img;
	}

	Image* Image::create(int w, int h, Color color)
	{
		unsigned char* data = new unsigned char[w * h * 4];
		bool uniColor = (color.r == color.g == color.b == color.a);
		if (uniColor)
		{
			memset(data, color.r, w * h * 4 * sizeof(unsigned char));
		}
		Image* img = new Image();
		img->w = w;
		img->h = h;
		img->bpp = 4;
		img->format = Image::FORMAT_RGBA;
		img->data = data;
		img->compressedSize = 0;
		if (!uniColor)
		{
			img->setPixels(0, 0, w, h, color);
		}
		return img;
	}
	
}
