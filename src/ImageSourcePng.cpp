/// @file
/// @author  Boris Mikic
/// @version 1.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifndef USE_IL
#include <stdio.h>
extern "C"
{
	#include <jpeg/jpeglib.h>
}
#include <png/png.h>
#include <png/pngpriv.h>
#include <png/pngstruct.h>

#include <hltypes/hltypesUtil.h>
#include <hltypes/hresource.h>

#include "ImageSource.h"
#include "RenderSystem.h"

namespace april
{
	void _pngZipRead(png_structp png, png_bytep data, png_size_t size)
	{
		hresource* file = (hresource*)png->io_ptr;
		file->read_raw(data, size);
	}

	ImageSource::ImageSource()
	{
		this->data = NULL;
		this->w = 0;
		this->h = 0;
		this->bpp = 0;
		this->format = april::AF_UNDEFINED;
	}
	
	ImageSource::~ImageSource()
	{
		if (this->data != NULL)
		{
			delete[] this->data;
		}
	}

	void ImageSource::copyImage(ImageSource* source, int bpp)
	{
		memcpy(this->data, source->data, bpp * source->w * source->h * sizeof(unsigned char));
	}
	
	ImageSource* loadImage(chstr filename)
	{
		ImageSource* img = new ImageSource();
		if (filename.lower().ends_with(".png"))
		{
			hresource file(filename);
			png_structp pngPtr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
			png_infop infoPtr = png_create_info_struct(pngPtr);
			png_infop endInfo = png_create_info_struct(pngPtr);
			setjmp(png_jmpbuf(pngPtr));
			png_set_read_fn(pngPtr, &file, &_pngZipRead);
			png_read_info(pngPtr, infoPtr);
			int bpp = infoPtr->pixel_depth / 8;
			int width;
			int height;
			int bitDepth;
			int colorType;
			png_get_IHDR(pngPtr, infoPtr, (unsigned int*)&width, (unsigned int*)&height, &bitDepth, &colorType, NULL, NULL, NULL);
			png_read_update_info(pngPtr, infoPtr);
			int rowBytes = png_get_rowbytes(pngPtr, infoPtr);
			png_byte* imageData = new png_byte[rowBytes * height];
			png_bytep* rowPointers = new png_bytep[height];
			for (int i = 0; i < height; i++)
			{
				rowPointers[i] = imageData + i * rowBytes;
			}
			png_read_image(pngPtr, rowPointers);
			png_destroy_read_struct(&pngPtr, &infoPtr, &endInfo);
			delete[] rowPointers;
			// assign ImageSource data
			img->data = imageData;
			img->w = width;
			img->h = height;
			img->bpp = bpp;
			switch (colorType)
			{
			case PNG_COLOR_TYPE_RGBA:
				img->format = AF_RGBA;
				break;
			case PNG_COLOR_TYPE_RGB:
				img->format = AF_RGB;
				break;
			default:
				img->format = AF_RGBA;
				break;
			}
		}
		else if (filename.lower().ends_with(".jpg") || filename.lower().ends_with(".jpeg"))
		{
			// first read the whole data from the resource file
			hresource file(filename);
			int compressedSize = file.size();
			unsigned char* compressedData = new unsigned char[compressedSize];
			file.read_raw(compressedData, compressedSize);
			file.close();
			// read JPEG image from file data
			struct jpeg_decompress_struct cInfo;
			struct jpeg_error_mgr jErr;
			unsigned char* imageData;
			cInfo.err = jpeg_std_error(&jErr);
			jpeg_create_decompress(&cInfo);
			jpeg_mem_src(&cInfo, compressedData, compressedSize);
			jpeg_read_header(&cInfo, TRUE);
			jpeg_start_decompress(&cInfo);
			imageData = new unsigned char[cInfo.output_width * cInfo.output_height * 3]; // JPEG is always RGB
			unsigned char* ptr;
			for (unsigned int i = 0; i < cInfo.output_height; i++)
			{
				ptr = imageData + i * 3 * cInfo.output_width;
				jpeg_read_scanlines(&cInfo, &ptr, 1);
			}
			delete[] compressedData;
			// assign ImageSource data
			img->data = imageData;
			img->w = cInfo.output_width;
			img->h = cInfo.output_height;
			img->bpp = 3; // JPEG is always RGB
			img->format = AF_RGB; // JPEG is always RGB
		}
		return img;

	}

}
#endif