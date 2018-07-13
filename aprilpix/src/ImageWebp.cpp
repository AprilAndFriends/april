/// @file
/// @version 1.1
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifdef _WEBP
#include <webp/decode.h>
#ifndef _WEBP_NO_ENCODE
#include <webp/encode.h>
#endif

#include <hltypes/hlog.h>
#include <hltypes/hstring.h>

#include "aprilpix.h"
#include "ImageWebp.h"

#define SAVE_QUALITY "quality"
#define SAVE_QUALITY_DEFAULT 95.0f
#define SAVE_LOSSLESS "lossless"
#define SAVE_LOSSLESS_DEFAULT true

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
		WebPBitstreamFeatures features;
		VP8StatusCode code = WebPGetFeatures(data, size, &features);
		delete[] data;
		if (code != VP8_STATUS_OK || features.width <= 0 || features.height <= 0)
		{
			hlog::error(logTag, "Could not load WEBP meta data!");
			return NULL;
		}
		april::Image* image = new ImageWebp();
		image->data = NULL;
		image->w = features.width;
		image->h = features.height;
		image->format = (features.has_alpha ? Format::RGBA : Format::RGB);
		return image;
	}

#ifndef _WEBP_NO_ENCODE
	bool ImageWebp::save(hsbase& stream, april::Image* image, april::Image::SaveParameters parameters)
	{
		bool result = false;
		if (image->format == april::Image::Format::RGB || image->format == april::Image::Format::RGBA ||
			image->format == april::Image::Format::BGR || image->format == april::Image::Format::BGRA)
		{
			int bpp = image->getBpp();
			int stride = image->w * bpp;
			int size = 0;
			unsigned char* fileData = NULL;
			bool lossless = (bool)parameters.tryGet(SAVE_LOSSLESS, SAVE_LOSSLESS_DEFAULT);
			if (lossless)
			{
				size_t(*function)(const uint8_t*, int, int, int, uint8_t**) = NULL;
				if (image->format == april::Image::Format::RGB)			function = &WebPEncodeLosslessRGB;
				else if (image->format == april::Image::Format::RGBA)	function = &WebPEncodeLosslessRGBA;
				else if (image->format == april::Image::Format::BGR)	function = &WebPEncodeLosslessBGR;
				else if (image->format == april::Image::Format::BGRA)	function = &WebPEncodeLosslessBGRA;
				if (function != NULL)
				{
					size = (int)(*function)((uint8_t*)image->data, image->w, image->h, stride, &fileData);
				}
			}
			else
			{
				size_t (*function)(const uint8_t*, int, int, int, float, uint8_t**) = NULL;
				if (image->format == april::Image::Format::RGB)			function = &WebPEncodeRGB;
				else if (image->format == april::Image::Format::RGBA)	function = &WebPEncodeRGBA;
				else if (image->format == april::Image::Format::BGR)	function = &WebPEncodeBGR;
				else if (image->format == april::Image::Format::BGRA)	function = &WebPEncodeBGRA;
				if (function != NULL)
				{
					float quality = hclamp((float)parameters.tryGet(SAVE_QUALITY, SAVE_QUALITY_DEFAULT), 0.0f, 100.0f);
					size = (int)(*function)((uint8_t*)image->data, image->w, image->h, stride, quality, &fileData);
				}
			}
			if (fileData != NULL)
			{
				if (size > 0)
				{
					stream.writeRaw(fileData, size);
					result = true;
				}
				WebPFree(fileData);
			}
		}
		return result;
	}

	april::Image::SaveParameters ImageWebp::makeDefaultSaveParameters()
	{
		SaveParameters result;
		result[SAVE_QUALITY] = SAVE_QUALITY_DEFAULT;
		result[SAVE_LOSSLESS] = SAVE_LOSSLESS_DEFAULT;
		return result;
	}
#endif

}
#endif
