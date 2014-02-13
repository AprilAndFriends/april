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
/// Defines a generic texture.

#ifndef APRIL_TEXTURE_H
#define APRIL_TEXTURE_H

#include <gtypes/Rectangle.h>
#include <gtypes/Vector2.h>
#include <hltypes/harray.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hstring.h>

#include "aprilExport.h"
#include "Color.h"
#include "Image.h"

namespace april
{
	class Image;
	
	class aprilExport Texture
	{
	public:
		friend class RenderSystem;

		enum Type
		{
			/// @brief Cannot be modified. Texture with manual data will have a copy of the data in RAM.
			TYPE_IMMUTABLE = 1,
			/// @brief Resides in RAM and on GPU, can be modified. Best used for manually created textures or loaded from files which will be modified.
			TYPE_MANAGED = 2,
			/// @brief Used for feeding the GPU texture data constantly (e.g. video). It has no local RAM copy for when the rendering context is lost.
			TYPE_VOLATILE = 3
		};

		enum Filter
		{
			FILTER_NEAREST = 1,
			FILTER_LINEAR = 2,
			FILTER_UNDEFINED = 0x7FFFFFFF
		};

		enum AddressMode
		{
			ADDRESS_WRAP = 0,
			ADDRESS_CLAMP = 1,
			ADDRESS_UNDEFINED = 0x7FFFFFFF
		};

		DEPRECATED_ATTRIBUTE static Image::Format FORMAT_ALPHA;
		DEPRECATED_ATTRIBUTE static Image::Format FORMAT_ARGB;

		Texture();
		virtual ~Texture();
		virtual bool load();
		virtual void unload() = 0;

		HL_DEFINE_GET(hstr, filename, Filename);
		HL_DEFINE_GET(Image::Format, format, Format);
		HL_DEFINE_GETSET(Filter, filter, Filter);
		HL_DEFINE_GETSET(AddressMode, addressMode, AddressMode);
		int getWidth();
		int getHeight();
		int getBpp();
		int getByteSize();

		virtual bool isLoaded() = 0;
		
		virtual bool clear();
		virtual Color getPixel(int x, int y);
		virtual bool setPixel(int x, int y, Color color);
		Color getInterpolatedPixel(float x, float y);
		virtual bool fillRect(int x, int y, int w, int h, Color color);
		virtual bool copyPixelData(unsigned char** output, Image::Format format);
		virtual bool write(int sx, int sy, int sw, int sh, int dx, int dy, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat);
		virtual bool writeStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat);
		virtual bool blit(int sx, int sy, int sw, int sh, int dx, int dy, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat, unsigned char alpha = 255);
		virtual bool blitStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat, unsigned char alpha = 255);
		// TODOaa - rotateHue goes here
		// TODOaa - saturate goes here
		virtual bool insertAlphaMap(unsigned char* srcData, Image::Format srcFormat/*, unsigned char median, int ambiguity*/);

		virtual bool rotateHue(float degrees);
		virtual bool saturate(float factor);

		// TODOaa - new overloads
		Color getPixel(gvec2 position);
		bool setPixel(gvec2 position, Color color);
		Color getInterpolatedPixel(gvec2 position);
		bool fillRect(grect rect, Color color);
		bool copyPixelData(unsigned char** output);
		bool write(grect srcRect, gvec2 destPosition, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat);
		bool writeStretch(grect srcRect, grect destRect, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat);

		/*
		Color getPixel(gvec2 position);
		bool setPixel(gvec2 position, Color color);
		Color getInterpolatedPixel(gvec2 position);
		bool fillRect(grect rect, Color color);
		bool copyPixelData(unsigned char** output);
		bool write(int sx, int sy, int sw, int sh, int dx, int dy, Image* other);
		bool write(grect srcRect, gvec2 destPosition, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat);
		bool write(grect srcRect, gvec2 destPosition, Image* other);
		bool writeStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, Image* other);
		bool writeStretch(grect srcRect, grect destRect, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat);
		bool writeStretch(grect srcRect, grect destRect, Image* other);
		bool blit(int sx, int sy, int sw, int sh, int dx, int dy, Image* other, unsigned char alpha = 255);
		bool blit(grect srcRect, gvec2 destPosition, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat, unsigned char alpha = 255);
		bool blit(grect srcRect, gvec2 destPosition, Image* other, unsigned char alpha = 255);
		bool blitStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, Image* other, unsigned char alpha = 255);
		bool blitStretch(grect srcRect, grect destRect, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat, unsigned char alpha = 255);
		bool blitStretch(grect srcRect, grect destRect, Image* other, unsigned char alpha = 255);
		*/

		// TODOaa - old overloads
		//void blit(int x, int y, Image* image, int sx, int sy, int sw, int sh, unsigned char alpha = 255);
		//void blit(gvec2 position, Texture* texture, grect source, unsigned char alpha = 255);
		//void blit(gvec2 position, Image* image, grect source, unsigned char alpha = 255);
		//void blit(gvec2 position, unsigned char* data, int dataWidth, int dataHeight, int dataBpp, grect source, unsigned char alpha = 255);
		//void stretchBlit(int x, int y, int w, int h, Image* image, int sx, int sy, int sw, int sh, unsigned char alpha = 255);
		//void stretchBlit(grect destination, Texture* texture, grect source, unsigned char alpha = 255);
		//void stretchBlit(grect destination, Image* image, grect source, unsigned char alpha = 255);
		//void stretchBlit(grect destination, unsigned char* data, int dataWidth, int dataHeight, int dataBpp, grect source, unsigned char alpha = 255);

	protected:
		hstr filename;
		Type type;
		Image::Format format;
		unsigned int dataFormat; // used internally for special image data formatting
		int width;
		int height;
		Filter filter;
		AddressMode addressMode;
		unsigned char* data;

		virtual bool _create(chstr filename, Type type);
		virtual bool _create(int w, int h, unsigned char* data, Image::Format format, Type type);
		virtual bool _create(int w, int h, Color color, Image::Format format, Type type);

		virtual bool _createInternalTexture(unsigned char* data, int size) = 0;
		virtual void _assignFormat() = 0;

		hstr _getInternalName();

		// TODOaa - these may currently not work well with anything else than DirectX9
		//void _blit(unsigned char* thisData, int x, int y, unsigned char* data, int dataWidth, int dataHeight, int dataBpp, int sx, int sy, int sw, int sh, unsigned char alpha = 255);
		//void _stretchBlit(unsigned char* thisData, int x, int y, int w, int h, unsigned char* data, int dataWidth, int dataHeight, int dataBpp, int sx, int sy, int sw, int sh, unsigned char alpha = 255);

		virtual bool _uploadDataToGpu(int x, int y, int w, int h) = 0;

	};
	
}

#endif
