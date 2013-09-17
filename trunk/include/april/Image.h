/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @version 3.1
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/// 
/// @section DESCRIPTION
/// 
/// Defines a generic image source.

#ifndef APRIL_IMAGE_H
#define APRIL_IMAGE_H

#include <hltypes/hsbase.h>
#include <hltypes/hstring.h>

#include "aprilExport.h"
#include "Color.h"

namespace april
{
	class Color;

	class aprilExport Image
	{
	public:
		enum Format
		{
			FORMAT_UNDEFINED = 0,
			FORMAT_RGBA = 1,
			FORMAT_RGB = 2,
			FORMAT_BGRA = 3,
			FORMAT_BGR = 4,
			FORMAT_GRAYSCALE = 5,
			FORMAT_PALETTE = 6
		};
	
		unsigned char* data;
		int w;
		int h;
		int bpp;
		Format format;
		int compressedSize;

		Image(); // TODO3 - make protected
		~Image();
		
		Color getPixel(int x, int y);
		void setPixel(int x, int y, Color c);
		Color getInterpolatedPixel(float x, float y);
		void copyPixels(void* output, Format format);
		void setPixels(int x, int y, int w, int h, Color c);
		void copyImage(Image* source, bool fillAlpha = false);
		void clear();
		void blit(int x, int y, Image* source, int sx, int sy, int sw, int sh, unsigned char alpha = 255);
		void stretchBlit(int x, int y, int w, int h, Image* source, int sx, int sy, int sw, int sh, unsigned char alpha = 255);
		void insertAsAlphaMap(Image* source);
		
		static Image* load(chstr filename);
		static Image* create(int w, int h, Color fillColor = Color::Clear);
		
	protected:
		static Image* _loadPng(hsbase& stream);
		static Image* _loadJpg(hsbase& stream);
		static Image* _loadJpt(hsbase& stream);

	};
	
	// TODO3
	//aprilFnExport Image* loadImage(chstr filename);
	//aprilFnExport Image* createEmptyImage(int w, int h);
	//aprilFnExport Image* createBlankImage(int w, int h);
	//Image* _loadImagePng(hsbase& stream);
	//Image* _loadImageJpg(hsbase& stream);
	//Image* _loadImageJpt(hsbase& stream);
	
}

#endif
