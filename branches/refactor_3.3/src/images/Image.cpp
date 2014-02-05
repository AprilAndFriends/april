/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @version 3.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <string.h>

#include <hltypes/hstring.h>
#include <hltypes/hlog.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hresource.h>

#include "april.h"
#include "Color.h"
#include "Image.h"
#include "RenderSystem.h"

#ifdef __APPLE__
#include <TargetConditionals.h>
#endif

#define CHECK_RIGHT_SHIFT_FORMATS(srcFormat, destFormat) \
	(((srcFormat) == FORMAT_RGBA || (srcFormat) == FORMAT_RGBX) && ((destFormat) == FORMAT_ARGB || (destFormat) == FORMAT_XRGB) || \
	((srcFormat) == FORMAT_BGRA || (srcFormat) == FORMAT_BGRX) && ((destFormat) == FORMAT_ABGR || (destFormat) == FORMAT_XBGR))
#define CHECK_LEFT_SHIFT_FORMATS(srcFormat, destFormat) \
	(((srcFormat) == FORMAT_ARGB || (srcFormat) == FORMAT_XRGB) && ((destFormat) == FORMAT_RGBA || (destFormat) == FORMAT_RGBX) || \
	((srcFormat) == FORMAT_ABGR || (srcFormat) == FORMAT_XBGR) && ((destFormat) == FORMAT_BGRA || (destFormat) == FORMAT_BGRX))
#define CHECK_INVERT_ORDER_FORMATS(srcFormat, destFormat) \
	(((srcFormat) == FORMAT_RGBA || (srcFormat) == FORMAT_ARGB || (srcFormat) == FORMAT_RGBX || (srcFormat) == FORMAT_XRGB) && \
	((destFormat) == FORMAT_BGRA || (destFormat) == FORMAT_ABGR || (destFormat) == FORMAT_BGRX || (destFormat) == FORMAT_XBGR) || \
	((srcFormat) == FORMAT_BGRA || (srcFormat) == FORMAT_ABGR || (srcFormat) == FORMAT_BGRX || (srcFormat) == FORMAT_XBGR) && \
	((destFormat) == FORMAT_RGBA || (destFormat) == FORMAT_ARGB || (destFormat) == FORMAT_RGBX || (destFormat) == FORMAT_XRGB))
#define CHECK_COPY_ALPHA_FORMATS(srcFormat, destFormat) \
	(((srcFormat) == FORMAT_RGBA || (srcFormat) == FORMAT_ARGB || (srcFormat) == FORMAT_BGRA || (srcFormat) == FORMAT_ABGR) && \
	((destFormat) == FORMAT_RGBA || (destFormat) == FORMAT_ARGB || (destFormat) == FORMAT_BGRA || (destFormat) == FORMAT_ABGR))

