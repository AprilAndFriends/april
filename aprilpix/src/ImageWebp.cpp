/// @file
/// @version 1.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifdef _WEBP
#include <webp/decode.h>

#include <hltypes/hlog.h>
#include <hltypes/hstring.h>

#include "aprilpix.h"
#include "ImageWebp.h"

namespace aprilpix
{
	ImageWebp::ImageWebp() : april::Image()
	{
	}

	ImageWebp::~ImageWebp()
	{
	}

	april::Image* ImageWebp::load(hsbase& stream)
	{
		int size = (int)stream.size();
		uint8_t* data = new uint8_t[size];
		stream.readRaw(data, size);
		WebPBitstreamFeatures features;
		VP8StatusCode code = WebPGetFeatures(data, size, &features);
		if (code != VP8_STATUS_OK || features.width <= 0 || features.height <= 0)
		{
			hlog::error(logTag, "Could not load WEBP file!");
			delete[] data;
			return NULL;
		}
		april::Image* image = new ImageWebp();
		image->w = features.width;
		image->h = features.height;
		int bpp = 0;
		if (features.has_alpha)
		{
			image->format = Format::RGBA;
			bpp = 4;
		}
		else
		{
			image->format = Format::RGB;
			bpp = 3;
		}
		int imageDataSize = image->w * image->h * bpp;
		image->data = new unsigned char[imageDataSize];
		uint8_t* result = NULL;
		if (bpp == 4)
		{
			result = WebPDecodeRGBAInto(data, size, image->data, imageDataSize, image->w * bpp);
		}
		else
		{
			result = WebPDecodeRGBInto(data, size, image->data, imageDataSize, image->w * bpp);
		}
		delete[] data;
		if (result == NULL || result != image->data)
		{
			hlog::error(logTag, "Could not decode WEBP file! Possibly not enough memory allocated.");
			delete image;
			image = NULL;
		}
		return image;
	}

	april::Image* ImageWebp::loadMetaData(hsbase& stream)
	{
		int size = (int)stream.size();
		uint8_t* data = new uint8_t[size];
		stream.readRaw(data, size);
		int width = 0;
		int height = 0;
		int result = WebPGetInfo(data, size, &width, &height);
		delete[] data;
		if (result != 0 || width <= 0 || height <= 0)
		{
			hlog::error(logTag, "Could not load WEBP meta data!");
			return NULL;
		}
		april::Image* image = new ImageWebp();
		image->data = NULL;
		image->w = width;
		image->h = height;
		return image;
	}

}
#endif
