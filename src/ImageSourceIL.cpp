/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @version 1.8
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef USE_IL
#include <IL/il.h>

#include <hltypes/hltypesUtil.h>
#include <hltypes/hresource.h>
#include <hltypes/hstream.h>

#include "ImageSource.h"
#include "RenderSystem.h"

namespace april
{
	ImageSource::ImageSource()
	{
		ilGenImages(1, &mImageId);
		this->compressedLength = 0;
	}
	
	ImageSource::~ImageSource()
	{
		ilDeleteImages(1, &mImageId);
	}

	void ImageSource::copyImage(ImageSource* source, bool fillAlpha)
	{
		ilBindImage(source->getImageId());
		ilCopyPixels(0, 0, 0, this->w, this->h, 1, this->internalFormat, IL_UNSIGNED_BYTE, this->data);
	}

	ImageSource* _loadImageJpt(chstr filename)
	{
		ImageSource* jpg = new ImageSource();
		ImageSource* png = new ImageSource();
		hresource stream(filename);
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
		ilBindImage(jpg->getImageId());
		ilLoadL(IL_JPG, buffer, size);
		delete [] buffer;
		jpg->w = ilGetInteger(IL_IMAGE_WIDTH);
		jpg->h = ilGetInteger(IL_IMAGE_HEIGHT);
		jpg->bpp = ilGetInteger(IL_IMAGE_BPP);
		jpg->data = ilGetData();
		jpg->internalFormat = ilGetInteger(IL_IMAGE_FORMAT);
		jpg->format = (jpg->internalFormat == 6408 ? AF_RGBA : AF_RGB);
		// read PNG
		stream.read_raw(bytes, 4);
		size = bytes[0] + bytes[1] * 256 + bytes[2] * 256 * 256 + bytes[3] * 256 * 256 * 256;
		buffer = new unsigned char[size];
		stream.read_raw(buffer, size);
		ilBindImage(png->getImageId());
		ilLoadL(IL_PNG, buffer, size);
		delete [] buffer;
		png->w = ilGetInteger(IL_IMAGE_WIDTH);
		png->h = ilGetInteger(IL_IMAGE_HEIGHT);
		png->bpp = ilGetInteger(IL_IMAGE_BPP);
		png->data = ilGetData();
		png->internalFormat = ilGetInteger(IL_IMAGE_FORMAT);
		png->format = (png->internalFormat == 6408 ? AF_RGBA : AF_RGB);
		// combine
		ImageSource* img = createEmptyImage(jpg->w, jpg->h);
		img->copyImage(jpg);
		img->insertAsAlphaMap(png);
		delete jpg;
		delete png;
		return img;
	}

	ImageSource* loadImage(chstr filename)
	{
		if (filename.lower().ends_with(".jpt"))
		{
			return _loadImageJpt(filename);
		}
		ImageSource* img = new ImageSource();
		ilBindImage(img->getImageId());
		int success = ilLoadImage(filename.c_str());
		if (!success)
		{
			delete img;
			return NULL;
		}
		img->w = ilGetInteger(IL_IMAGE_WIDTH);
		img->h = ilGetInteger(IL_IMAGE_HEIGHT);
		img->bpp = ilGetInteger(IL_IMAGE_BPP);
		img->data = ilGetData();
		img->internalFormat = ilGetInteger(IL_IMAGE_FORMAT);
		img->format = AF_UNDEFINED;
		switch (img->internalFormat)
		{
		case IL_RGBA:
			img->format = AF_RGBA;
			break;
		case IL_RGB:
			img->format = AF_RGB;
			break;
		case IL_BGR:
			img->format = AF_BGR;
			break;
		case IL_BGRA:
			img->format = AF_BGRA;
			break;
		case IL_LUMINANCE:
			img->format = AF_GRAYSCALE;
			break;
		case IL_COLOUR_INDEX:
			img->format = AF_PALETTE;
			break;
		default:
			img->format = AF_UNDEFINED;
			break;
		}
		return img;
	}

}
#endif
