/// @file
/// @author  Boris Mikic
/// @version 3.2
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <stdio.h>

#include <hltypes/hltypesUtil.h>
#include <hltypes/hresource.h>
#include <hltypes/hsbase.h>
#include <hltypes/hstream.h>

extern "C"
{
#include <jpeglib.h> // has to be down here because of problems with header order
}

#include "Image.h"

namespace april
{
	Image* Image::_loadJpg(hsbase& stream)
	{
		// first read the whole data from the resource file
		int compressedSize = stream.size();
		unsigned char* compressedData = new unsigned char[compressedSize];
		stream.read_raw(compressedData, compressedSize);
		// read JPEG image from file data
		struct jpeg_decompress_struct cInfo;
		struct jpeg_error_mgr jErr;
		unsigned char* imageData = NULL;
		cInfo.err = jpeg_std_error(&jErr);
		jpeg_create_decompress(&cInfo);
		jpeg_mem_src(&cInfo, compressedData, compressedSize);
		jpeg_read_header(&cInfo, TRUE);
		jpeg_start_decompress(&cInfo);
		imageData = new unsigned char[cInfo.output_width * cInfo.output_height * 3]; // JPEG is always RGB
		unsigned char* ptr = NULL;
		for_itert (unsigned int, i, 0, cInfo.output_height)
		{
			ptr = imageData + i * cInfo.output_width * 3;
			jpeg_read_scanlines(&cInfo, &ptr, 1);
		}
		jpeg_finish_decompress(&cInfo);
		jpeg_destroy_decompress(&cInfo);
		delete [] compressedData;
		// assign Image data
		Image* img = new Image();
		img->data = imageData;
		img->w = cInfo.output_width;
		img->h = cInfo.output_height;
		img->bpp = 3; // JPEG is always RGB
		img->format = Image::FORMAT_RGB; // JPEG is always RGB
		return img;
	}

}
