/// @file
/// @version 4.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <string.h>

#include <hltypes/hfile.h>
#include <hltypes/hlog.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hmap.h>
#include <hltypes/hresource.h>
#include <hltypes/hstring.h>

#include "april.h"
#include "Color.h"
#include "Image.h"
#include "RenderSystem.h"

#ifdef __APPLE__
#include <TargetConditionals.h>
#endif

// reguired, because some system headers have this defined
#ifdef RGB
#undef RGB
#endif

#define MAX_WRITE_SIZE 65536

#define CHECK_SHIFT_FORMATS(format1, format2) (\
	((format1) == Format::RGBA || (format1) == Format::RGBX || (format1) == Format::BGRA || (format1) == Format::BGRX) && \
	((format2) == Format::ARGB || (format2) == Format::XRGB || (format2) == Format::ABGR || (format2) == Format::XBGR) \
)
#define CHECK_INVERT_ORDER_FORMATS(format1, format2) (\
	((format1) == Format::RGBA || (format1) == Format::RGBX || (format1) == Format::ARGB || (format1) == Format::XRGB) && \
	((format2) == Format::BGRA || (format2) == Format::BGRX || (format2) == Format::ABGR || (format2) == Format::XBGR) \
)
#define CHECK_LEFT_RGB(format) \
	((format) == Format::RGBA || (format) == Format::RGBX || (format) == Format::BGRA || (format) == Format::BGRX)
#define CHECK_ALPHA_FORMAT(format) \
	((format) == Format::RGBA || (format) == Format::ARGB || (format) == Format::BGRA || (format) == Format::ABGR)

#define FOR_EACH_4BPP_PIXEL(macro) \
	for_iterx (y, 0, h) \
	{ \
		for_iterx (x, 0, w) \
		{ \
			i = (x + y * w); \
			dest[i] = macro(src[i]); \
		} \
	}
#define FOR_EACH_3BPP_TO_4BPP_PIXEL(exec) \
	for_iterx (y, 0, h) \
	{ \
		for_iterx (x, 0, w) \
		{ \
			i = (x + y * w) * srcBpp; \
			dest[x + y * w] = (exec); \
		} \
	}
