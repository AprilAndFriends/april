/************************************************************************************\
This source file is part of the Awesome Portable Rendering Interface Library         *
For latest info, see http://libapril.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes, Boris Mikic                                        *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#ifndef APRIL_TEXTURE_H
#define APRIL_TEXTURE_H

#include <hltypes/hstring.h>
#include <hltypes/harray.h>

#include "Color.h"
#include "aprilExport.h"

enum TextureType
{
	AT_NORMAL = 1,
	AT_RENDER_TARGET = 2
};

enum TextureFormat
{
	AT_XRGB = 1,
	AT_ARGB = 2,
    AT_RGB = 3
};

namespace april
{
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
		virtual void unload() = 0;
		virtual int getSizeInBytes() = 0;
		
		virtual Color getPixel(int x, int y);
		virtual Color getInterpolatedPixel(float x, float y);
		
		void addDynamicLink(Texture* lnk);
		void removeDynamicLink(Texture* lnk);
		void _resetUnusedTimer(bool recursive = true);
		
		int getWidth() { return mWidth; };
		int getHeight() { return mHeight; };
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
		
	protected:
		bool mDynamic;
		hstr mFilename;
		int mWidth;
		int mHeight;
		float mUnusedTimer;
		TextureFilter mTextureFilter;
		bool mTextureWrapping;
		harray<Texture*> mDynamicLinks;
		
	};
	
	class aprilExport RAMTexture : public Texture
	{
	public:
		RAMTexture(chstr filename, bool dynamic);
		virtual ~RAMTexture();
		void load();
		void unload();
		bool isLoaded();
		Color getPixel(int x, int y);
		void setPixel(int x, int y, Color c);
		Color getInterpolatedPixel(float x, float y);
		int getSizeInBytes();
		
	protected:
		ImageSource* mBuffer;
		
	};

}

#endif
