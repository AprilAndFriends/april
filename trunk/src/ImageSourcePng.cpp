/// @file
/// @author  Boris Mikic
/// @version 2.31
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifndef USE_IL
#include <stdio.h>
extern "C"
{
#ifdef __APPLE__
	#include <jpeglib.h>
#else
	#include <jpeg/jpeglib.h>
#endif
}
#ifdef _IOS
	#include <png.h>
	#include <pngpriv.h>
	#include <pngstruct.h>
#else
	#include <png/png.h>
	#include <png/pngpriv.h>
	#include <png/pngstruct.h>
#endif
#include <hltypes/hltypesUtil.h>
#include <hltypes/hresource.h>
#include <hltypes/hsbase.h>
#include <hltypes/hstream.h>

#include "ImageSource.h"
#include "RenderSystem.h"

namespace april
{
	void _pngZipRead(png_structp png, png_bytep data, png_size_t size)
	{
		hsbase* file = (hsbase*)png->io_ptr;
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
			delete [] this->data;
		}
	}

	void ImageSource::copyImage(ImageSource* source, bool fillAlpha)
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

	ImageSource* loadImage(chstr filename)
	{
		ImageSource* img = NULL;
		if (filename.lower().ends_with(".png"))
		{
			hresource file(filename);
			img = _loadImagePng(file);
		}
		else if (filename.lower().ends_with(".jpg") || filename.lower().ends_with(".jpeg"))
		{
			hresource file(filename);
			img = _loadImageJpg(file);
		}
		else if (filename.lower().ends_with(".jpt"))
		{
			hresource file(filename);
			img = _loadImageJpt(file);
		}
		return img;
	}

	ImageSource* _loadImagePng(hsbase& stream)
	{
		ImageSource* img = new ImageSource();
		png_structp pngPtr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
		png_infop infoPtr = png_create_info_struct(pngPtr);
		png_infop endInfo = png_create_info_struct(pngPtr);
		setjmp(png_jmpbuf(pngPtr));
		png_set_read_fn(pngPtr, &stream, &_pngZipRead);
		png_read_info(pngPtr, infoPtr);
		png_get_IHDR(pngPtr, infoPtr, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
		png_set_interlace_handling(pngPtr);
		int bpp = pngPtr->channels;
		if (pngPtr->color_type == PNG_COLOR_TYPE_PALETTE)
		{
			png_set_palette_to_rgb(pngPtr);
			bpp = 3;
		}
		if (png_get_valid(pngPtr, infoPtr, PNG_INFO_tRNS))
		{
			png_set_tRNS_to_alpha(pngPtr);
			bpp++;
		}
		if (pngPtr->bit_depth == 16)
		{
			png_set_strip_16(pngPtr);
		}
		png_read_update_info(pngPtr, infoPtr);
		int rowBytes = png_get_rowbytes(pngPtr, infoPtr);
		png_byte* imageData = new png_byte[rowBytes * pngPtr->height];
		png_bytep* rowPointers = new png_bytep[pngPtr->height];
		for_itert (unsigned int, i, 0, pngPtr->height)
		{
			rowPointers[i] = imageData + i * rowBytes;
		}
		png_read_image(pngPtr, rowPointers);
		png_read_end(pngPtr, infoPtr);
		// assign ImageSource data
		img->data = imageData;
		img->w = pngPtr->width;
		img->h = pngPtr->height;
		img->bpp = bpp;
		img->format = (img->bpp == 4 ? AF_RGBA : AF_RGB);
		// clean up
		png_destroy_read_struct(&pngPtr, &infoPtr, &endInfo);
		delete [] rowPointers;
		return img;
	}

	ImageSource* _loadImageJpg(hsbase& stream)
	{
		ImageSource* img = new ImageSource();
		// first read the whole data from the resource file
		int compressedSize = stream.size();
		unsigned char* compressedData = new unsigned char[compressedSize];
		stream.read_raw(compressedData, compressedSize);
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
		for_itert (unsigned int, i, 0, cInfo.output_height)
		{
			ptr = imageData + i * 3 * cInfo.output_width;
			jpeg_read_scanlines(&cInfo, &ptr, 1);
		}
		delete [] compressedData;
		// assign ImageSource data
		img->data = imageData;
		img->w = cInfo.output_width;
		img->h = cInfo.output_height;
		img->bpp = 3; // JPEG is always RGB
		img->format = AF_RGB; // JPEG is always RGB
		return img;
	}

	ImageSource* _loadImageJpt(hsbase& stream)
	{
		ImageSource* jpg;
		ImageSource* png;
		hstream subStream;
		int size;
		unsigned char bytes[4];
		unsigned char* buffer;
		// file header ("JPT" + 1 byte for version code)
		stream.read_raw(bytes, 4);
		// read JPEG
		stream.read_raw(bytes, 4);
		size = bytes[0] + bytes[1] * 256 + bytes[2] * 256 * 256 + bytes[3] * 256 * 256 * 256;
		buffer = new unsigned char[size];
		stream.read_raw(buffer, size);
		subStream.clear();
		subStream.write_raw(buffer, size);
		delete [] buffer;
		subStream.rewind();
		jpg = _loadImageJpg(subStream);
		// read PNG
		stream.read_raw(bytes, 4);
		size = bytes[0] + bytes[1] * 256 + bytes[2] * 256 * 256 + bytes[3] * 256 * 256 * 256;
		buffer = new unsigned char[size];
		stream.read_raw(buffer, size);
		subStream.clear();
		subStream.write_raw(buffer, size);
		delete [] buffer;
		subStream.rewind();
		png = _loadImagePng(subStream);
		// combine
		ImageSource* img = createEmptyImage(jpg->w, jpg->h);
		img->copyImage(jpg);
		img->insertAsAlphaMap(png);
		delete jpg;
		delete png;
		return img;
	}

}
#endif