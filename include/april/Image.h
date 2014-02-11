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

#include <gtypes/Vector2.h>
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
		Format format;
		int internalFormat; // used for special platform dependent formats, usually used internally only
		int compressedSize;

		~Image();
		
		int getBpp();
		int getByteSize();

		void clear();
		Color getPixel(int x, int y);
		void setPixel(int x, int y, Color color);
		Color getInterpolatedPixel(float x, float y);
		void fillRect(int x, int y, int w, int h, Color color);
		void write(int sx, int sy, int sw, int sh, int dx, int dy, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat);
		bool copyPixelData(unsigned char** output, Format format);
		// TODOaa - blit goes here
		// TODOaa - stretchBlit goes here
		// TODOaa - rotateHue goes here
		// TODOaa - saturate goes here
		void insertAlphaMap(unsigned char* srcData, Format srcFormat);
		
		Color getPixel(gvec2 position);
		void setPixel(gvec2 position, Color color);
		Color getInterpolatedPixel(gvec2 position);
		void write(int sx, int sy, int sw, int sh, int dx, int dy, Image* other);
		bool copyPixelData(unsigned char** output);
		void insertAlphaMap(Image* source);

		// TODOaa - need a new/better implementation
		void blit(int x, int y, Image* source, int sx, int sy, int sw, int sh, unsigned char alpha = 255);
		void stretchBlit(int x, int y, int w, int h, Image* source, int sx, int sy, int sw, int sh, unsigned char alpha = 255);

		static Image* load(chstr filename);
		static Image* load(chstr filename, Format format);
		static Image* create(int w, int h, unsigned char* data, Format format);
		static Image* create(int w, int h, Color color, Format format);
		static Image* create(Image* other);

		static int getFormatBpp(Format format);

		static Color getPixel(int x, int y, unsigned char* srcData, int srcWidth, int srcHeight, Format srcFormat);
		static bool setPixel(int x, int y, Color color, unsigned char* destData, int destWidth, int destHeight, Format destFormat);
		static Color getInterpolatedPixel(float x, float y, unsigned char* srcData, int srcWidth, int srcHeight, Format srcFormat);
		static bool fillRect(int x, int y, int w, int h, Color color, unsigned char* destData, int destWidth, int destHeight, Format destFormat);
		static bool write(int sx, int sy, int sw, int sh, int dx, int dy, unsigned char* srcData, int srcWidth, int srcHeight, Format srcFormat, unsigned char* destData, int destWidth, int destHeight, Format destFormat);
		static bool blit(int sx, int sy, int sw, int sh, int dx, int dy, unsigned char* srcData, int srcWidth, int srcHeight, Format srcFormat, unsigned char* destData, int destWidth, int destHeight, Format destFormat);
		static bool insertAlphaMap(int w, int h, unsigned char* srcData, Format srcFormat, unsigned char* destData, Format destFormat);

		/// @param[in] preventCopy If true, will make a copy even if source and destination formats are the same.
		static bool convertToFormat(int w, int h, unsigned char* srcData, Format srcFormat, unsigned char** destData, Format destFormat, bool preventCopy = true);
		/// @brief Checks if an image format conversion is needed.
		/// @param[in] preventCopy If true, will return false if source and destination formats are the same.
		/// @note Helps to determine whether there is a need to convert an image format into another. It can be helpful to avoid conversion from e.g. RGBA to RGBX if the GPU ignores the X anyway.
		static bool needsConversion(Format srcFormat, Format destFormat, bool preventCopy = true);
		
	protected:
		Image();

		static Image* _loadPng(hsbase& stream);
		static Image* _loadJpg(hsbase& stream);
		static Image* _loadJpt(hsbase& stream);

		static bool _convertFrom1Bpp(int w, int h, unsigned char* srcData, Format srcFormat, unsigned char** destData, Format destFormat);
		static bool _convertFrom3Bpp(int w, int h, unsigned char* srcData, Format srcFormat, unsigned char** destData, Format destFormat);
		static bool _convertFrom4Bpp(int w, int h, unsigned char* srcData, Format srcFormat, unsigned char** destData, Format destFormat);

	};
	
}

#endif