#define RIGHT_SHIFT(value) ((value) >> 8)
#define RIGHT_SHIFT_WITH_ALPHA(value) (((value) >> 8) | ((value) << 24))
#define LEFT_SHIFT(value) ((value) << 8)
#define LEFT_SHIFT_WITH_ALPHA(value) (((value) << 8) | ((value) >> 24))
#define INVERTED_RIGHT_SHIFT(value) (((((value) & 0xFF000000 >> 16) | ((value) & 0xFF00 << 16) | ((value) & 0xFF0000))) >> 8)
#define INVERTED_RIGHT_SHIFT_WITH_ALPHA(value) (((((value) & 0xFF000000 >> 16) | ((value) & 0xFF00 << 16) | ((value) & 0xFF0000)) >> 8) | ((value) << 24))
#define INVERTED_LEFT_SHIFT(value) ((((value) & 0xFF0000 >> 16) | ((value) & 0xFF << 16) | ((value) & 0xFF00)) << 8)
#define INVERTED_LEFT_SHIFT_WITH_ALPHA(value) (((((value) & 0xFF0000 >> 16) | ((value) & 0xFF << 16) | ((value) & 0xFF00)) << 8) | ((value) >> 24))

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
		this->format = FORMAT_INVALID;
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
		if (_format == FORMAT_BGRA)
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
		else if (_format == FORMAT_BGR)
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
	
	int Image::getFormatBpp(Image::Format format)
	{
		switch (format)
		{
		case FORMAT_RGBA:	return 4;
		case FORMAT_ARGB:	return 4;
		case FORMAT_BGRA:	return 4;
		case FORMAT_ABGR:	return 4;
		case FORMAT_RGBX:	return 4;
		case FORMAT_XRGB:	return 4;
		case FORMAT_BGRX:	return 4;
		case FORMAT_XBGR:	return 4;
		case FORMAT_RGB:	return 3;
		case FORMAT_BGR:	return 3;
		case FORMAT_ALPHA:	return 1;
		}
		return 0;
	}
	
	bool Image::convertToFormat(unsigned char* srcData, unsigned char** destData, int w, int h, Image::Format srcFormat, Image::Format destFormat)
	{
		if (*destData != NULL)
		{
			hlog::error(april::logTag, "The destination data pointer is not NULL!");
			return false;
		}
		if (srcFormat == destFormat)
		{
			hlog::warn(april::logTag, "The source format's and destination format's are the same!");
			return false;
		}
		int srcBpp = Image::getFormatBpp(srcFormat);
		if (srcBpp == 0)
		{
			hlog::error(april::logTag, "The source format's BPP is not supported!");
			return false;
		}
		int destBpp = Image::getFormatBpp(destFormat);
		if (destBpp == 0)
		{
			hlog::error(april::logTag, "The destination format's BPP is not supported!");
			return false;
		}
		if (srcBpp != destBpp)
		{
			hlog::error(april::logTag, "The source format's and destination format's BPP are different!");
			return false;
		}
		if (srcBpp == 1)
		{
			*destData = new unsigned char[w * h * srcBpp];
			memcpy(*destData, srcData, w * h * srcBpp);
			return true;
		}
		if (srcBpp == 3)
		{
			*destData = new unsigned char[w * h * srcBpp];
			memcpy(*destData, srcData, w * h * srcBpp);
			// FORMAT_RGB to FORMAT_BGR and vice versa, thus switching 2 bytes around is enough
			int i = 0;
			for_iter (y, 0, h)
			{
				for_iter (x, 0, w)
				{
					i = (x + y * w) * srcBpp;
					(*destData)[i] = srcData[i + 2];
					(*destData)[i + 2] = srcData[i];
				}
			}
			return true;
		}
		if (srcBpp == 4)
		{
			*destData = new unsigned char[w * h * srcBpp];
			memset(*destData, 255, w * h * srcBpp);
			// shifting unsigned int's around is faster than pure assigning (like at 3 BPP)
			unsigned int* src = (unsigned int*)srcData;
			unsigned int* dest = (unsigned int*)*destData;
			bool rightShift = CHECK_RIGHT_SHIFT_FORMATS(srcFormat, destFormat);
			bool leftShift = CHECK_LEFT_SHIFT_FORMATS(srcFormat, destFormat);
			bool invertOrder = CHECK_INVERT_ORDER_FORMATS(srcFormat, destFormat);
			bool copyAlpha = CHECK_COPY_ALPHA_FORMATS(srcFormat, destFormat);
			int i = 0;
			if (invertOrder)
			{
				if (rightShift)
				{
					if (copyAlpha)
					{
						for_iter (y, 0, h)
						{
							for_iter (x, 0, w)
							{
								i = (x + y * w);
								dest[i] = INVERTED_RIGHT_SHIFT_WITH_ALPHA(src[i]);
							}
						}
					}
					else
					{
						for_iter (y, 0, h)
						{
							for_iter (x, 0, w)
							{
								i = (x + y * w);
								dest[i] = INVERTED_RIGHT_SHIFT(src[i]);
							}
						}
					}
				}
				else if (copyAlpha)
				{
					for_iter (y, 0, h)
					{
						for_iter (x, 0, w)
						{
							i = (x + y * w);
							dest[i] = INVERTED_LEFT_SHIFT_WITH_ALPHA(src[i]);
						}
					}
				}
				else
				{
					for_iter (y, 0, h)
					{
						for_iter (x, 0, w)
						{
							i = (x + y * w);
							dest[i] = INVERTED_LEFT_SHIFT(src[i]);
						}
					}
				}
			}
			else if (rightShift)
			{
				if (copyAlpha)
				{
					for_iter (y, 0, h)
					{
						for_iter (x, 0, w)
						{
							i = (x + y * w);
							dest[i] = RIGHT_SHIFT_WITH_ALPHA(src[i]);
						}
					}
				}
				else
				{
					for_iter (y, 0, h)
					{
						for_iter (x, 0, w)
						{
							i = (x + y * w);
							dest[i] = RIGHT_SHIFT(src[i]);
						}
					}
				}
			}
			else if (copyAlpha)
			{
				for_iter (y, 0, h)
				{
					for_iter (x, 0, w)
					{
						i = (x + y * w);
						dest[i] = LEFT_SHIFT_WITH_ALPHA(src[i]);
					}
				}
			}
			else
			{
				for_iter (y, 0, h)
				{
					for_iter (x, 0, w)
					{
						i = (x + y * w);
						dest[i] = LEFT_SHIFT(src[i]);
					}
				}
			}
			return true;
		}
		hlog::errorf(april::logTag, "Formats' BPP of %d is not supported!", srcBpp);
		return false;
	}

	void Image::fillRect(int x, int y, int w, int h, Color color, unsigned char* destData, int destWidth, int destHeight, Format destFormat)
	{
		x = hclamp(x, 0, destWidth - 1);
		y = hclamp(y, 0, destHeight - 1);
		w = hclamp(w, 1, destWidth - x);
		h = hclamp(h, 1, destHeight - y);
		int destBpp = Image::getFormatBpp(destFormat);
		int i = (x + y * destWidth) * destBpp;
		int copyWidth = w * destBpp;
		int size = copyWidth * destHeight;
		if (destBpp == 1 || destBpp == 3 && color.r == color.g && color.r == color.g || destBpp == 4 && color.r == color.g && color.r == color.g && color.r == color.a)
		{
			if (x == 0 && w == destWidth)
			{
				memset(&destData[i], color.r, copyWidth * h);
			}
			else
			{
				for_iter (j, 0, h)
				{
					memset(&destData[(x + (y + j) * destWidth) * destBpp], color.r, copyWidth);
				}
			}
			return;
		}
		unsigned char colorData[4] = {color.r, color.g, color.b, color.a};
		// convert to right format first
		Format srcFormat = (destBpp == 4 ? FORMAT_RGBA : (destBpp == 3 ? FORMAT_RGB : FORMAT_ALPHA));
		if (srcFormat != destFormat)
		{
			unsigned char* c = NULL;
			Image::convertToFormat(colorData, &c, destWidth, destHeight, srcFormat, destFormat);
			memcpy(&destData[i], c, destBpp);
			delete [] c;
		}
		else
		{
			memcpy(&destData[i], colorData, destBpp);
		}
		int currentSize = destBpp;
		int copySize = 0;
		// TODOaa - test if this works as intended
		if (x == 0 && w == destWidth)
		{
			while (currentSize < size)
			{
				copySize = hmin(size - currentSize, currentSize);
				memcpy(&destData[i + currentSize], &destData[i], copySize);
				currentSize += copySize;
			}
		}
		else
		{
			// copy on first line
			while (currentSize < copyWidth)
			{
				copySize = hmin(copyWidth - currentSize, currentSize);
				memcpy(&destData[i + currentSize], &destData[i], copySize);
				currentSize += copySize;
			}
			// copy to all lines
			for_iter (j, 1, h)
			{
				memcpy(&destData[(x + (y + j) * destWidth) * destBpp], &destData[i], currentSize);
			}
		}
	}
	
}
