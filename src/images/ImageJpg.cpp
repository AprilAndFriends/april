/// @file
/// @version 5.1
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#define __HL_INCLUDE_PLATFORM_HEADERS
#include <hltypes/hplatform.h>

#include <stdio.h>
#include <jpeglib.h>

#include <hltypes/hlog.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hresource.h>
#include <hltypes/hstream.h>
#include <hltypes/hstring.h>

#include "april.h"
#include "Image.h"

namespace april
{
	static bool hasError = false;

	static void _onError(j_common_ptr cInfo)
	{
		char buffer[JMSG_LENGTH_MAX] = { '\0' };
		(*cInfo->err->format_message)(cInfo, buffer);
		hlog::error(logTag, buffer);
		hasError = true;
	}

	Image* Image::_loadJpg(hsbase& stream, int size)
	{
		hasError = false;
		// first read the whole data from the resource file
		unsigned char* compressedData = new unsigned char[size];
		stream.readRaw(compressedData, size);
		// read JPEG image from file data
		struct jpeg_decompress_struct cInfo;
		struct jpeg_error_mgr jErr;
		cInfo.err = jpeg_std_error(&jErr);
		cInfo.err->error_exit = &_onError;
		jpeg_create_decompress(&cInfo);
		jpeg_mem_src(&cInfo, compressedData, size);
		jpeg_read_header(&cInfo, TRUE);
		if (hasError)
		{
			return NULL;
		}
		jpeg_start_decompress(&cInfo);
		if (hasError)
		{
			jpeg_destroy_decompress(&cInfo);
			return NULL;
		}
		unsigned char* imageData = new unsigned char[cInfo.output_width * cInfo.output_height * 3]; // JPEG is always RGB
		unsigned char* ptr = NULL;
		for_itert (unsigned int, i, 0, cInfo.output_height)
		{
			ptr = imageData + i * cInfo.output_width * 3;
			jpeg_read_scanlines(&cInfo, &ptr, 1);
			if (hasError)
			{
				jpeg_destroy_decompress(&cInfo);
				delete[] compressedData;
				return NULL;
			}
		}
		jpeg_finish_decompress(&cInfo);
		jpeg_destroy_decompress(&cInfo);
		delete[] compressedData;
		// assign Image data
		Image* image = new Image();
		image->data = imageData;
		image->w = cInfo.output_width;
		image->h = cInfo.output_height;
		image->format = Image::Format::RGB; // JPEG is always RGB
		return image;
	}

	Image* Image::_loadJpg(hsbase& stream)
	{
		return Image::_loadJpg(stream, (int)stream.size());
	}

	Image* Image::_readMetaDataJpg(hsbase& stream, int size)
	{
		hasError = false;
		// first read the whole data from the resource file
		unsigned char* compressedData = new unsigned char[size];
		stream.readRaw(compressedData, size);
		// read JPEG image from file data
		struct jpeg_decompress_struct cInfo;
		struct jpeg_error_mgr jErr;
		cInfo.err = jpeg_std_error(&jErr);
		cInfo.err->error_exit = &_onError;
		jpeg_create_decompress(&cInfo);
		jpeg_mem_src(&cInfo, compressedData, size);
		jpeg_read_header(&cInfo, TRUE);
		delete[] compressedData;
		if (hasError)
		{
			return NULL;
		}
		// assign Image data
		Image* image = new Image();
		image->data = NULL;
		image->w = cInfo.image_width;
		image->h = cInfo.image_height;
		image->format = Image::Format::RGB; // JPEG is always RGB
		return image;
	}

	Image* Image::_readMetaDataJpg(hsbase& stream)
	{
		return Image::_readMetaDataJpg(stream, (int)stream.size());
	}

}
