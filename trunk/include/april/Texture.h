/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @version 1.51
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

#include <hltypes/hstring.h>
#include <hltypes/harray.h>
#include <gtypes/Rectangle.h>

#include "Color.h"
#include "aprilExport.h"

namespace april
{
	enum TextureType
	{
		AT_NORMAL = 1,
		AT_RENDER_TARGET = 2
	};

	enum TextureFormat
	{
		AT_XRGB = 1,
		AT_ARGB = 2,
		AT_RGB = 3,
		AT_RGBA = 4
	};

	class ImageSource;
	
	enum TextureFilter
	{
		Nearest = 1,
		Linear = 2
	};
	
	class aprilExport Texture
	{
	public:
		Texture();
		virtual ~Texture();
		virtual bool load() = 0;
		virtual void unload() = 0;
		virtual int getSizeInBytes() = 0;
		virtual bool isValid() = 0;
		
		virtual Color getPixel(int x, int y) { return APRIL_COLOR_CLEAR; }
		virtual void setPixel(int x, int y, Color color) { }
		virtual void fillRect(int x, int y, int w, int h, Color color) { }
		virtual void blit(int x, int y, Texture* texture, int sx, int sy, int sw, int sh, unsigned char alpha = 255) { }
		virtual void blit(int x, int y, unsigned char* data, int dataWidth, int dataHeight, int dataBpp, int sx, int sy, int sw, int sh, unsigned char alpha = 255) { }
		virtual void stretchBlit(int x, int y, int w, int h, Texture* texture, int sx, int sy, int sw, int sh, unsigned char alpha = 255) { }
		virtual void stretchBlit(int x, int y, int w, int h, unsigned char* data,int dataWidth, int dataHeight, int dataBpp, int sx, int sy, int sw, int sh, unsigned char alpha = 255) { }
		virtual void clear() { }
		virtual void rotateHue(float degrees) { }
		virtual void saturate(float factor) { }
		void fillRect(grect rect, Color color);
		virtual Color getInterpolatedPixel(float x, float y);
		
		void addDynamicLink(Texture* lnk);
		void removeDynamicLink(Texture* lnk);
		void _resetUnusedTimer(bool recursive = true);
		float getUnusedTime() { return mUnusedTimer; }
		
		int getWidth() { return mWidth; };
		int getHeight() { return mHeight; };
		int getBpp() { return mBpp; }
		/// only used with dynamic textures since at chapter load you need it's dimensions for images, but you don't know them yet
		void _setDimensions(int w, int h) { mWidth = w; mHeight = h; }
		bool isDynamic() { return mDynamic; }
		virtual bool isLoaded() = 0;
		
		void update(float time_increase);
		hstr getFilename() { return mFilename; }
		
		void setTextureFilter(TextureFilter filter) { mTextureFilter = filter; }
		void setTextureWrapping(bool wrap) { mTextureWrapping = wrap; }
		bool isTextureWrappingEnabled() { return mTextureWrapping; }
		TextureFilter getTextureFilter() { return mTextureFilter; }

		virtual void insertAsAlphaMap(Texture* source, unsigned char median, int ambiguity);
		
	protected:
		bool mDynamic;
		hstr mFilename;
		int mWidth;
		int mHeight;
		int mBpp;
		float mUnusedTimer;
		TextureFilter mTextureFilter;
		bool mTextureWrapping;
		harray<Texture*> mDynamicLinks;

		hstr _getInternalName();
		
	};
	
	class aprilExport RamTexture : public Texture
	{
	public:
		RamTexture(chstr filename, bool dynamic);
		RamTexture(int w, int h);
		virtual ~RamTexture();
		bool load();
		void unload();
		bool isLoaded();
		Color getPixel(int x, int y);
		void setPixel(int x, int y, Color c);
		Color getInterpolatedPixel(float x, float y);
		int getSizeInBytes();
		bool isValid();
		
	protected:
		ImageSource* mBuffer;
		
	};

}

#endif
