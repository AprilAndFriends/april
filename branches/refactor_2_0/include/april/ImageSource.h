/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @version 2.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/// 
/// @section DESCRIPTION
/// 
/// Defines a generic image source.

#ifndef APRIL_IMAGE_SOURCE_H
#define APRIL_IMAGE_SOURCE_H

#include <hltypes/hsbase.h>
#include <hltypes/hstring.h>

#include "aprilExport.h"

namespace april
{
	class Color;

	enum ImageFormat
	{
		AF_UNDEFINED = 0,
		AF_RGB = 1,
		AF_RGBA = 2,
		AF_BGR = 3,
		AF_BGRA = 4,
		AF_GRAYSCALE = 5,
		AF_PALETTE = 6
	};
	
	class aprilExport ImageSource
	{
	public:
		ImageSource();
		~ImageSource();
		
		unsigned int getImageId() { return this->imageId; };
		Color getPixel(int x, int y);
		void setPixel(int x, int y, Color c);
		Color getInterpolatedPixel(float x, float y);
		void copyPixels(void* output, ImageFormat format);
		void setPixels(int x, int y, int w, int h, Color c);
		void copyImage(ImageSource* source, bool fillAlpha = false);
		void clear();
		void blit(int x, int y, ImageSource* source, int sx, int sy, int sw, int sh, unsigned char alpha = 255);
		void stretchBlit(int x, int y, int w, int h, ImageSource* source, int sx, int sy, int sw, int sh, unsigned char alpha = 255);
		void insertAsAlphaMap(ImageSource* source);
		
		unsigned char* data;
		int w;
		int h;
		int bpp;
		ImageFormat format;
		int internalFormat;

		int compressedLength;
		
	protected:
		unsigned int imageId;
		
	};
	
	aprilFnExport ImageSource* loadImage(chstr filename);
	aprilFnExport ImageSource* createEmptyImage(int w, int h);
	aprilFnExport ImageSource* createBlankImage(int w, int h);
	ImageSource* _loadImagePng(hsbase& stream);
	ImageSource* _loadImageJpg(hsbase& stream);
	ImageSource* _loadImageJpt(hsbase& stream);
	
}

#endif
