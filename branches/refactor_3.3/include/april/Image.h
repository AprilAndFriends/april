/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @version 3.3
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
		/// @note Some formats are intended to improve speed with the underlying engine if really needed. *X* formats are always 4 BPP even if the a byte is not used.
		enum Format
		{
			FORMAT_INVALID,
			FORMAT_RGBA,
			FORMAT_ARGB,
			FORMAT_BGRA,
			FORMAT_ABGR,
			FORMAT_RGBX,
			FORMAT_XRGB,
			FORMAT_BGRX,
			FORMAT_XBGR,
			FORMAT_RGB,
			FORMAT_BGR,
			FORMAT_ALPHA,
			FORMAT_GRAYSCALE,
			FORMAT_PALETTE
		};

		unsigned char* data;
		int w;
		int h;
		int bpp; // TODOaa - remove this
		Format format;
		int internalFormat; // used for special platform dependent formats, usually used internally only
		int compressedSize;

		Image(); // TODOaa - make protected
		~Image();
		
		int getBpp();
		int getByteSize();
		Color getPixel(int x, int y);
		void setPixel(int x, int y, Color c);
		Color getInterpolatedPixel(float x, float y);
		void copyPixels(void* output, Format format); // TODOaa - use similar method like in texture
		void setPixels(int x, int y, int w, int h, Color c); // TODOaa - use similar method like in texture, rename to write()
		void copyImage(Image* source, bool fillAlpha = false); // TODOaa - use similar method like in texture, rename to write()
		void clear();
		void blit(int x, int y, Image* source, int sx, int sy, int sw, int sh, unsigned char alpha = 255);
		void stretchBlit(int x, int y, int w, int h, Image* source, int sx, int sy, int sw, int sh, unsigned char alpha = 255);
		void insertAsAlphaMap(Image* source);
		
		static Image* load(chstr filename);
		static Image* create(int w, int h, Color fillColor = Color::Clear);

		static bool fillRect(int x, int y, int w, int h, Color color, unsigned char* destData, int destWidth, int destHeight, Format destFormat);
		static bool write(int sx, int sy, int sw, int sh, int dx, int dy, unsigned char* srcData, int srcWidth, int srcHeight, Format srcFormat, unsigned char* destData, int destWidth, int destHeight, Format destFormat);
		static void blit(int sx, int sy, int sw, int sh, int dx, int dy, unsigned char* srcData, int srcWidth, int srcHeight, Format srcFormat, unsigned char* destData, int destWidth, int destHeight, Format destFormat);
		static int getFormatBpp(Format format);
		static bool convertToFormat(int w, int h, unsigned char* srcData, Format srcFormat, unsigned char** destData, Format destFormat, bool preventCopy = true);
		static bool needsConversion(Format srcFormat, Format destFormat, bool preventCopy = true);
		
	protected:
		static Image* _loadPng(hsbase& stream);
		static Image* _loadJpg(hsbase& stream);
		static Image* _loadJpt(hsbase& stream);

		static bool _convertFrom1Bpp(int w, int h, unsigned char* srcData, Format srcFormat, unsigned char** destData, Format destFormat);
		static bool _convertFrom3Bpp(int w, int h, unsigned char* srcData, Format srcFormat, unsigned char** destData, Format destFormat);
		static bool _convertFrom4Bpp(int w, int h, unsigned char* srcData, Format srcFormat, unsigned char** destData, Format destFormat);

	};
	
}

#endif