#define FOR_EACH_4BPP_TO_3BPP_PIXEL(exec1, exec2, exec3) \
	for_iterx (y, 0, h) \
	{ \
		for_iterx (x, 0, w) \
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

#define HROUND_GRECT(rect) hround(rect.x), hround(rect.y), hround(rect.w), hround(rect.h)
#define HROUND_GVEC2(vec2) hround(vec2.x), hround(vec2.y)

namespace april
{
	HL_ENUM_CLASS_DEFINE(Image::Format,
	(
		HL_ENUM_DEFINE(Image::Format, Invalid);
		HL_ENUM_DEFINE(Image::Format, RGBA);
		HL_ENUM_DEFINE(Image::Format, ARGB);
		HL_ENUM_DEFINE(Image::Format, BGRA);
		HL_ENUM_DEFINE(Image::Format, ABGR);
		HL_ENUM_DEFINE(Image::Format, RGBX);
		HL_ENUM_DEFINE(Image::Format, XRGB);
		HL_ENUM_DEFINE(Image::Format, BGRX);
		HL_ENUM_DEFINE(Image::Format, XBGR);
		HL_ENUM_DEFINE(Image::Format, RGB);
		HL_ENUM_DEFINE(Image::Format, BGR);
		HL_ENUM_DEFINE(Image::Format, Alpha);
		HL_ENUM_DEFINE(Image::Format, Greyscale);
		HL_ENUM_DEFINE(Image::Format, Compressed);
		HL_ENUM_DEFINE(Image::Format, Palette);

		int Image::Format::getBpp() const
		{
			if ((*this) == RGBA)		return 4;
			if ((*this) == ARGB)		return 4;
			if ((*this) == BGRA)		return 4;
			if ((*this) == ABGR)		return 4;
			if ((*this) == RGBX)		return 4;
			if ((*this) == XRGB)		return 4;
			if ((*this) == BGRX)		return 4;
			if ((*this) == XBGR)		return 4;
			if ((*this) == RGB)			return 3;
			if ((*this) == BGR)			return 3;
			if ((*this) == Alpha)		return 1;
			if ((*this) == Greyscale)	return 1;
			return 0;
		}

		int Image::Format::getIndexRed() const
		{
			if ((*this) == RGBA || (*this) == RGBX || (*this) == RGB || (*this) == Greyscale)	return 0;
			if ((*this) == ARGB || (*this) == XRGB)												return 1;
			if ((*this) == BGRA || (*this) == BGRX || (*this) == BGR)							return 2;
			if ((*this) == ABGR || (*this) == XBGR)												return 3;
			return -1;
		}

		int Image::Format::getIndexGreen() const
		{
			if ((*this) == Greyscale)																		return 0;
			if ((*this) == BGRA || (*this) == RGBX || (*this) == BGRX || (*this) == RGB || (*this) == BGR)	return 1;
			if ((*this) == ARGB || (*this) == ABGR || (*this) == XRGB || (*this) == XBGR)					return 2;
			return -1;
		}

		int Image::Format::getIndexBlue() const
		{
			if ((*this) == BGRA || (*this) == BGRX || (*this) == BGR || (*this) == Greyscale)	return 0;
			if ((*this) == ABGR || (*this) == XBGR)												return 1;
			if ((*this) == RGBA || (*this) == RGBX || (*this) == RGB)							return 2;
			if ((*this) == ARGB || (*this) == XRGB)												return 3;
			return -1;
		}

		int Image::Format::getIndexAlpha() const
		{
			if ((*this) == ARGB || (*this) == ABGR || (*this) == Alpha)	return 0;
			if ((*this) == RGBA || (*this) == BGRA)						return 3;
			return -1;
		}

		void Image::Format::getChannelIndices(int* red, int* green, int* blue, int* alpha) const
		{
			if ((*this) == RGBA || (*this) == RGBX)
			{
				if (red != NULL)	*red = 0;
				if (green != NULL)	*green = 1;
				if (blue != NULL)	*blue = 2;
				if (alpha != NULL)	*alpha = 3;
			}
			else if ((*this) == BGRA || (*this) == BGRX)
			{
				if (red != NULL)	*red = 2;
				if (green != NULL)	*green = 1;
				if (blue != NULL)	*blue = 0;
				if (alpha != NULL)	*alpha = 3;
			}
			else if ((*this) == ARGB || (*this) == XRGB)
			{
				if (red != NULL)	*red = 1;
				if (green != NULL)	*green = 2;
				if (blue != NULL)	*blue = 3;
				if (alpha != NULL)	*alpha = 0;
			}
			else if ((*this) == ABGR || (*this) == XBGR)
			{
				if (red != NULL)	*red = 3;
				if (green != NULL)	*green = 2;
				if (blue != NULL)	*blue = 1;
				if (alpha != NULL)	*alpha = 0;
			}
			else if ((*this) == RGB)
			{
				if (red != NULL)	*red = 0;
				if (green != NULL)	*green = 1;
				if (blue != NULL)	*blue = 2;
				if (alpha != NULL)	*alpha = -1;
			}
			else if ((*this) == BGR)
			{
				if (red != NULL)	*red = 2;
				if (green != NULL)	*green = 1;
				if (blue != NULL)	*blue = 0;
				if (alpha != NULL)	*alpha = -1;
			}
			else if ((*this) == Alpha || (*this) == Greyscale)
			{
				if (red != NULL)	*red = 0;
				if (green != NULL)	*green = 0;
				if (blue != NULL)	*blue = 0;
				if (alpha != NULL)	*alpha = 0;
			}
			else
			{
				if (red != NULL)	*red = -1;
				if (green != NULL)	*green = -1;
				if (blue != NULL)	*blue = -1;
				if (alpha != NULL)	*alpha = -1;
			}
		}

	));

	HL_ENUM_CLASS_DEFINE(Image::FileFormat,
	(
		HL_ENUM_DEFINE(Image::FileFormat, Png);
		HL_ENUM_DEFINE(Image::FileFormat, Custom);
	));

	hmap<hstr, Image* (*)(hsbase&)> Image::customLoaders;
	hmap<hstr, Image* (*)(hsbase&)> Image::customMetaDataLoaders;
	hmap<hstr, bool (*)(hsbase&, Image*)> Image::customSavers;

	Image::Image()
	{
		this->data = NULL;
		this->w = 0;
		this->h = 0;
		this->format = Format::Invalid;
		this->internalFormat = 0;
		this->compressedSize = 0;
	}
	
	Image::Image(const Image& other)
	{
		this->data = NULL;
		this->w = 0;
		this->h = 0;
		this->format = Format::Invalid;
		this->internalFormat = 0;
		this->compressedSize = 0;
		hlog::error(logTag, "Creating april::Image instances using copy-constructor is not allowed! Use april::Image::create() instead.");
	}

	Image::~Image()
	{
		if (this->data != NULL)
		{
			delete[] this->data;
		}
	}

	int Image::getBpp() const
	{
		return this->format.getBpp();
	}

	int Image::getByteSize() const
	{
		return (this->w * this->h * this->format.getBpp());
	}

	bool Image::isValid() const
	{
		return (this->data != NULL && this->getByteSize() > 0);
	}

	bool Image::clear()
	{
		bool result = this->isValid();
		if (result)
		{
			memset(this->data, 0, this->getByteSize());
		}
		return (result);
	}

	Color Image::getPixel(int x, int y) const
	{
		return (this->isValid() ? Image::getPixel(x, y, this->data, this->w, this->h, this->format) : Color::Clear);
	}
	
	bool Image::setPixel(int x, int y, Color color)
	{
		return (this->isValid() && Image::setPixel(x, y, color, this->data, this->w, this->h, this->format));
	}
	
	Color Image::getInterpolatedPixel(float x, float y) const
	{
		return (this->isValid() ? Image::getInterpolatedPixel(x, y, this->data, this->w, this->h, this->format) : Color::Clear);
	}
	
	bool Image::fillRect(int x, int y, int w, int h, Color color)
	{
		return (this->isValid() && Image::fillRect(x, y, w, h, color, this->data, this->w, this->h, this->format));
	}

	bool Image::copyPixelData(unsigned char** output, Format format) const
	{
		return (this->isValid() && Image::convertToFormat(this->w, this->h, this->data, this->format, output, format, false));
	}

	bool Image::write(int sx, int sy, int sw, int sh, int dx, int dy, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat)
	{
		return (this->isValid() && Image::write(sx, sy, sw, sh, dx, dy, srcData, srcWidth, srcHeight, srcFormat, this->data, this->w, this->h, this->format));
	}

	bool Image::writeStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat)
	{
		return (this->isValid() && Image::writeStretch(sx, sy, sw, sh, dx, dy, dw, dh, srcData, srcWidth, srcHeight, srcFormat, this->data, this->w, this->h, this->format));
	}

	bool Image::blit(int sx, int sy, int sw, int sh, int dx, int dy, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat, unsigned char alpha)
	{
		return (this->isValid() && Image::blit(sx, sy, sw, sh, dx, dy, srcData, srcWidth, srcHeight, srcFormat, this->data, this->w, this->h, this->format, alpha));
	}

	bool Image::blitStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat, unsigned char alpha)
	{
		return (this->isValid() && Image::blitStretch(sx, sy, sw, sh, dx, dy, dw, dh, srcData, srcWidth, srcHeight, srcFormat, this->data, this->w, this->h, this->format, alpha));
	}

	bool Image::rotateHue(int x, int y, int w, int h, float degrees)
	{
		return (this->isValid() && Image::rotateHue(x, y, w, h, degrees, this->data, this->w, this->h, this->format));
	}

	bool Image::saturate(int x, int y, int w, int h, float factor)
	{
		return (this->isValid() && Image::saturate(x, y, w, h, factor, this->data, this->w, this->h, this->format));
	}

	bool Image::insertAlphaMap(unsigned char* srcData, Format srcFormat, unsigned char median, int ambiguity)
	{
		return (this->isValid() && Image::insertAlphaMap(this->w, this->h, srcData, srcFormat, this->data, this->format, median, ambiguity));
	}

	bool Image::dilate(unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat)
	{
		return (this->isValid() && Image::dilate(srcData, srcWidth, srcHeight, srcFormat, this->data, this->w, this->h, this->format));
	}

	Image* Image::extractRed() const
	{
		return this->extractColor(this->format.getIndexRed());
	}

	Image* Image::extractGreen() const
	{
		return this->extractColor(this->format.getIndexGreen());
	}

	Image* Image::extractBlue() const
	{
		return this->extractColor(this->format.getIndexBlue());
	}

	Image* Image::extractAlpha() const
	{
		if (!CHECK_ALPHA_FORMAT(this->format) && this->format != Format::Alpha && this->format != Format::Compressed && this->format != Format::Palette)
		{
			return Image::create(this->w, this->h, april::Color::White, Format::Alpha);
		}
		return this->extractColor(this->format.getIndexAlpha());
	}

	Image* Image::extractColor(int index) const
	{
		if (index < 0)
		{
			return NULL;
		}
		int srcBpp = this->format.getBpp();
		if (index >= srcBpp)
		{
			return NULL;
		}
		Image* image = Image::create(this->w, this->h, april::Color::Clear, Format::Alpha);
		if (srcBpp == 1)
		{
			memcpy(image->data, this->data, this->w * this->h);
			return image;
		}
		if (srcBpp == 3 || srcBpp == 4)
		{
			int x = 0;
			int y = 0;
			for_iterx (y, 0, this->h)
			{
				for_iterx (x, 0, this->w)
				{
					// red is used as main component
					image->data[x + y * this->w] = this->data[(x + y * this->w) * srcBpp + index];
				}
			}
			return image;
		}
		delete image;
		return NULL;
	}

	// overloads

	Color Image::getPixel(cgvec2 position) const
	{
		return this->getPixel(hround(position.x), hround(position.y));
	}
	
	bool Image::setPixel(cgvec2 position, const Color& color)
	{
		return this->setPixel(hround(position.x), hround(position.y), color);
	}
	
	Color Image::getInterpolatedPixel(cgvec2 position) const
	{
		return this->getInterpolatedPixel(position.x, position.y);
	}
	
	bool Image::copyPixelData(unsigned char** output) const
	{
		return (this->data != NULL && Image::convertToFormat(this->w, this->h, this->data, this->format, output, this->format, false));
	}

	bool Image::fillRect(cgrect rect, const Color& color)
	{
		return this->fillRect(HROUND_GRECT(rect), color);
	}

	bool Image::write(int sx, int sy, int sw, int sh, int dx, int dy, Image* other)
	{
		return this->write(sx, sy, sw, sh, dx, dy, other->data, other->w, other->h, other->format);
	}

	bool Image::write(cgrect srcRect, cgvec2 destPosition, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat)
	{
		return this->write(HROUND_GRECT(srcRect), HROUND_GVEC2(destPosition), srcData, srcWidth, srcHeight, srcFormat);
	}

	bool Image::write(cgrect srcRect, cgvec2 destPosition, Image* other)
	{
		return this->write(HROUND_GRECT(srcRect), HROUND_GVEC2(destPosition), other->data, other->w, other->h, other->format);
	}

	bool Image::writeStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, Image* other)
	{
		return this->writeStretch(sx, sy, sw, sh, dx, dy, dw, dh, other->data, other->w, other->h, other->format);
	}

	bool Image::writeStretch(cgrect srcRect, cgrect destRect, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat)
	{
		return this->writeStretch(HROUND_GRECT(srcRect), HROUND_GRECT(destRect), srcData, srcWidth, srcHeight, srcFormat);
	}

	bool Image::writeStretch(cgrect srcRect, cgrect destRect, Image* other)
	{
		return this->writeStretch(HROUND_GRECT(srcRect), HROUND_GRECT(destRect), other->data, other->w, other->h, other->format);
	}

	bool Image::blit(int sx, int sy, int sw, int sh, int dx, int dy, Image* other, unsigned char alpha)
	{
		return this->blit(sx, sy, sw, sh, dx, dy, other->data, other->w, other->h, other->format, alpha);
	}

	bool Image::blit(cgrect srcRect, cgvec2 destPosition, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat, unsigned char alpha)
	{
		return this->blit(HROUND_GRECT(srcRect), HROUND_GVEC2(destPosition), srcData, srcWidth, srcHeight, srcFormat);
	}

	bool Image::blit(cgrect srcRect, cgvec2 destPosition, Image* other, unsigned char alpha)
	{
		return this->blit(HROUND_GRECT(srcRect), HROUND_GVEC2(destPosition), other->data, other->w, other->h, other->format, alpha);
	}

	bool Image::blitStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, Image* other, unsigned char alpha)
	{
		return this->blitStretch(sx, sy, sw, sh, dx, dy, dw, dh, other->data, other->w, other->h, other->format, alpha);
	}

	bool Image::blitStretch(cgrect srcRect, cgrect destRect, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat, unsigned char alpha)
	{
		return this->blitStretch(HROUND_GRECT(srcRect), HROUND_GRECT(destRect), srcData, srcWidth, srcHeight, srcFormat);
	}

	bool Image::blitStretch(cgrect srcRect, cgrect destRect, Image* other, unsigned char alpha)
	{
		return this->blitStretch(HROUND_GRECT(srcRect), HROUND_GRECT(destRect), other->data, other->w, other->h, other->format, alpha);
	}

	bool Image::rotateHue(cgrect rect, float degrees)
	{
		return this->rotateHue(HROUND_GRECT(rect), degrees);
	}

	bool Image::saturate(cgrect rect, float factor)
	{
		return this->saturate(HROUND_GRECT(rect), factor);
	}

	bool Image::insertAlphaMap(unsigned char* srcData, Format srcFormat)
	{
		return (this->insertAlphaMap(srcData, srcFormat, 0, 0));
	}

	bool Image::insertAlphaMap(Image* image, unsigned char median, int ambiguity)
	{
		return this->insertAlphaMap(image->data, image->format, median, ambiguity);
	}

	bool Image::insertAlphaMap(Image* image)
	{
		return this->insertAlphaMap(image->data, image->format, 0, 0);
	}

	bool Image::dilate(Image* image)
	{
		return this->dilate(image->data, image->w, image->h, image->format);
	}

	// loading/creating functions

	Image* Image::createFromResource(chstr filename)
	{
		hresource file;
		if (filename.lowered().endsWith(".png"))
		{
			file.open(filename);
			return Image::_loadPng(file);
		}
		if (filename.lowered().endsWith(".jpg") || filename.lowered().endsWith(".jpeg"))
		{
			file.open(filename);
			return Image::_loadJpg(file);
		}
		if (filename.lowered().endsWith(".jpt"))
		{
			file.open(filename);
			return Image::_loadJpt(file);
		}
#ifdef _IMAGE_PVR
		if (filename.lowered().endsWith(".pvr"))
		{
			file.open(filename);
			return Image::_loadPvr(file);
		}
		if (filename.lowered().endsWith(".pvrz"))
		{
			file.open(filename);
			return Image::_loadPvrz(file);
		}
#endif
#ifdef _ANDROID
		if (filename.lowered().endsWith(".etcx"))
		{
			file.open(filename);
			return Image::_loadEtcx(file);
		}
#endif
		foreach_m (Image* (*)(hsbase&), it, Image::customLoaders)
		{
			if (filename.lowered().endsWith(it->first.lowered()))
			{
				file.open(filename);
				return (*it->second)(file);
			}
		}
		return NULL;
	}

	Image* Image::createFromResource(chstr filename, Image::Format format)
	{
		Image* image = Image::createFromResource(filename);
		if (image != NULL && Image::needsConversion(image->format, format))
		{
			unsigned char* data = NULL;
			if (Image::convertToFormat(image->w, image->h, image->data, image->format, &data, format))
			{
				delete[] image->data;
				image->format = format;
				image->data = data;
			}
		}
		return image;
	}

	Image* Image::createFromFile(chstr filename)
	{
		hfile file;
		if (filename.lowered().endsWith(".png"))
		{
			file.open(filename);
			return Image::_loadPng(file);
		}
		if (filename.lowered().endsWith(".jpg") || filename.lowered().endsWith(".jpeg"))
		{
			file.open(filename);
			return Image::_loadJpg(file);
		}
		if (filename.lowered().endsWith(".jpt"))
		{
			file.open(filename);
			return Image::_loadJpt(file);
		}
#ifdef _IMAGE_PVR
		if (filename.lowered().endsWith(".pvr"))
		{
			file.open(filename);
			return Image::_loadPvr(file);
		}
		if (filename.lowered().endsWith(".pvrz"))
		{
			file.open(filename);
			return Image::_loadPvrz(file);
		}
#endif
#ifdef _ANDROID
		if (filename.lowered().endsWith(".etcx"))
		{
			file.open(filename);
			return Image::_loadEtcx(file);
		}
#endif
		foreach_m (Image* (*)(hsbase&), it, Image::customLoaders)
		{
			if (filename.lowered().endsWith(it->first.lowered()))
			{
				file.open(filename);
				return (*it->second)(file);
			}
		}
		return NULL;
	}

	Image* Image::createFromFile(chstr filename, Image::Format format)
	{
		Image* image = Image::createFromFile(filename);
		if (image != NULL && Image::needsConversion(image->format, format))
		{
			unsigned char* data = NULL;
			if (Image::convertToFormat(image->w, image->h, image->data, image->format, &data, format))
			{
				delete[] image->data;
				image->format = format;
				image->data = data;
			}
		}
		return image;
	}

	Image* Image::createFromStream(hsbase& stream, chstr logicalExtension)
	{
		if (logicalExtension.lowered().endsWith(".png"))
		{
			return Image::_loadPng(stream);
		}
		if (logicalExtension.lowered().endsWith(".jpg") || logicalExtension.lowered().endsWith(".jpeg"))
		{
			return Image::_loadJpg(stream);
		}
		if (logicalExtension.lowered().endsWith(".jpt"))
		{
			return Image::_loadJpt(stream);
		}
#ifdef _IMAGE_PVR
		if (logicalExtension.lowered().endsWith(".pvr"))
		{
			return Image::_loadPvr(stream);
		}
		if (logicalExtension.lowered().endsWith(".pvrz"))
		{
			return Image::_loadPvrz(stream);
		}
#endif
#ifdef _ANDROID
		if (logicalExtension.lowered().endsWith(".etcx"))
		{
			return Image::_loadEtcx(stream);
		}
#endif
		foreach_m (Image* (*)(hsbase&), it, Image::customLoaders)
		{
			if (logicalExtension.lowered().endsWith(it->first.lowered()))
			{
				return (*it->second)(stream);
			}
		}
		return NULL;
	}

	Image* Image::createFromStream(hsbase& stream, chstr logicalExtension, Image::Format format)
	{
		Image* image = Image::createFromStream(stream, logicalExtension);
		if (image != NULL && Image::needsConversion(image->format, format))
		{
			unsigned char* data = NULL;
			if (Image::convertToFormat(image->w, image->h, image->data, image->format, &data, format))
			{
				delete[] image->data;
				image->format = format;
				image->data = data;
			}
		}
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
		image->data = NULL;
		if (data != NULL && size > 0)
		{
			image->data = new unsigned char[size];
			memcpy(image->data, data, size);
		}
		return image;
	}

	Image* Image::create(int w, int h, const Color& color, Image::Format format)
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
		image->data = NULL;
		if (other->data != NULL)
		{
			if (size == 0 && image->compressedSize > 0)
			{
				size = image->compressedSize;
			}
			if (size > 0)
			{
				image->data = new unsigned char[size];
				memcpy(image->data, other->data, size);
			}
		}
		return image;
	}

	bool Image::save(Image* image, chstr filename, Image::FileFormat format, chstr customExtension)
	{
		if (format == FileFormat::Png)
		{
			hfile file;
			file.open(filename, hfaccess::Write);
			return Image::_savePng(file, image);
		}
		if (format == FileFormat::Custom && Image::customSavers.hasKey(customExtension))
		{
			hfile file;
			file.open(filename, hfaccess::Write);
			return (*Image::customSavers[customExtension])(file, image);
		}
		return false;
	}

	Image* Image::readMetaDataFromResource(chstr filename)
	{
		hresource file;
		if (filename.lowered().endsWith(".png"))
		{
			file.open(filename);
			return Image::_readMetaDataPng(file);
		}
		if (filename.lowered().endsWith(".jpg") || filename.lowered().endsWith(".jpeg"))
		{
			file.open(filename);
			return Image::_readMetaDataJpg(file);
		}
		if (filename.lowered().endsWith(".jpt"))
		{
			file.open(filename);
			return Image::_readMetaDataJpt(file);
		}
#ifdef _IMAGE_PVR
		if (filename.lowered().endsWith(".pvr"))
		{
			file.open(filename);
			return Image::_readMetaDataPvr(file);
		}
		if (filename.lowered().endsWith(".pvrz"))
		{
			file.open(filename);
			return Image::_readMetaDataPvrz(file);
		}
#endif
#ifdef _ANDROID
		if (filename.lowered().endsWith(".etcx"))
		{
			file.open(filename);
			return Image::_readMetaDataEtcx(file);
		}
#endif
		foreach_m (Image* (*)(hsbase&), it, Image::customMetaDataLoaders)
		{
			if (filename.lowered().endsWith(it->first.lowered()))
			{
				file.open(filename);
				return (*it->second)(file);
			}
		}
		return NULL;
	}

	Image* Image::readMetaDataFromFile(chstr filename)
	{
		hfile file;
		if (filename.lowered().endsWith(".png"))
		{
			file.open(filename);
			return Image::_readMetaDataPng(file);
		}
		if (filename.lowered().endsWith(".jpg") || filename.lowered().endsWith(".jpeg"))
		{
			file.open(filename);
			return Image::_readMetaDataJpg(file);
		}
		if (filename.lowered().endsWith(".jpt"))
		{
			file.open(filename);
			return Image::_readMetaDataJpt(file);
		}
#ifdef _IMAGE_PVR
		if (filename.lowered().endsWith(".pvr"))
		{
			file.open(filename);
			return Image::_readMetaDataPvr(file);
		}
		if (filename.lowered().endsWith(".pvrz"))
		{
			file.open(filename);
			return Image::_readMetaDataPvrz(file);
		}
#endif
#ifdef _ANDROID
		if (filename.lowered().endsWith(".etcx"))
		{
			file.open(filename);
			return Image::_readMetaDataEtcx(file);
		}
#endif
		foreach_m (Image* (*)(hsbase&), it, Image::customMetaDataLoaders)
		{
			if (filename.lowered().endsWith(it->first.lowered()))
			{
				file.open(filename);
				return (*it->second)(file);
			}
		}
		return NULL;
	}

	Image* Image::readMetaDataFromStream(hsbase& stream, chstr logicalExtension)
	{
		if (logicalExtension.lowered().endsWith(".png"))
		{
			return Image::_readMetaDataPng(stream);
		}
		if (logicalExtension.lowered().endsWith(".jpg") || logicalExtension.lowered().endsWith(".jpeg"))
		{
			return Image::_readMetaDataJpg(stream);
		}
		if (logicalExtension.lowered().endsWith(".jpt"))
		{
			return Image::_readMetaDataJpt(stream);
		}
#ifdef _IMAGE_PVR
		if (logicalExtension.lowered().endsWith(".pvr"))
		{
			return Image::_readMetaDataPvr(stream);
		}
		if (logicalExtension.lowered().endsWith(".pvrz"))
		{
			return Image::_readMetaDataPvrz(stream);
		}
#endif
#ifdef _ANDROID
		if (logicalExtension.lowered().endsWith(".etcx"))
		{
			return Image::_readMetaDataEtcx(stream);
		}
#endif
		foreach_m (Image* (*)(hsbase&), it, Image::customMetaDataLoaders)
		{
			if (logicalExtension.lowered().endsWith(it->first.lowered()))
			{
				return (*it->second)(stream);
			}
		}
		return NULL;
	}

	// image data manipulation functions

	Color Image::getPixel(int x, int y, unsigned char* srcData, int srcWidth, int srcHeight, Format srcFormat)
	{
		Color color = Color::Clear;
		// has to create data, doesn't work with static unsigned char[4] for some reason
		unsigned char* rgba = NULL;
		if (Image::checkRect(x, y, srcWidth, srcHeight) && Image::convertToFormat(1, 1, &srcData[(x + y * srcWidth) * srcFormat.getBpp()], srcFormat, (unsigned char**)&rgba, Image::Format::RGBA, false))
		{
			color.r = rgba[0];
			color.g = rgba[1];
			color.b = rgba[2];
			color.a = rgba[3];
			delete[] rgba;
		}
		return color;
	}
	
	bool Image::setPixel(int x, int y, const Color& color, unsigned char* destData, int destWidth, int destHeight, Format destFormat)
	{
		if (!Image::checkRect(x, y, destWidth, destHeight))
		{
			return false;
		}
		unsigned char rgba[4] = {color.r, color.g, color.b, color.a};
		unsigned char* p = &destData[(x + y * destWidth) * destFormat.getBpp()];
		return Image::convertToFormat(1, 1, rgba, Image::Format::RGBA, &p, destFormat, false);
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

	bool Image::fillRect(int x, int y, int w, int h, const Color& color, unsigned char* destData, int destWidth, int destHeight, Format destFormat)
	{
		if (!Image::correctRect(x, y, w, h, destWidth, destHeight))
		{
			return false;
		}
		int destBpp = destFormat.getBpp();
		int i = (x + y * destWidth) * destBpp;
		int copyWidth = w * destBpp;
		int size = copyWidth * h;
		if (destBpp == 1 || (destBpp == 3 && color.r == color.g && color.r == color.b) || (destBpp == 4 && color.r == color.g && color.r == color.b && color.r == color.a))
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
		Format srcFormat = (destBpp == 4 ? Format::RGBA : (destBpp == 3 ? Format::RGB : Format::Greyscale));
		if (srcFormat != destFormat && destBpp > 1)
		{
			// has to create data, doesn't work with static unsigned char[4] for some reason
			unsigned char* rgba = NULL;
			if (!Image::convertToFormat(1, 1, colorData, srcFormat, &rgba, destFormat))
			{
				return false;
			}
			memcpy(&destData[i], rgba, destBpp);
			delete[] rgba;
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
			// copy only first line
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
		if (!Image::correctRect(sx, sy, sw, sh, srcWidth, srcHeight, dx, dy, destWidth, destHeight))
		{
			return false;
		}
		int srcBpp = srcFormat.getBpp();
		int destBpp = destFormat.getBpp();
		if (srcFormat == Format::Alpha && destFormat != Format::Alpha)
		{
			if (destBpp == 4)
			{
				if (CHECK_ALPHA_FORMAT(destFormat))
				{
					int x = 0;
					int y = 0;
					int da = -1;
					destFormat.getChannelIndices(NULL, NULL, NULL, &da);
					for_iterx (y, 0, sh)
					{
						for_iterx (x, 0, sw)
						{
							destData[((dx + x) + (dy + y) * destWidth) * destBpp + da] = srcData[((sx + x) + (sy + y) * srcWidth) * srcBpp];
						}
					}
				}
				return true;
			}
			return false;
		}
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

	static float _srcXs[MAX_WRITE_SIZE];
	static int _x0s[MAX_WRITE_SIZE];
	static int _x1s[MAX_WRITE_SIZE];
	static float _rx0s[MAX_WRITE_SIZE];
	static float _rx1s[MAX_WRITE_SIZE];
	static float _srcYs[MAX_WRITE_SIZE];
	static int _y0s[MAX_WRITE_SIZE];
	static int _y1s[MAX_WRITE_SIZE];
	static float _ry0s[MAX_WRITE_SIZE];
	static float _ry1s[MAX_WRITE_SIZE];

	bool Image::writeStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, unsigned char* srcData, int srcWidth, int srcHeight, Format srcFormat,
		unsigned char* destData, int destWidth, int destHeight, Format destFormat)
	{
		if (!Image::correctRect(sx, sy, sw, sh, srcWidth, srcHeight, dx, dy, dw, dh, destWidth, destHeight))
		{
			return false;
		}
		if (sw == dw && sh == dh)
		{
			return Image::write(sx, sy, sw, sh, dx, dh, srcData, srcWidth, srcHeight, srcFormat, destData, destWidth, destHeight, destFormat);
		}
		if (dw > MAX_WRITE_SIZE || dh > MAX_WRITE_SIZE)
		{
			hlog::errorf(logTag, "Cannot call Image::writeStretch() with dimensions bigger than %d!", MAX_WRITE_SIZE);
			return false;
		}
		int bpp = destFormat.getBpp();
		float fw = (dw > sw ? (sw - 1.0f) / dw : (float)sw / dw);
		float fh = (dh > sh ? (sh - 1.0f) / dh : (float)sh / dh);
		unsigned char* dest = NULL;
		unsigned char* ctl = NULL;
		unsigned char* ctr = NULL;
		unsigned char* cbl = NULL;
		unsigned char* cbr = NULL;
		// preparing some data first
		int x = 0;
		int y = 0;
		for_iterx (y, 0, dh)
		{
			_srcYs[y] = sy + y * fh;
			_y0s[y] = (int)_srcYs[y];
			_ry0s[y] = _srcYs[y] - _y0s[y];
			_y1s[y] = hmin(_y0s[y] + 1, sy + sh - 1);
			_ry1s[y] = 1.0f - _ry0s[y];
		}
		for_iterx (x, 0, dw)
		{
			_srcXs[x] = sx + x * fw;
			_x0s[x] = (int)_srcXs[x];
			_rx0s[x] = _srcXs[x] - _x0s[x];
			_x1s[x] = hmin(_x0s[x] + 1, sx + sw - 1);
			_rx1s[x] = 1.0f - _rx0s[x];
		}
		// the interpolated writing
		if (srcFormat == Format::Alpha && destFormat != Format::Alpha)
		{
			if (bpp == 4)
			{
				if (CHECK_ALPHA_FORMAT(destFormat))
				{
					int da = -1;
					destFormat.getChannelIndices(NULL, NULL, NULL, &da);
					for_iterx (y, 0, dh)
					{
						for_iterx (x, 0, dw)
						{
							dest = &destData[((dx + x) + (dy + y) * destWidth) * bpp];
							// linear interpolation
							ctl = &srcData[(_x0s[x] + _y0s[y] * srcWidth) * bpp];
							if (_rx0s[x] != 0.0f && _ry0s[y] != 0.0f)
							{
								ctr = &srcData[(_x1s[x] + _y0s[y] * srcWidth) * bpp];
								cbl = &srcData[(_x0s[x] + _y1s[y] * srcWidth) * bpp];
								cbr = &srcData[(_x1s[x] + _y1s[y] * srcWidth) * bpp];
								dest[da] = (unsigned char)(((ctl[0] * _ry1s[y] + cbl[0] * _ry0s[y]) * _rx1s[x] + (ctr[0] * _ry1s[y] + cbr[0] * _ry0s[y]) * _rx0s[x]));
							}
							else if (_rx0s[x] != 0.0f)
							{
								ctr = &srcData[(_x1s[x] + _y0s[y] * srcWidth) * bpp];
								dest[da] = (unsigned char)((ctl[0] * _rx1s[x] + ctr[0] * _rx0s[x]));
							}
							else if (_ry0s[y] != 0.0f)
							{
								cbl = &srcData[(_x0s[x] + _y1s[y] * srcWidth) * bpp];
								dest[da] = (unsigned char)((ctl[0] * _ry1s[y] + cbl[0] * _ry0s[y]));
							}
							else
							{
								dest[da] = ctl[0];
							}
						}
					}
				}
				return true;
			}
			return false;
		}
		bool createNew = Image::needsConversion(srcFormat, destFormat);
		if (createNew)
		{
			unsigned char* data = srcData;
			srcData = new unsigned char[sw * sh * bpp];
			if (!Image::write(sx, sy, sw, sh, 0, 0, data, srcWidth, srcHeight, srcFormat, srcData, sw, sh, destFormat))
			{
				delete[] srcData;
				return false;
			}
			// changed size of data, needs to readjust
			sx = 0;
			sy = 0;
			srcWidth = sw;
			srcHeight = sh;
		}
		bool result = false;
		if (bpp == 1)
		{
			for_iterx (y, 0, dh)
			{
				for_iterx (x, 0, dw)
				{
					dest = &destData[((dx + x) + (dy + y) * destWidth) * bpp];
					// linear interpolation
					ctl = &srcData[(_x0s[x] + _y0s[y] * srcWidth) * bpp];
					if (_rx0s[x] != 0.0f && _ry0s[y] != 0.0f)
					{
						ctr = &srcData[(_x1s[x] + _y0s[y] * srcWidth) * bpp];
						cbl = &srcData[(_x0s[x] + _y1s[y] * srcWidth) * bpp];
						cbr = &srcData[(_x1s[x] + _y1s[y] * srcWidth) * bpp];
						dest[0] = (unsigned char)(((ctl[0] * _ry1s[y] + cbl[0] * _ry0s[y]) * _rx1s[x] + (ctr[0] * _ry1s[y] + cbr[0] * _ry0s[y]) * _rx0s[x]));
					}
					else if (_rx0s[x] != 0.0f)
					{
						ctr = &srcData[(_x1s[x] + _y0s[y] * srcWidth) * bpp];
						dest[0] = (unsigned char)((ctl[0] * _rx1s[x] + ctr[0] * _rx0s[x]));
					}
					else if (_ry0s[y] != 0.0f)
					{
						cbl = &srcData[(_x0s[x] + _y1s[y] * srcWidth) * bpp];
						dest[0] = (unsigned char)((ctl[0] * _ry1s[y] + cbl[0] * _ry0s[y]));
					}
					else
					{
						dest[0] = ctl[0];
					}
				}
			}
			result = true;
		}
		else if (bpp == 3)
		{
			for_iterx (y, 0, dh)
			{
				for_iterx (x, 0, dw)
				{
					dest = &destData[((dx + x) + (dy + y) * destWidth) * bpp];
					// linear interpolation
					ctl = &srcData[(_x0s[x] + _y0s[y] * srcWidth) * bpp];
					if (_rx0s[x] != 0.0f && _ry0s[y] != 0.0f)
					{
						ctr = &srcData[(_x1s[x] + _y0s[y] * srcWidth) * bpp];
						cbl = &srcData[(_x0s[x] + _y1s[y] * srcWidth) * bpp];
						cbr = &srcData[(_x1s[x] + _y1s[y] * srcWidth) * bpp];
						dest[0] = (unsigned char)(((ctl[0] * _ry1s[y] + cbl[0] * _ry0s[y]) * _rx1s[x] + (ctr[0] * _ry1s[y] + cbr[0] * _ry0s[y]) * _rx0s[x]));
						dest[1] = (unsigned char)(((ctl[1] * _ry1s[y] + cbl[1] * _ry0s[y]) * _rx1s[x] + (ctr[1] * _ry1s[y] + cbr[1] * _ry0s[y]) * _rx0s[x]));
						dest[2] = (unsigned char)(((ctl[2] * _ry1s[y] + cbl[2] * _ry0s[y]) * _rx1s[x] + (ctr[2] * _ry1s[y] + cbr[2] * _ry0s[y]) * _rx0s[x]));
					}
					else if (_rx0s[x] != 0.0f)
					{
						ctr = &srcData[(_x1s[x] + _y0s[y] * srcWidth) * bpp];
						dest[0] = (unsigned char)((ctl[0] * _rx1s[x] + ctr[0] * _rx0s[x]));
						dest[1] = (unsigned char)((ctl[1] * _rx1s[x] + ctr[1] * _rx0s[x]));
						dest[2] = (unsigned char)((ctl[2] * _rx1s[x] + ctr[2] * _rx0s[x]));
					}
					else if (_ry0s[y] != 0.0f)
					{
						cbl = &srcData[(_x0s[x] + _y1s[y] * srcWidth) * bpp];
						dest[0] = (unsigned char)((ctl[0] * _ry1s[y] + cbl[0] * _ry0s[y]));
						dest[1] = (unsigned char)((ctl[1] * _ry1s[y] + cbl[1] * _ry0s[y]));
						dest[2] = (unsigned char)((ctl[2] * _ry1s[y] + cbl[2] * _ry0s[y]));
					}
					else
					{
						dest[0] = ctl[0];
						dest[1] = ctl[1];
						dest[2] = ctl[2];
					}
				}
			}
			result = true;
		}
		else if (bpp == 4)
		{
			for_iterx (y, 0, dh)
			{
				for_iterx (x, 0, dw)
				{
					dest = &destData[((dx + x) + (dy + y) * destWidth) * bpp];
					// linear interpolation
					ctl = &srcData[(_x0s[x] + _y0s[y] * srcWidth) * bpp];
					if (_rx0s[x] != 0.0f && _ry0s[y] != 0.0f)
					{
						ctr = &srcData[(_x1s[x] + _y0s[y] * srcWidth) * bpp];
						cbl = &srcData[(_x0s[x] + _y1s[y] * srcWidth) * bpp];
						cbr = &srcData[(_x1s[x] + _y1s[y] * srcWidth) * bpp];
						dest[0] = (unsigned char)(((ctl[0] * _ry1s[y] + cbl[0] * _ry0s[y]) * _rx1s[x] + (ctr[0] * _ry1s[y] + cbr[0] * _ry0s[y]) * _rx0s[x]));
						dest[1] = (unsigned char)(((ctl[1] * _ry1s[y] + cbl[1] * _ry0s[y]) * _rx1s[x] + (ctr[1] * _ry1s[y] + cbr[1] * _ry0s[y]) * _rx0s[x]));
						dest[2] = (unsigned char)(((ctl[2] * _ry1s[y] + cbl[2] * _ry0s[y]) * _rx1s[x] + (ctr[2] * _ry1s[y] + cbr[2] * _ry0s[y]) * _rx0s[x]));
						dest[3] = (unsigned char)(((ctl[3] * _ry1s[y] + cbl[3] * _ry0s[y]) * _rx1s[x] + (ctr[3] * _ry1s[y] + cbr[3] * _ry0s[y]) * _rx0s[x]));
					}
					else if (_rx0s[x] != 0.0f)
					{
						ctr = &srcData[(_x1s[x] + _y0s[y] * srcWidth) * bpp];
						dest[0] = (unsigned char)((ctl[0] * _rx1s[x] + ctr[0] * _rx0s[x]));
						dest[1] = (unsigned char)((ctl[1] * _rx1s[x] + ctr[1] * _rx0s[x]));
						dest[2] = (unsigned char)((ctl[2] * _rx1s[x] + ctr[2] * _rx0s[x]));
						dest[3] = (unsigned char)((ctl[3] * _rx1s[x] + ctr[3] * _rx0s[x]));
					}
					else if (_ry0s[y] != 0.0f)
					{
						cbl = &srcData[(_x0s[x] + _y1s[y] * srcWidth) * bpp];
						dest[0] = (unsigned char)((ctl[0] * _ry1s[y] + cbl[0] * _ry0s[y]));
						dest[1] = (unsigned char)((ctl[1] * _ry1s[y] + cbl[1] * _ry0s[y]));
						dest[2] = (unsigned char)((ctl[2] * _ry1s[y] + cbl[2] * _ry0s[y]));
						dest[3] = (unsigned char)((ctl[3] * _ry1s[y] + cbl[3] * _ry0s[y]));
					}
					else
					{
						dest[0] = ctl[0];
						dest[1] = ctl[1];
						dest[2] = ctl[2];
						dest[3] = ctl[3];
					}
				}
			}
			result = true;
		}
		if (createNew)
		{
			delete[] srcData;
		}
		return result;
	}

	bool Image::blit(int sx, int sy, int sw, int sh, int dx, int dy, unsigned char* srcData, int srcWidth, int srcHeight, Format srcFormat,
		unsigned char* destData, int destWidth, int destHeight, Format destFormat, unsigned char alpha)
	{
		if (!Image::correctRect(sx, sy, sw, sh, srcWidth, srcHeight, dx, dy, destWidth, destHeight))
		{
			return false;
		}
		// source format doesn't have alpha and no alpha multiplier is used, so using write() is enough
		if (!CHECK_ALPHA_FORMAT(srcFormat) && alpha == 255)
		{
			return Image::write(sx, sy, sw, sh, dx, dy, srcData, srcWidth, srcHeight, srcFormat, destData, destWidth, destHeight, destFormat);
		}
		// it's invisible anyway, so let's say it's successful
		if (alpha == 0)
		{
			return true;
		}
		int srcBpp = srcFormat.getBpp();
		if (srcBpp == 1)
		{
			if (Image::_blitFrom1Bpp(sx, sy, sw, sh, dx, dy, srcData, srcWidth, srcHeight, srcFormat, destData, destWidth, destHeight, destFormat, alpha))
			{
				return true;
			}
		}
		else if (srcBpp == 3)
		{
			if (Image::_blitFrom3Bpp(sx, sy, sw, sh, dx, dy, srcData, srcWidth, srcHeight, srcFormat, destData, destWidth, destHeight, destFormat, alpha))
			{
				return true;
			}
		}
		else if (srcBpp == 4)
		{
			if (Image::_blitFrom4Bpp(sx, sy, sw, sh, dx, dy, srcData, srcWidth, srcHeight, srcFormat, destData, destWidth, destHeight, destFormat, alpha))
			{
				return true;
			}
		}
		return false;
	}

	bool Image::_blitFrom1Bpp(int sx, int sy, int sw, int sh, int dx, int dy, unsigned char* srcData, int srcWidth, int srcHeight, Format srcFormat, unsigned char* destData, int destWidth, int destHeight, Format destFormat, unsigned char alpha)
	{
		static int srcBpp = 1;
		int destBpp = destFormat.getBpp();
		if (srcFormat == Format::Alpha && destFormat != Format::Alpha && destBpp != 4)
		{
			return false;
		}
		unsigned char* src = NULL;
		unsigned char* dest = NULL;
		unsigned char a1;
		unsigned int c;
		int x = 0;
		int y = 0;
		a1 = 255 - alpha;
		if (destBpp == 1)
		{
			for_iterx (y, 0, sh)
			{
				for_iterx (x, 0, sw)
				{
					src = &srcData[((sx + x) + (sy + y) * srcWidth) * srcBpp];
					dest = &destData[((dx + x) + (dy + y) * destWidth) * destBpp];
					dest[0] = (src[0] * alpha + dest[0] * a1) / 255;
				}
			}
			return true;
		}
		int dr = -1;
		int dg = -1;
		int db = -1;
		if (destBpp == 3 || !CHECK_ALPHA_FORMAT(destFormat)) // 3 BPP and 4 BPP without alpha
		{
			if (srcFormat != Format::Alpha)
			{
				destFormat.getChannelIndices(&dr, &dg, &db, NULL);
				for_iterx (y, 0, sh)
				{
					for_iterx (x, 0, sw)
					{
						src = &srcData[((sx + x) + (sy + y) * srcWidth) * srcBpp];
						dest = &destData[((dx + x) + (dy + y) * destWidth) * destBpp];
						c = src[0] * alpha;
						dest[dr] = (c + dest[dr] * a1) / 255;
						dest[dg] = (c + dest[dg] * a1) / 255;
						dest[db] = (c + dest[db] * a1) / 255;
					}
				}
			}
			return true;
		}
		int da = -1;
		if (destBpp == 4) // 4 BPP with alpha
		{
			destFormat.getChannelIndices(&dr, &dg, &db, &da);
			if (srcFormat != Format::Alpha)
			{
				for_iterx (y, 0, sh)
				{
					for_iterx (x, 0, sw)
					{
						src = &srcData[((sx + x) + (sy + y) * srcWidth) * srcBpp];
						dest = &destData[((dx + x) + (dy + y) * destWidth) * destBpp];
						c = src[0] * alpha;
						dest[dr] = (c + dest[dr] * a1) / 255;
						dest[dg] = (c + dest[dg] * a1) / 255;
						dest[db] = (c + dest[db] * a1) / 255;
						dest[da] = alpha + dest[da] * a1 / 255;
					}
				}
			}
			else
			{
				for_iterx (y, 0, sh)
				{
					for_iterx (x, 0, sw)
					{
						src = &srcData[((sx + x) + (sy + y) * srcWidth) * srcBpp];
						dest = &destData[((dx + x) + (dy + y) * destWidth) * destBpp];
						dest[da] = (src[0] * alpha + dest[da] * a1) / 255;
					}
				}
			}
			return true;
		}
		return false;
	}

	bool Image::_blitFrom3Bpp(int sx, int sy, int sw, int sh, int dx, int dy, unsigned char* srcData, int srcWidth, int srcHeight, Format srcFormat, unsigned char* destData, int destWidth, int destHeight, Format destFormat, unsigned char alpha)
	{
		static int srcBpp = 3;
		int destBpp = destFormat.getBpp();
		unsigned char* src = NULL;
		unsigned char* dest = NULL;
		unsigned char a1 = 255 - alpha;
		int x = 0;
		int y = 0;
		int sr = -1;
		if (destBpp == 1)
		{
			srcFormat.getChannelIndices(&sr, NULL, NULL, NULL);
			for_iterx (y, 0, sh)
			{
				for_iterx (x, 0, sw)
				{
					src = &srcData[((sx + x) + (sy + y) * srcWidth) * srcBpp];
					dest = &destData[((dx + x) + (dy + y) * destWidth) * destBpp];
					dest[0] = (src[sr] * alpha + dest[0] * a1) / 255;
				}
			}
			return true;
		}
		int sg = -1;
		int sb = -1;
		srcFormat.getChannelIndices(&sr, &sg, &sb, NULL);
		int dr = -1;
		int dg = -1;
		int db = -1;
		if (destBpp == 3 || !CHECK_ALPHA_FORMAT(destFormat)) // 3 BPP and 4 BPP without alpha
		{
			destFormat.getChannelIndices(&dr, &dg, &db, NULL);
			for_iterx (y, 0, sh)
			{
				for_iterx (x, 0, sw)
				{
					src = &srcData[((sx + x) + (sy + y) * srcWidth) * srcBpp];
					dest = &destData[((dx + x) + (dy + y) * destWidth) * destBpp];
					dest[dr] = (src[sr] * alpha + dest[dr] * a1) / 255;
					dest[dg] = (src[sg] * alpha + dest[dg] * a1) / 255;
					dest[db] = (src[sb] * alpha + dest[db] * a1) / 255;
				}
			}
			return true;
		}
		int da = -1;
		if (destBpp == 4) // 4 BPP with alpha
		{
			destFormat.getChannelIndices(&dr, &dg, &db, &da);
			for_iterx (y, 0, sh)
			{
				for_iterx (x, 0, sw)
				{
					src = &srcData[((sx + x) + (sy + y) * srcWidth) * srcBpp];
					dest = &destData[((dx + x) + (dy + y) * destWidth) * destBpp];
					dest[dr] = (src[sr] * alpha + dest[dr] * a1) / 255;
					dest[dg] = (src[sg] * alpha + dest[dg] * a1) / 255;
					dest[db] = (src[sb] * alpha + dest[db] * a1) / 255;
					dest[da] = alpha + dest[da] * a1 / 255;
				}
			}
			return true;
		}
		return false;
	}
	
	bool Image::_blitFrom4Bpp(int sx, int sy, int sw, int sh, int dx, int dy, unsigned char* srcData, int srcWidth, int srcHeight, Format srcFormat, unsigned char* destData, int destWidth, int destHeight, Format destFormat, unsigned char alpha)
	{
		static int srcBpp = 4;
		int destBpp = destFormat.getBpp();
		unsigned char* src = NULL;
		unsigned char* dest = NULL;
		unsigned char a0 = 0;
		unsigned char a1 = 0;
		int x = 0;
		int y = 0;
		int sr = -1;
		int sa = -1;
		if (destBpp == 1)
		{
			srcFormat.getChannelIndices(&sr, NULL, NULL, &sa);
			for_iterx (y, 0, sh)
			{
				for_iterx (x, 0, sw)
				{
					src = &srcData[((sx + x) + (sy + y) * srcWidth) * srcBpp];
					dest = &destData[((dx + x) + (dy + y) * destWidth) * destBpp];
					a0 = src[sa] * alpha / 255;
					if (a0 > 0)
					{
						dest[0] = (src[sr] * a0 + dest[0] * (255 - a0)) / 255;
					}
				}
			}
			return true;
		}
		int sg = -1;
		int sb = -1;
		srcFormat.getChannelIndices(&sr, &sg, &sb, &sa);
		int dr = -1;
		int dg = -1;
		int db = -1;
		if (destBpp == 3 || !CHECK_ALPHA_FORMAT(destFormat)) // 3 BPP and 4 BPP without alpha
		{
			destFormat.getChannelIndices(&dr, &dg, &db, NULL);
			for_iterx (y, 0, sh)
			{
				for_iterx (x, 0, sw)
				{
					src = &srcData[((sx + x) + (sy + y) * srcWidth) * srcBpp];
					dest = &destData[((dx + x) + (dy + y) * destWidth) * destBpp];
					a0 = src[sa] * alpha / 255;
					if (a0 > 0)
					{
						a1 = (255 - a0);
						dest[dr] = (src[sr] * a0 + dest[dr] * a1) / 255;
						dest[dg] = (src[sg] * a0 + dest[dg] * a1) / 255;
						dest[db] = (src[sb] * a0 + dest[db] * a1) / 255;
					}
				}
			}
			return true;
		}
		int da = -1;
		if (destBpp == 4) // 4 BPP with alpha
		{
			destFormat.getChannelIndices(&dr, &dg, &db, &da);
			for_iterx (y, 0, sh)
			{
				for_iterx (x, 0, sw)
				{
					src = &srcData[((sx + x) + (sy + y) * srcWidth) * srcBpp];
					dest = &destData[((dx + x) + (dy + y) * destWidth) * destBpp];
					a0 = src[sa] * alpha / 255;
					if (a0 > 0)
					{
						a1 = (255 - a0) * dest[da] / 255;
						dest[da] = a0 + a1;
						dest[dr] = (src[sr] * a0 + dest[dr] * a1) / dest[da];
						dest[dg] = (src[sg] * a0 + dest[dg] * a1) / dest[da];
						dest[db] = (src[sb] * a0 + dest[db] * a1) / dest[da];
					}
				}
			}
			return true;
		}
		return false;
	}

	bool Image::blitStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, unsigned char* srcData, int srcWidth, int srcHeight, Format srcFormat,
		unsigned char* destData, int destWidth, int destHeight, Format destFormat, unsigned char alpha)
	{
		if (!Image::correctRect(sx, sy, sw, sh, srcWidth, srcHeight, dx, dy, dw, dh, destWidth, destHeight))
		{
			return false;
		}
		if (sw == dw && sh == dh)
		{
			return Image::blit(sx, sy, sw, sh, dx, dh, srcData, srcWidth, srcHeight, srcFormat, destData, destWidth, destHeight, destFormat, alpha);
		}
		unsigned char* stretched = new unsigned char[dw * dh * srcFormat.getBpp()];
		bool result = Image::writeStretch(sx, sy, sw, sh, 0, 0, dw, dh, srcData, srcWidth, srcHeight, srcFormat, stretched, dw, dh, srcFormat);
		if (result)
		{
			result = Image::blit(0, 0, dw, dh, dx, dy, stretched, dw, dh, srcFormat, destData, destWidth, destHeight, destFormat, alpha);
		}
		delete[] stretched;
		return result;
	}

	bool Image::rotateHue(int x, int y, int w, int h, float degrees, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat)
	{
		if (!Image::correctRect(x, y, w, h, srcWidth, srcHeight))
		{
			return false;
		}
		int srcBpp = srcFormat.getBpp();
		if (srcBpp == 1)
		{
			return true;
		}
		float range = hmodf(degrees / 360.0f, 1.0f);
		if (range == 0.0f)
		{
			return true;
		}
		int sr = -1;
		int sg = -1;
		int sb = -1;
		srcFormat.getChannelIndices(&sr, &sg, &sb, NULL);
		float _h;
		float _s;
		float _l;
		int i;
		for_iter (dy, 0, h)
		{
			for_iter (dx, 0, w)
			{
				i = ((x + dx) + (y + dy) * srcWidth) * srcBpp;
				april::rgbToHsl(srcData[i + sr], srcData[i + sg], srcData[i + sb], &_h, &_s, &_l);
				april::hslToRgb(hmodf(_h + range, 1.0f), _s, _l, &srcData[i + sr], &srcData[i + sg], &srcData[i + sb]);
			}
		}
		return true;
	}

	bool Image::saturate(int x, int y, int w, int h, float factor, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat)
	{
		if (!Image::correctRect(x, y, w, h, srcWidth, srcHeight))
		{
			return false;
		}
		int srcBpp = srcFormat.getBpp();
		if (srcBpp == 1)
		{
			return true;
		}
		int sr = -1;
		int sg = -1;
		int sb = -1;
		srcFormat.getChannelIndices(&sr, &sg, &sb, NULL);
		float _h;
		float _s;
		float _l;
		int i;
		for_iter (dy, 0, h)
		{
			for_iter (dx, 0, w)
			{
				i = ((x + dx) + (y + dy) * srcWidth) * srcBpp;
				april::rgbToHsl(srcData[i + sr], srcData[i + sg], srcData[i + sb], &_h, &_s, &_l);
				april::hslToRgb(_h, hclamp(_s * factor, 0.0f, 1.0f), _l, &srcData[i + sr], &srcData[i + sg], &srcData[i + sb]);
			}
		}
		return true;
	}

	bool Image::invert(int x, int y, int w, int h, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat)
	{
		if (!Image::correctRect(x, y, w, h, srcWidth, srcHeight))
		{
			return false;
		}
		int i;
		int srcBpp = srcFormat.getBpp();
		if (srcBpp == 1)
		{
			for_iter (dy, 0, h)
			{
				for_iter (dx, 0, w)
				{
					i = ((x + dx) + (y + dy) * srcWidth);
					srcData[i] = 255 - srcData[i];
				}
			}
			return true;
		}
		int sr = -1;
		int sg = -1;
		int sb = -1;
		srcFormat.getChannelIndices(&sr, &sg, &sb, NULL);
		for_iter (dy, 0, h)
		{
			for_iter (dx, 0, w)
			{
				i = ((x + dx) + (y + dy) * srcWidth) * srcBpp;
				srcData[i + sr] = 255 - srcData[i + sr];
				srcData[i + sg] = 255 - srcData[i + sg];
				srcData[i + sb] = 255 - srcData[i + sb];
			}
		}
		return true;
	}

	bool Image::insertAlphaMap(int w, int h, unsigned char* srcData, Image::Format srcFormat, unsigned char* destData, Image::Format destFormat, unsigned char median, int ambiguity)
	{
		if (!CHECK_ALPHA_FORMAT(destFormat)) // not a format that supports an alpha channel
		{
			return false;
		}
		int srcBpp = srcFormat.getBpp();
		if (srcBpp == 1 || srcBpp == 3 || srcBpp == 4)
		{
			int destBpp = destFormat.getBpp();
			int sr = -1;
			srcFormat.getChannelIndices(&sr, NULL, NULL, NULL);
			int da = -1;
			destFormat.getChannelIndices(NULL, NULL, NULL, &da);
			unsigned char* src = NULL;
			unsigned char* dest = NULL;
			int i = 0;
			int x = 0;
			int y = 0;
			if (ambiguity == 0)
			{
				for_iterx (y, 0, h)
				{
					for_iterx (x, 0, w)
					{
						i = (x + y * w);
						// takes the red second color channel for alpha value
						destData[i * destBpp + da] = srcData[i * srcBpp + sr];
					}
				}
			}
			else
			{
				int min = (int)median - ambiguity / 2;
				int max = (int)median + ambiguity / 2;
				for_iterx (y, 0, h)
				{
					for_iterx (x, 0, w)
					{
						i = (x + y * w);
						src = &srcData[i * srcBpp];
						dest = &destData[i * destBpp];
						// takes the red second color channel for alpha value
						if (src[sr] < min)
						{
							dest[da] = 255;
						}
						else if (src[sr] >= max)
						{
							dest[da] = 0;
						}
						else
						{
							dest[da] = (max - src[sr]) * 255 / ambiguity;
						}
					}
				}
			}
			return true;
		}
		return false;
	}

	bool Image::dilate(unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat, unsigned char* destData, int destWidth, int destHeight, Image::Format destFormat)
	{
		// both images must be single-channel 8-bit images, currently other formats are not supported
		if ((srcFormat != Format::Alpha && srcFormat != Format::Greyscale) || (destFormat != Format::Alpha && destFormat != Format::Greyscale))
		{
			return false;
		}
		if (srcWidth % 2 == 0 || srcHeight % 2 == 0) // has to have odd-numbered dimensions with a central pixel
		{
			return false;
		}
		Image* original = Image::create(destWidth, destHeight, destData, destFormat);
		unsigned char* originalData = original->data;
		memset(destData, 0, destWidth * destHeight * destFormat.getBpp());
		int i = 0;
		int j = 0;
		int m = 0;
		int n = 0;
		int ox = srcWidth / 2;
		int oy = srcHeight / 2;
		int index = 0;
		int indexSrc = 0;
		int indexOriginal = 0;
		for_iterx (j, 0, destHeight)
		{
			for_iterx (i, 0, destWidth)
			{
				index = i + j * destWidth;
				for_iterx (n, 0, srcHeight)
				{
					if (hbetweenIE(j + n - oy, 0, destHeight))
					{
						for_iterx (m, 0, srcWidth)
						{
							if (hbetweenIE(i + m - ox, 0, destWidth))
							{
								indexSrc = m + n * srcWidth;
								if (srcData[indexSrc] > 0)
								{
									indexOriginal = i + m - ox + (j + n - oy) * destWidth;
									if (originalData[indexOriginal] > 0)
									{
										// multiplication is faster than dividing by 255
										destData[index] = hmax(destData[index], (unsigned char)(0.003921569f * originalData[indexOriginal] * srcData[indexSrc]));
										if (destData[index] == 255)
										{
											break;
										}
									}
								}
							}
						}
						if (destData[index] == 255)
						{
							break;
						}
					}
				}
			}
		}
		delete original;
		return true;
	}

	bool Image::convertToFormat(int w, int h, unsigned char* srcData, Image::Format srcFormat, unsigned char** destData, Image::Format destFormat, bool preventCopy)
	{
		if (preventCopy && srcFormat == destFormat)
		{
			hlog::warn(logTag, "The source's and destination's formats are the same!");
			return false;
		}
		int srcBpp = srcFormat.getBpp();
		if ((srcFormat == Format::Compressed || srcFormat == Format::Palette) && (destFormat == Format::Compressed || destFormat == Format::Palette))
		{
			return true;
		}
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
		hlog::errorf(logTag, "Conversion from %d BPP to %d BPP is not supported!", srcBpp, destFormat.getBpp());
		return false;
	}

	bool Image::_convertFrom1Bpp(int w, int h, unsigned char* srcData, Format srcFormat, unsigned char** destData, Format destFormat)
	{
		int destBpp = destFormat.getBpp();
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
		int x = 0;
		int y = 0;
		if (destBpp == 3 || destBpp == 4)
		{
			int i = 0;
			if (destBpp > 3)
			{
				memset(*destData, 255, w * h * destBpp);
				if (!CHECK_LEFT_RGB(destFormat))
				{
					for_iterx (y, 0, h)
					{
						for_iterx (x, 0, w)
						{
							i = (x + y * w) * destBpp;
							(*destData)[i + 1] = (*destData)[i + 2] = (*destData)[i + 3] = srcData[x + y * w];
						}
					}
					return true;
				}
			}
			for_iterx (y, 0, h)
			{
				for_iterx (x, 0, w)
				{
					i = (x + y * w) * destBpp;
					(*destData)[i] = (*destData)[i + 1] = (*destData)[i + 2] = srcData[x + y * w];
				}
			}
			return true;
		}
		if (createData)
		{
			delete[] *destData;
			*destData = NULL;
		}
		return false;
	}

	bool Image::_convertFrom3Bpp(int w, int h, unsigned char* srcData, Format srcFormat, unsigned char** destData, Format destFormat)
	{
		static int srcBpp = 3;
		int destBpp = destFormat.getBpp();
		bool createData = (*destData == NULL);
		if (createData)
		{
			*destData = new unsigned char[w * h * destBpp];
		}
		int x = 0;
		int y = 0;
		if (destBpp == 1)
		{
			int redIndex = (srcFormat == Format::RGB ? 0 : 2);
			for_iterx (y, 0, h)
			{
				for_iterx (x, 0, w)
				{
					// red is used as main component
					(*destData)[x + y * w] = srcData[(x + y * w) * srcBpp + redIndex];
				}
			}
			return true;
		}
		if (destBpp == 3)
		{
			memcpy(*destData, srcData, w * h * destBpp);
			// Format::RGB to Format::BGR and vice versa, thus switching 2 bytes around is enough
			if (srcFormat != destFormat)
			{
				int i = 0;
				for_iterx (y, 0, h)
				{
					for_iterx (x, 0, w)
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
			Format extended = (srcFormat == Format::RGB ? Format::RGBX : Format::BGRX);
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
			delete[] *destData;
			*destData = NULL;
		}
		return false;
	}

	bool Image::_convertFrom4Bpp(int w, int h, unsigned char* srcData, Format srcFormat, unsigned char** destData, Format destFormat)
	{
		static int srcBpp = 4;
		int destBpp = destFormat.getBpp();
		bool createData = (*destData == NULL);
		if (createData)
		{
			*destData = new unsigned char[w * h * destBpp];
		}
		int x = 0;
		int y = 0;
		if (destBpp == 1)
		{
			int redIndex = 0;
			if (srcFormat == Format::ARGB || srcFormat == Format::XRGB)
			{
				redIndex = 1;
			}
			else if (srcFormat == Format::BGRA || srcFormat == Format::BGRX)
			{
				redIndex = 2;
			}
			else if (srcFormat == Format::ABGR || srcFormat == Format::XBGR)
			{
				redIndex = 3;
			}
			for_iterx (y, 0, h)
			{
				for_iterx (x, 0, w)
				{
					// red is used as main component
					(*destData)[x + y * w] = srcData[(x + y * w) * srcBpp + redIndex];
				}
			}
			return true;
		}
		if (destBpp == 3)
		{
			unsigned int* src = (unsigned int*)srcData;
			unsigned char* dest = *destData;
			Format extended = (destFormat == Format::RGB ? Format::RGBX : Format::BGRX);
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
			delete[] *destData;
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
		int srcBpp = srcFormat.getBpp();
		int destBpp = destFormat.getBpp();
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

	bool Image::checkRect(int x, int y, int destWidth, int destHeight)
	{
		return (hbetweenIE(x, 0, destWidth) && hbetweenIE(y, 0, destHeight));
	}

	bool Image::checkRect(int x, int y, int w, int h, int destWidth, int destHeight)
	{
		return (Image::checkRect(x, y, destWidth, destHeight) && x + w <= destWidth && y + h <= destHeight);
	}

	bool Image::correctRect(int& x, int& y, int& w, int& h, int dataWidth, int dataHeight)
	{
		if (x >= dataWidth || y >= dataHeight)
		{
			return false;
		}
		if (x < 0)
		{
			w += x;
			x = 0;
		}
		w = hmin(w, dataWidth - x);
		if (w < 0)
		{
			return false;
		}
		if (y < 0)
		{
			h += y;
			y = 0;
		}
		h = hmin(h, dataHeight - y);
		if (h < 0)
		{
			return false;
		}
		return true;
	}

	bool Image::correctRect(int& sx, int& sy, int& sw, int& sh, int srcWidth, int srcHeight, int& dx, int& dy, int destWidth, int destHeight)
	{
		if (!Image::checkRect(sx, sy, sw, sh, srcWidth, srcHeight))
		{
			return false;
		}
		if (dx < 0)
		{
			sx -= dx;
			sw += dx;
			dx = 0;
		}
		if (sx >= srcWidth || sw <= 0)
		{
			return false;
		}
		sw = hmin(sw, destWidth - dx);
		if (sw <= 0)
		{
			return false;
		}
		if (dy < 0)
		{
			sy -= dy;
			sh += dy;
			dy = 0;
		}
		if (sy >= srcHeight || sh <= 0)
		{
			return false;
		}
		sh = hmin(sh, destHeight - dy);
		if (sh <= 0)
		{
			return false;
		}
		return true;
	}

	bool Image::correctRect(int& sx, int& sy, int& sw, int& sh, int srcWidth, int srcHeight, int& dx, int& dy, int& dw, int& dh, int destWidth, int destHeight)
	{
		if (!Image::checkRect(sx, sy, sw, sh, srcWidth, srcHeight))
		{
			return false;
		}
		if (dw <= 0 || dh <= 0)
		{
			return false;
		}
		float fw = (float)sw / dw;
		if (dx < 0)
		{
			sx = (int)(sx - dx * fw);
			sw = (int)(sw + dx * fw);
			dw += dx;
			dx = 0;
		}
		if (sx >= srcWidth || dw <= 0)
		{
			return false;
		}
		int ox = dw - destWidth + dx;
		if (ox > 0)
		{
			sw = (int)(sw - ox * fw);
			dw -= ox;
		}
		if (sw <= 0 || dw <= 0)
		{
			return false;
		}
		float fh = (float)sh / dh;
		if (dy < 0)
		{
			sy = (int)(sy - dy * fh);
			sh = (int)(sh + dy * fh);
			dh += dy;
			dy = 0;
		}
		if (sy >= srcHeight || dh <= 0)
		{
			return false;
		}
		int oy = dh - destHeight + dy;
		if (oy > 0)
		{
			sh = (int)(sh - oy * fh);
			dh -= oy;
		}
		if (sh <= 0 || dh <= 0)
		{
			return false;
		}
		return true;
	}

	void Image::registerCustomLoader(chstr extension, Image* (*loadFunction)(hsbase&), Image* (*metaDataLoadfunction)(hsbase&))
	{
		Image::customLoaders[extension] = loadFunction;
		Image::customMetaDataLoaders[extension] = metaDataLoadfunction;
	}

	void Image::registerCustomSaver(chstr extension, bool (*saveFunction)(hsbase&, Image*))
	{
		Image::customSavers[extension] = saveFunction;
	}

}
