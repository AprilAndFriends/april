/************************************************************************************\
This source file is part of the Awesome Portable Rendering Interface Library         *
For latest info, see http://libapril.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes, Ivan Vucica                                        *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#ifdef USE_IL
#include <IL/il.h>
#endif

#include <hltypes/harray.h>
#include <hltypes/hfile.h>
#include <hltypes/hstring.h>
#include <hltypes/util.h>

#include "Color.h"
#include "ImageSource.h"
#include "RenderSystem.h"

namespace april
{
	Texture::Texture()
	{
		mFilename = "";
		mUnusedTimer = 0;
		mTextureFilter = Linear;
		mTextureWrapping = true;
	}

	Texture::~Texture()
	{
		foreach (Texture*, it, mDynamicLinks)
		{
			(*it)->removeDynamicLink(this);
		}
	}
	
	Color Texture::getPixel(int x, int y)
	{
		return APRIL_COLOR_CLEAR;
	}
	
	Color Texture::getInterpolatedPixel(float x, float y)
	{
		return APRIL_COLOR_CLEAR; // TODO
	}
	
	void Texture::update(float time_increase)
	{
		if (mDynamic && isLoaded())
		{
			float max_time = rendersys->getIdleTextureUnloadTime();
			if (max_time > 0)
			{
				if (mUnusedTimer > max_time)
				{
					unload();
				}
				mUnusedTimer += time_increase;
			}
		}
	}
	
	void Texture::addDynamicLink(Texture* lnk)
	{
		if (!mDynamicLinks.contains(lnk))
		{
			mDynamicLinks += lnk;
			lnk->addDynamicLink(this);
		}
	}
	
	void Texture::removeDynamicLink(Texture* lnk)
	{
		if (mDynamicLinks.contains(lnk))
		{
			mDynamicLinks -= lnk;
		}
	}
	
	void  Texture::_resetUnusedTimer(bool recursive)
	{
		mUnusedTimer = 0;
		if (recursive)
		{
			foreach (Texture*, it, mDynamicLinks)
			{
				(*it)->_resetUnusedTimer(0);
			}
		}
	}
/************************************************************************************/
	RAMTexture::RAMTexture(chstr filename, bool dynamic)
	{
		mFilename = filename;
		mBuffer = NULL;
		if (!dynamic)
		{
			load();
		}
	}

	RAMTexture::~RAMTexture()
	{
		unload();
	}
	
	void RAMTexture::load()
	{
		if (!mBuffer)
		{
			april::log("loading RAM texture '" + mFilename + "'");
			mBuffer = loadImage(mFilename);
			mWidth = mBuffer->w;
			mHeight = mBuffer->h;
		}
	}
	
	void RAMTexture::unload()
	{
		if (mBuffer)
		{
			april::log("unloading RAM texture '" + mFilename + "'");
			delete mBuffer;
			mBuffer = NULL;
		}
	}
	
	bool RAMTexture::isLoaded()
	{
		return (mBuffer != NULL);
	}
	
	Color RAMTexture::getPixel(int x, int y)
	{
		if (!mBuffer)
		{
			load();
		}
		mUnusedTimer = 0;
		return mBuffer->getPixel(x, y);
	}
	
	void RAMTexture::setPixel(int x, int y, Color c)
	{
		if (!mBuffer)
		{
			load();
		}
		mUnusedTimer = 0;
		mBuffer->setPixel(x, y, c);
	}
	
	Color RAMTexture::getInterpolatedPixel(float x, float y)
	{
		if (!mBuffer)
		{
			load();
		}
		mUnusedTimer = 0;
		return mBuffer->getInterpolatedPixel(x, y);
	}
	
	int RAMTexture::getSizeInBytes()
	{
		return (mWidth * mHeight * 3);
	}

}
