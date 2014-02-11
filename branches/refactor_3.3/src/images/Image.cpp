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

#define CHECK_SHIFT_FORMATS(format1, format2) (\
	((format1) == FORMAT_RGBA || (format1) == FORMAT_RGBX || (format1) == FORMAT_BGRA || (format1) == FORMAT_BGRX) && \
	((format2) == FORMAT_ARGB || (format2) == FORMAT_XRGB || (format2) == FORMAT_ABGR || (format2) == FORMAT_XBGR) \
)
#define CHECK_INVERT_ORDER_FORMATS(format1, format2) (\
	((format1) == FORMAT_RGBA || (format1) == FORMAT_RGBX || (format1) == FORMAT_ARGB || (format1) == FORMAT_XRGB) && \
	((format2) == FORMAT_BGRA || (format2) == FORMAT_BGRX || (format2) == FORMAT_ABGR || (format2) == FORMAT_XBGR) \
)
#define CHECK_LEFT_RGB(format) \
	((format) == FORMAT_RGBA || (format) == FORMAT_RGBX || (format) == FORMAT_BGRA || (format) == FORMAT_BGRX)
#define CHECK_ALPHA_FORMAT(format) \
	((format) == FORMAT_RGBA || (format) == FORMAT_ARGB || (format) == FORMAT_BGRA || (format) == FORMAT_ABGR)

#define FOR_EACH_4BPP_PIXEL(macro) \
	for_iter (y, 0, h) \
	{ \
		for_iter (x, 0, w) \
		{ \
			i = (x + y * w); \
			dest[i] = macro(src[i]); \
		} \
	}
#define FOR_EACH_3BPP_TO_4BPP_PIXEL(exec) \
	for_iter (y, 0, h) \
	{ \
		for_iter (x, 0, w) \
		{ \
			i = (x + y * w) * srcBpp; \
			dest[x + y * w] = (exec); \
		} \
	}
#define FOR_EACH_4BPP_TO_3BPP_PIXEL(exec1, exec2, exec3) \
	for_iter (y, 0, h) \
	{ \
		for_iter (x, 0, w) \
		{ \
			i = x + y * w; \
			j = (x + y * w) * destBpp; \
			dest[j] = (unsigned char)(exec1); \
			dest[j + 1] = (unsigned char)(exec2); \
			dest[j + 2] = (unsigned char)(exec3); \
		} \
	}

// these all assume little endian
#define _R_ALPHA 0xFF
#define _L_ALPHA 0xFF000000
#define _KEEP_R(value) ((value) & 0xFFFFFF00)
#define _KEEP_L(value) ((value) & 0xFFFFFF)
#define _RIGHT_SHIFT(value) ((value) << 8)
#define _LEFT_SHIFT(value) ((value) >> 8)
#define _R_SHIFT_ALPHA(value) ((value) >> 24)
#define _L_SHIFT_ALPHA(value) ((value) << 24)
#define _INVERTED_R(value) ((((value) & 0xFF000000) >> 16) | (((value) & 0xFF00) << 16) | ((value) & 0xFF0000))
#define _INVERTED_L(value) ((((value) & 0xFF0000) >> 16) | (((value) & 0xFF) << 16) | ((value) & 0xFF00))
#define _INVERTED_RIGHT_SHIFT(value) (((value) << 24) | (((value) & 0xFF00) << 8) | (((value) & 0xFF0000) >> 8))
#define _INVERTED_LEFT_SHIFT(value) (((value) >> 24) | (((value) & 0xFF0000) >> 8) | (((value) & 0xFF00) << 8))

