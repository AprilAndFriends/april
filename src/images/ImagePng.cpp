/// @file
/// @version 3.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <png.h>
#include <pngpriv.h>
#include <pngstruct.h>

#include <hltypes/hlog.h>
#include <hltypes/hsbase.h>
#include <hltypes/hstring.h>

#include "april.h"
#include "Image.h"

#define PNG_SIGNATURE_SIZE 8

namespace april
{
	void _pngZipRead(png_structp png, png_bytep data, png_size_t size)
	{
		((hsbase*)png->io_ptr)->readRaw(data, (int)size);
	}

	Image* Image::_loadPng(hsbase& stream, int size)
	{
		if (size < PNG_SIGNATURE_SIZE)
		{
			hlog::error(logTag, "Not a PNG file!");
			return NULL;
		}
		png_byte signature[PNG_SIGNATURE_SIZE] = { '\0' };
		stream.readRaw(signature, PNG_SIGNATURE_SIZE);
		if (png_sig_cmp(signature, 0, PNG_SIGNATURE_SIZE))
		{
			hlog::error(logTag, "Not a PNG file!");
			return NULL;
		}
		stream.seek(-PNG_SIGNATURE_SIZE, hsbase::CURRENT);
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
		if (pngPtr->color_type == PNG_COLOR_TYPE_GRAY_ALPHA && bpp > 1)
		{
			png_set_strip_alpha(pngPtr);
			--bpp;
		}
		if (png_get_valid(pngPtr, infoPtr, PNG_INFO_tRNS))
		{
			png_set_tRNS_to_alpha(pngPtr);
			++bpp;
		}
		if (pngPtr->bit_depth == 16)
		{
			png_set_strip_16(pngPtr);
		}
		png_read_update_info(pngPtr, infoPtr);
		int rowBytes = (int)png_get_rowbytes(pngPtr, infoPtr);
		png_byte* imageData = new png_byte[rowBytes * pngPtr->height];
		png_bytep* rowPointers = new png_bytep[pngPtr->height];
		for_itert (unsigned int, i, 0, pngPtr->height)
		{
			rowPointers[i] = imageData + i * rowBytes;
		}
		png_read_image(pngPtr, rowPointers);
		png_read_end(pngPtr, infoPtr);
		// assign Image data
		Image* image = new Image();
		image->data = (unsigned char*)imageData;
		image->w = pngPtr->width;
		image->h = pngPtr->height;
		switch (bpp)
		{
		case 4:
			image->format = FORMAT_RGBA;
			break;
		case 3:
			image->format = FORMAT_RGB;
			break;
		case 1:
			image->format = FORMAT_ALPHA;
			break;
		default:
			image->format = FORMAT_RGBA; // TODOaa - maybe palette should go here
			break;
		}
		// clean up
		png_destroy_read_struct(&pngPtr, &infoPtr, &endInfo);
		delete[] rowPointers;
		return image;
	}

	Image* Image::_loadPng(hsbase& stream)
	{
		return Image::_loadPng(stream, (int)stream.size());
	}

	Image* Image::_readMetaDataPng(hsbase& stream, int size)
	{
		if (size < PNG_SIGNATURE_SIZE)
		{
			hlog::error(logTag, "Not a PNG file!");
			return NULL;
		}
		png_byte signature[PNG_SIGNATURE_SIZE] = { '\0' };
		stream.readRaw(signature, PNG_SIGNATURE_SIZE);
		if (png_sig_cmp(signature, 0, PNG_SIGNATURE_SIZE))
		{
			hlog::error(logTag, "Not a PNG file!");
			return NULL;
		}
		stream.seek(-PNG_SIGNATURE_SIZE, hsbase::CURRENT);
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
		if (pngPtr->color_type == PNG_COLOR_TYPE_GRAY_ALPHA && bpp > 1)
		{
			png_set_strip_alpha(pngPtr);
			--bpp;
		}
		if (png_get_valid(pngPtr, infoPtr, PNG_INFO_tRNS))
		{
			png_set_tRNS_to_alpha(pngPtr);
			++bpp;
		}
		if (pngPtr->bit_depth == 16)
		{
			png_set_strip_16(pngPtr);
		}
		png_read_update_info(pngPtr, infoPtr);
		png_read_end(pngPtr, infoPtr);
		// assign Image data
		Image* image = new Image();
		image->data = NULL;
		image->w = pngPtr->width;
		image->h = pngPtr->height;
		switch (bpp)
		{
		case 4:
			image->format = FORMAT_RGBA;
			break;
		case 3:
			image->format = FORMAT_RGB;
			break;
		case 1:
			image->format = FORMAT_ALPHA;
			break;
		default:
			image->format = FORMAT_RGBA; // TODOaa - maybe palette should go here
			break;
		}
		// clean up
		png_destroy_read_struct(&pngPtr, &infoPtr, &endInfo);
		return image;
	}

	Image* Image::_readMetaDataPng(hsbase& stream)
	{
		return Image::_readMetaDataPng(stream, (int)stream.size());
	}

}