#define KEEP_R(value) (_KEEP_R(value) | _R_ALPHA)
#define KEEP_L(value) (_KEEP_L(value) | _L_ALPHA)
#define RIGHT_SHIFT(value) (_RIGHT_SHIFT(value) | _R_ALPHA)
#define RIGHT_SHIFT_WITH_ALPHA(value) (_RIGHT_SHIFT(value) | _R_SHIFT_ALPHA(value))
#define LEFT_SHIFT(value) (_LEFT_SHIFT(value) | _L_ALPHA)
#define LEFT_SHIFT_WITH_ALPHA(value) (_LEFT_SHIFT(value) | _L_SHIFT_ALPHA(value))
#define INVERTED_R(value) (_INVERTED_R(value) | _R_ALPHA)
#define INVERTED_R_WITH_ALPHA(value) (_INVERTED_R(value) | ((value) & _R_ALPHA))
#define INVERTED_L(value) (_INVERTED_L(value) | _L_ALPHA)
#define INVERTED_L_WITH_ALPHA(value) (_INVERTED_L(value) | ((value) & _L_ALPHA))
#define INVERTED_RIGHT_SHIFT(value) (_INVERTED_RIGHT_SHIFT(value) | _R_ALPHA)
#define INVERTED_RIGHT_SHIFT_WITH_ALPHA(value) (_INVERTED_RIGHT_SHIFT(value) | _R_SHIFT_ALPHA(value))
#define INVERTED_LEFT_SHIFT(value) (_INVERTED_LEFT_SHIFT(value) | _L_ALPHA)
#define INVERTED_LEFT_SHIFT_WITH_ALPHA(value) (_INVERTED_LEFT_SHIFT(value) | _L_SHIFT_ALPHA(value))

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
		this->format = FORMAT_INVALID;
		this->internalFormat = 0;
		this->compressedSize = 0;
	}
	
	Image::~Image()
	{
		if (this->data != NULL)
		{
			delete [] this->data;
		}
	}

	int Image::getBpp()
	{
		return Image::getFormatBpp(this->format);
	}

	int Image::getByteSize()
	{
		return (this->w * this->h * Image::getFormatBpp(this->format));
	}

	void Image::clear()
	{
		memset(this->data, 0, this->getByteSize());
	}

	Color Image::getPixel(int x, int y)
	{
		Color color = Color::Clear;
		if (this->data != NULL)
		{
			color = Image::getPixel(x, y, this->data, this->w, this->h, this->format);
		}
		return color;
	}
	
	void Image::setPixel(int x, int y, Color color)
	{
		if (this->data != NULL)
		{
			Image::setPixel(x, y, color, this->data, this->w, this->h, this->format);
		}
	}
	
	Color Image::getInterpolatedPixel(float x, float y)
	{
		Color color = Color::Clear;
		if (this->data != NULL)
		{
			color = Image::getInterpolatedPixel(x, y, this->data, this->w, this->h, this->format);
		}
		return color;
	}
	
	void Image::fillRect(int x, int y, int w, int h, Color color)
	{
		if (this->data != NULL)
		{
			Image::fillRect(x, y, w, h, color, this->data, this->w, this->h, this->format);
		}
	}

	void Image::write(int sx, int sy, int sw, int sh, int dx, int dy, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat)
	{
		if (this->data != NULL)
		{
			Image::write(sx, sy, sw, sh, dx, dy, srcData, srcWidth, srcHeight, srcFormat, this->data, this->w, this->h, this->format);
		}
	}

	bool Image::copyPixelData(unsigned char** output, Format format)
	{
		return (this->data != NULL && Image::convertToFormat(this->w, this->h, this->data, this->format, output, format, false));
	}

	void Image::insertAlphaMap(unsigned char* srcData, Format srcFormat)
	{
		if (this->data != NULL)
		{
			Image::insertAlphaMap(this->w, this->h, srcData, srcFormat, this->data, this->format);
		}
	}

	// TODOaa - more functions go here
	




	Color Image::getPixel(gvec2 position)
	{
		return this->getPixel((int)position.x, (int)position.y);
	}
	
	void Image::setPixel(gvec2 position, Color color)
	{
		this->setPixel((int)position.x, (int)position.y, color);
	}
	
	Color Image::getInterpolatedPixel(gvec2 position)
	{
		return this->getPixel((int)position.x, (int)position.y); // TODO
	}
	
	void Image::write(int sx, int sy, int sw, int sh, int dx, int dy, Image* other)
	{
		this->write(sx, sy, sw, sh, dx, dy, other->data, other->w, other->h, other->format);
	}

	bool Image::copyPixelData(unsigned char** output)
	{
		return (this->data != NULL && Image::convertToFormat(this->w, this->h, this->data, this->format, output, this->format, false));
	}

	void Image::insertAlphaMap(Image* source)
	{
		this->insertAlphaMap(source->data, source->format);
	}



	// TODOaa - more function overloads go here



	void Image::blit(int x, int y, Image* source, int sx, int sy, int sw, int sh, unsigned char alpha)
	{
		// TODOaa - refactor this
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
				c = &this->data[((x + i) + (y + j) * this->w) * this->getBpp()];
				sc = &source->data[((sx + i) + (sy + j) * source->w) * source->getBpp()];
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
		// TODOaa - refactor this
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
				c = &this->data[((x + i) + (y + j) * this->w) * this->getBpp()];
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
					ctl = &source->data[(x0 + y0 * source->w) * source->getBpp()];
					ctr = &source->data[(x1 + y0 * source->w) * source->getBpp()];
					cbl = &source->data[(x0 + y1 * source->w) * source->getBpp()];
					cbr = &source->data[(x1 + y1 * source->w) * source->getBpp()];
					color[0] = (unsigned char)((ctl[0] * ry1 + cbl[0] * ry0) * rx1 + (ctr[0] * ry1 + cbr[0] * ry0) * rx0);
					color[1] = (unsigned char)((ctl[1] * ry1 + cbl[1] * ry0) * rx1 + (ctr[1] * ry1 + cbr[1] * ry0) * rx0);
					color[2] = (unsigned char)((ctl[2] * ry1 + cbl[2] * ry0) * rx1 + (ctr[2] * ry1 + cbr[2] * ry0) * rx0);
					color[3] = (unsigned char)((ctl[3] * ry1 + cbl[3] * ry0) * rx1 + (ctr[3] * ry1 + cbr[3] * ry0) * rx0);
					sc = color;
				}
				else if (rx0 != 0.0f)
				{
					ctl = &source->data[(x0 + y0 * source->w) * source->getBpp()];
					ctr = &source->data[(x1 + y0 * source->w) * source->getBpp()];
					color[0] = (unsigned char)(ctl[0] * rx1 + ctr[0] * rx0);
					color[1] = (unsigned char)(ctl[1] * rx1 + ctr[1] * rx0);
					color[2] = (unsigned char)(ctl[2] * rx1 + ctr[2] * rx0);
					color[3] = (unsigned char)(ctl[3] * rx1 + ctr[3] * rx0);
					sc = color;
				}
				else if (ry0 != 0.0f)
				{
					ctl = &source->data[(x0 + y0 * source->w) * source->getBpp()];
					cbl = &source->data[(x0 + y1 * source->w) * source->getBpp()];
					color[0] = (unsigned char)(ctl[0] * ry1 + cbl[0] * ry0);
					color[1] = (unsigned char)(ctl[1] * ry1 + cbl[1] * ry0);
					color[2] = (unsigned char)(ctl[2] * ry1 + cbl[2] * ry0);
					color[3] = (unsigned char)(ctl[3] * ry1 + cbl[3] * ry0);
					sc = color;
				}
				else
				{
					sc = &source->data[(x0 + y0 * source->w) * source->getBpp()];
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
		Image* image = NULL;
		if (filename.lower().ends_with(".png"))
		{
			hresource res(filename);
			image = Image::_loadPng(res);
		}
		else if (filename.lower().ends_with(".jpg") || filename.lower().ends_with(".jpeg"))
		{
			hresource res(filename);
			image = Image::_loadJpg(res);
		}
		else if (filename.lower().ends_with(".jpt"))
		{
			hresource res(filename);
			image = Image::_loadJpt(res);
		}
#if TARGET_OS_IPHONE
		else if (filename.lower().ends_with(".pvr"))
		{
			image = _tryLoadingPVR(filename);
		}
#endif
		return image;
	}

	Image* Image::create(int w, int h, unsigned char* data, Image::Format format)
	{
		Image* image = new Image();
		image->w = w;
		image->h = h;
		image->format = format;
		image->compressedSize = 0;
		int size = image->getByteSize();
		image->data = new unsigned char[size];
		memcpy(image->data, data, size);
		return image;
	}

	Image* Image::create(int w, int h, Color color, Image::Format format)
	{
		Image* image = new Image();
		image->w = w;
		image->h = h;
		image->format = format;
		image->compressedSize = 0;
		int size = image->getByteSize();
		image->data = new unsigned char[size];
		image->fillRect(0, 0, image->w, image->h, color);
		return image;
	}

	Image* Image::create(Image* other)
	{
		Image* image = new Image();
		image->w = other->w;
		image->h = other->h;
		image->format = other->format;
		image->compressedSize = other->compressedSize;
		int size = image->getByteSize();
		image->data = new unsigned char[size];
		memcpy(image->data, other->data, size);
		return image;
	}

	int Image::getFormatBpp(Image::Format format)
	{
		switch (format)
		{
		case FORMAT_RGBA:		return 4;
		case FORMAT_ARGB:		return 4;
		case FORMAT_BGRA:		return 4;
		case FORMAT_ABGR:		return 4;
		case FORMAT_RGBX:		return 4;
		case FORMAT_XRGB:		return 4;
		case FORMAT_BGRX:		return 4;
		case FORMAT_XBGR:		return 4;
		case FORMAT_RGB:		return 3;
		case FORMAT_BGR:		return 3;
		case FORMAT_ALPHA:		return 1;
		case FORMAT_GRAYSCALE:	return 1;
		}
		return 0;
	}

	Color Image::getPixel(int x, int y, unsigned char* srcData, int srcWidth, int srcHeight, Format srcFormat)
	{
		Color color = Color::Clear;
		unsigned char* rgba = NULL;
		if (Image::convertToFormat(1, 1, &srcData[(x + y * srcWidth) * Image::getFormatBpp(srcFormat)], srcFormat, &rgba, Image::FORMAT_RGBA, false))
		{
			color.r = rgba[0];
			color.g = rgba[1];
			color.b = rgba[2];
			color.a = rgba[3];
			delete [] rgba;
		}
		return color;
	}
	
	bool Image::setPixel(int x, int y, Color color, unsigned char* destData, int destWidth, int destHeight, Format destFormat)
	{
		unsigned char rgba[4] = {color.r, color.g, color.b, color.a};
		unsigned char* p = &destData[(x + y * destWidth) * Image::getFormatBpp(destFormat)];
		return Image::convertToFormat(1, 1, rgba, Image::FORMAT_RGBA, &p, destFormat, false);
	}

	Color Image::getInterpolatedPixel(float x, float y, unsigned char* srcData, int srcWidth, int srcHeight, Format srcFormat)
	{
		Color result;
		int x0 = (int)x;
		int y0 = (int)y;
		int x1 = x0 + 1;
		int y1 = y0 + 1;
		float rx0 = x - x0;
		float ry0 = y - y0;
		float rx1 = 1.0f - rx0;
		float ry1 = 1.0f - ry0;
		if (rx0 != 0.0f && ry0 != 0.0f)
		{
			Color tl = Image::getPixel(x0, y0, srcData, srcWidth, srcHeight, srcFormat);
			Color tr = Image::getPixel(x1, y0, srcData, srcWidth, srcHeight, srcFormat);
			Color bl = Image::getPixel(x0, y1, srcData, srcWidth, srcHeight, srcFormat);
			Color br = Image::getPixel(x1, y1, srcData, srcWidth, srcHeight, srcFormat);
			result = (tl * ry1 + bl * ry0) * rx1 + (tr * ry1 + br * ry0) * rx0;
		}
		else if (rx0 != 0.0f)
		{
			Color tl = Image::getPixel(x0, y0, srcData, srcWidth, srcHeight, srcFormat);
			Color tr = Image::getPixel(x1, y0, srcData, srcWidth, srcHeight, srcFormat);
			result = tl * rx1 + tr * rx0;
		}
		else if (ry0 != 0.0f)
		{
			Color tl = Image::getPixel(x0, y0, srcData, srcWidth, srcHeight, srcFormat);
			Color bl = Image::getPixel(x0, y1, srcData, srcWidth, srcHeight, srcFormat);
			result = tl * ry1 + bl * ry0;
		}
		else
		{
			result = Image::getPixel(x0, y0, srcData, srcWidth, srcHeight, srcFormat);
		}
		return result;
	}
	
	bool Image::fillRect(int x, int y, int w, int h, Color color, unsigned char* destData, int destWidth, int destHeight, Format destFormat)
	{
		x = hclamp(x, 0, destWidth - 1);
		y = hclamp(y, 0, destHeight - 1);
		w = hclamp(w, 1, destWidth - x);
		h = hclamp(h, 1, destHeight - y);
		if (w < 1 || h < 1)
		{
			return false;
		}
		int destBpp = Image::getFormatBpp(destFormat);
		int i = (x + y * destWidth) * destBpp;
		int copyWidth = w * destBpp;
		int size = copyWidth * destHeight;
		if (destBpp == 1 || destBpp == 3 && color.r == color.g && color.r == color.g || destBpp == 4 && color.r == color.g && color.r == color.b && color.r == color.a)
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
			return true;
		}
		unsigned char colorData[4] = {color.r, color.g, color.b, color.a};
		// convert to right format first
		Format srcFormat = (destBpp == 4 ? FORMAT_RGBA : (destBpp == 3 ? FORMAT_RGB : FORMAT_ALPHA));
		if (srcFormat != destFormat && destBpp > 1)
		{
			unsigned char* rgba = NULL;
			if (!Image::convertToFormat(1, 1, colorData, srcFormat, &rgba, destFormat))
			{
				return false;
			}
			memcpy(&destData[i], rgba, destBpp);
			delete [] rgba;
		}
		else
		{
			memcpy(&destData[i], colorData, destBpp);
		}
		int currentSize = destBpp;
		int copySize = 0;
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
		return true;
	}
	
	bool Image::write(int sx, int sy, int sw, int sh, int dx, int dy, unsigned char* srcData, int srcWidth, int srcHeight, Format srcFormat,
		unsigned char* destData, int destWidth, int destHeight, Format destFormat)
	{
		dx = hclamp(dx, 0, destWidth - 1);
		dy = hclamp(dy, 0, destHeight - 1);
		sx = hclamp(sx, 0, srcWidth - 1);
		sy = hclamp(sy, 0, srcHeight - 1);
		sw = hmin(sw, hmin(destWidth - dx, srcWidth - sx));
		sh = hmin(sh, hmin(destHeight - dy, srcHeight - sy));
		if (sw < 1 || sh < 1)
		{
			return false;
		}
		int srcBpp = Image::getFormatBpp(srcFormat);
		int destBpp = Image::getFormatBpp(destFormat);
		unsigned char* p = &destData[(dx + dy * destWidth) * destBpp];
		if (sx == 0 && dx == 0 && srcWidth == destWidth && sw == destWidth)
		{
			Image::convertToFormat(sw, sh, &srcData[(sx + sy * srcWidth) * srcBpp], srcFormat, &p, destFormat, false);
		}
		else
		{
			for_iter (j, 0, sh)
			{
				Image::convertToFormat(sw, 1, &srcData[(sx + (sy + j) * srcWidth) * srcBpp], srcFormat, &p, destFormat, false);
				p += destWidth * destBpp;
			}
		}
		return true;
	}

	// TODOaa - blit goes here

	// TODOaa - stretchBlit goes here

	bool Image::insertAlphaMap(int w, int h, unsigned char* srcData, Image::Format srcFormat, unsigned char* destData, Image::Format destFormat)
	{
		if (!CHECK_ALPHA_FORMAT(destFormat)) // not a format that supports an alpha channel
		{
			return false;
		}
		int srcBpp = Image::getFormatBpp(srcFormat);
		int destBpp = Image::getFormatBpp(destFormat);
		int destAlpha = CHECK_LEFT_RGB(destFormat) ? 3 : 0;
		int i = 0;
		int srcIndex = 0;
		if (srcBpp > 1)
		{
			srcIndex = 1;
		}
		if (srcBpp == 1 || srcBpp == 3 || srcBpp == 4)
		{
			for_iter (y, 0, h)
			{
				for_iter (x, 0, w)
				{
					i = (x + y * w);
					// takes second color channel for alpha value, guaranteed to always be either R, G or B, but never A
					destData[i * destBpp + destAlpha] = srcData[i * srcBpp + srcIndex];
				}
			}
			return true;
		}
		return false;
	}
	
	bool Image::convertToFormat(int w, int h, unsigned char* srcData, Image::Format srcFormat, unsigned char** destData, Image::Format destFormat, bool preventCopy)
	{
		if (preventCopy && srcFormat == destFormat)
		{
			hlog::warn(april::logTag, "The source's and destination's formats are the same!");
			return false;
		}
		int srcBpp = Image::getFormatBpp(srcFormat);
		if (srcBpp == 1)
		{
			if (Image::_convertFrom1Bpp(w, h, srcData, srcFormat, destData, destFormat))
			{
				return true;
			}
		}
		else if (srcBpp == 3)
		{
			if (Image::_convertFrom3Bpp(w, h, srcData, srcFormat, destData, destFormat))
			{
				return true;
			}
		}
		else if (srcBpp == 4)
		{
			if (Image::_convertFrom4Bpp(w, h, srcData, srcFormat, destData, destFormat))
			{
				return true;
			}
		}
		hlog::errorf(april::logTag, "Conversion from %d BPP to %d BPP is not supported!", srcBpp, Image::getFormatBpp(destFormat));
		return false;
	}

	bool Image::_convertFrom1Bpp(int w, int h, unsigned char* srcData, Format srcFormat, unsigned char** destData, Format destFormat)
	{
		static int srcBpp = 1;
		int destBpp = Image::getFormatBpp(destFormat);
		bool createData = (*destData == NULL);
		if (createData)
		{
			*destData = new unsigned char[w * h * destBpp];
		}
		if (destBpp == 1)
		{
			memcpy(*destData, srcData, w * h * destBpp);
			return true;
		}
		if (destBpp == 3 || destBpp == 4)
		{
			int i = 0;
			if (destBpp > 3)
			{
				memset(*destData, 255, w * h * destBpp);
				if (!CHECK_LEFT_RGB(destFormat))
				{
					for_iter (y, 0, h)
					{
						for_iter (x, 0, w)
						{
							i = (x + y * w) * destBpp;
							(*destData)[i + 1] = (*destData)[i + 2] = (*destData)[i + 3] = srcData[x + y * w];
						}
					}
					return true;
				}
			}
			for_iter (y, 0, h)
			{
				for_iter (x, 0, w)
				{
					i = (x + y * w) * destBpp;
					(*destData)[i] = (*destData)[i + 1] = (*destData)[i + 2] = srcData[x + y * w];
				}
			}
			return true;
		}
		if (createData)
		{
			delete *destData;
			*destData = NULL;
		}
		return false;
	}

	bool Image::_convertFrom3Bpp(int w, int h, unsigned char* srcData, Format srcFormat, unsigned char** destData, Format destFormat)
	{
		static int srcBpp = 3;
		int destBpp = Image::getFormatBpp(destFormat);
		bool createData = (*destData == NULL);
		if (createData)
		{
			*destData = new unsigned char[w * h * destBpp];
		}
		if (destBpp == 1)
		{
			int i = (srcFormat == FORMAT_RGB ? 0 : 2);
			for_iter (y, 0, h)
			{
				for_iter (x, 0, w)
				{
					// red is used as main component
					(*destData)[x + y * w] = srcData[(x + y * w) * srcBpp + i];
				}
			}
			return true;
		}
		if (destBpp == 3)
		{
			memcpy(*destData, srcData, w * h * destBpp);
			// FORMAT_RGB to FORMAT_BGR and vice versa, thus switching 2 bytes around is enough
			if (srcFormat != destFormat)
			{
				int i = 0;
				for_iter (y, 0, h)
				{
					for_iter (x, 0, w)
					{
						i = (x + y * w) * destBpp;
						(*destData)[i] = srcData[i + 2];
						(*destData)[i + 2] = srcData[i];
					}
				}
			}
			return true;
		}
		if (destBpp == 4)
		{
			unsigned int* dest = (unsigned int*)*destData;
			Format extended = (srcFormat == FORMAT_RGB ? FORMAT_RGBX : FORMAT_BGRX);
			bool rightShift = CHECK_SHIFT_FORMATS(extended, destFormat);
			bool invertOrder = (CHECK_INVERT_ORDER_FORMATS(extended, destFormat) || CHECK_INVERT_ORDER_FORMATS(destFormat, extended));
			int i = 0;
			if (rightShift)
			{
				if (invertOrder)
				{
					FOR_EACH_3BPP_TO_4BPP_PIXEL((((unsigned int)srcData[i]) << 24) | (((unsigned int)srcData[i + 1]) << 16) | (((unsigned int)srcData[i + 2]) << 8) | _R_ALPHA);
				}
				else
				{
					FOR_EACH_3BPP_TO_4BPP_PIXEL((((unsigned int)srcData[i]) << 8) | (((unsigned int)srcData[i + 1]) << 16) | (((unsigned int)srcData[i + 2]) << 24) | _R_ALPHA);
				}
			}
			else if (invertOrder)
			{
				FOR_EACH_3BPP_TO_4BPP_PIXEL((((unsigned int)srcData[i]) << 16) | (((unsigned int)srcData[i + 1]) << 8) | srcData[i + 2] | _L_ALPHA);
			}
			else
			{
				FOR_EACH_3BPP_TO_4BPP_PIXEL(srcData[i] | (((unsigned int)srcData[i + 1]) << 8) | (((unsigned int)srcData[i + 2]) << 16) | _L_ALPHA);
			}
			return true;
		}
		if (createData)
		{
			delete *destData;
			*destData = NULL;
		}
		return false;
	}

	bool Image::_convertFrom4Bpp(int w, int h, unsigned char* srcData, Format srcFormat, unsigned char** destData, Format destFormat)
	{
		static int srcBpp = 4;
		int destBpp = Image::getFormatBpp(destFormat);
		bool createData = (*destData == NULL);
		if (createData)
		{
			*destData = new unsigned char[w * h * destBpp];
		}
		if (destBpp == 1)
		{
			int i = 0;
			if (srcFormat == FORMAT_ARGB || srcFormat == FORMAT_XRGB)
			{
				i = 1;
			}
			if (srcFormat == FORMAT_BGRA || srcFormat == FORMAT_BGRX)
			{
				i = 2;
			}
			if (srcFormat == FORMAT_ABGR || srcFormat == FORMAT_XBGR)
			{
				i = 3;
			}
			for_iter (y, 0, h)
			{
				for_iter (x, 0, w)
				{
					// red is used as main component
					(*destData)[x + y * w] = srcData[(x + y * w) * srcBpp + i];
				}
			}
			return true;
		}
		if (destBpp == 3)
		{
			unsigned int* src = (unsigned int*)srcData;
			unsigned char* dest = *destData;
			Format extended = (destFormat == FORMAT_RGB ? FORMAT_RGBX : FORMAT_BGRX);
			bool leftShift = CHECK_SHIFT_FORMATS(extended, srcFormat);
			bool invertOrder = (CHECK_INVERT_ORDER_FORMATS(srcFormat, extended) || CHECK_INVERT_ORDER_FORMATS(extended, srcFormat));
			int i = 0;
			int j = 0;
			if (leftShift)
			{
				if (invertOrder)
				{
					FOR_EACH_4BPP_TO_3BPP_PIXEL(src[i] >> 24, src[i] >> 16, src[i] >> 8);
				}
				else
				{
					FOR_EACH_4BPP_TO_3BPP_PIXEL(src[i] >> 8, src[i] >> 16, src[i] >> 24);
				}
			}
			else if (invertOrder)
			{
				FOR_EACH_4BPP_TO_3BPP_PIXEL(src[i] >> 16, src[i] >> 8, src[i]);
			}
			else
			{
				FOR_EACH_4BPP_TO_3BPP_PIXEL(src[i], src[i] >> 8, src[i] >> 16);
			}
			return true;
		}
		if (destBpp == 4)
		{
			// shifting unsigned int's around is faster than pure assigning (like at 3 BPP)
			unsigned int* src = (unsigned int*)srcData;
			unsigned int* dest = (unsigned int*)*destData;
			bool rightShift = CHECK_SHIFT_FORMATS(srcFormat, destFormat);
			bool leftShift = CHECK_SHIFT_FORMATS(destFormat, srcFormat);
			bool invertOrder = (CHECK_INVERT_ORDER_FORMATS(srcFormat, destFormat) || CHECK_INVERT_ORDER_FORMATS(destFormat, srcFormat));
			bool left = CHECK_LEFT_RGB(destFormat);
			bool invertOrderL = (invertOrder && left);
			bool invertOrderR = (invertOrder && !left);
			bool srcAlpha = CHECK_ALPHA_FORMAT(srcFormat);
			bool destAlpha = CHECK_ALPHA_FORMAT(destFormat);
			bool copyAlpha = (srcAlpha && destAlpha);
			int i = 0;
			if (rightShift)
			{
				if (invertOrder)
				{
					if (copyAlpha)
					{
						FOR_EACH_4BPP_PIXEL(INVERTED_RIGHT_SHIFT_WITH_ALPHA);
					}
					else
					{
						FOR_EACH_4BPP_PIXEL(INVERTED_RIGHT_SHIFT);
					}
				}
				else if (copyAlpha)
				{
					FOR_EACH_4BPP_PIXEL(RIGHT_SHIFT_WITH_ALPHA);
				}
				else
				{
					FOR_EACH_4BPP_PIXEL(RIGHT_SHIFT);
				}
			}
			else if (leftShift)
			{
				if (invertOrder)
				{
					if (copyAlpha)
					{
						FOR_EACH_4BPP_PIXEL(INVERTED_LEFT_SHIFT_WITH_ALPHA);
					}
					else
					{
						FOR_EACH_4BPP_PIXEL(INVERTED_LEFT_SHIFT);
					}
				}
				else if (copyAlpha)
				{
					FOR_EACH_4BPP_PIXEL(LEFT_SHIFT_WITH_ALPHA);
				}
				else
				{
					FOR_EACH_4BPP_PIXEL(LEFT_SHIFT);
				}
			}
			else if (invertOrderL)
			{
				if (copyAlpha)
				{
					FOR_EACH_4BPP_PIXEL(INVERTED_L_WITH_ALPHA);
				}
				else
				{
					FOR_EACH_4BPP_PIXEL(INVERTED_L);
				}
			}
			else if (invertOrderR)
			{
				if (copyAlpha)
				{
					FOR_EACH_4BPP_PIXEL(INVERTED_R_WITH_ALPHA);
				}
				else
				{
					FOR_EACH_4BPP_PIXEL(INVERTED_R);
				}
			}
			else if (srcAlpha ^ destAlpha)
			{
				if (left)
				{
					FOR_EACH_4BPP_PIXEL(KEEP_L);
				}
				else
				{
					FOR_EACH_4BPP_PIXEL(KEEP_R);
				}
			}
			else
			{
				memcpy(*destData, srcData, w * h * destBpp);
			}
			return true;
		}
		if (createData)
		{
			delete *destData;
			*destData = NULL;
		}
		return false;
	}

	bool Image::needsConversion(Format srcFormat, Format destFormat, bool preventCopy)
	{
		if (preventCopy && srcFormat == destFormat)
		{
			return false;
		}
		int srcBpp = Image::getFormatBpp(srcFormat);
		int destBpp = Image::getFormatBpp(destFormat);
		if (srcBpp != destBpp)
		{
			return true;
		}
		if (srcBpp != 4)
		{
			return false;
		}
		if (CHECK_SHIFT_FORMATS(srcFormat, destFormat))
		{
			return true;
		}
		if (CHECK_SHIFT_FORMATS(destFormat, srcFormat))
		{
			return true;
		}
		if (CHECK_INVERT_ORDER_FORMATS(srcFormat, destFormat) || CHECK_INVERT_ORDER_FORMATS(destFormat, srcFormat))
		{
			return true;
		}
		if (CHECK_ALPHA_FORMAT(destFormat))
		{
			return true;
		}
		return false;
	}

}
