/// @file
/// @author  Domagoj Cerjan
/// @version 1.31
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <hltypes/hstring.h>
#include "ImageSource.h"
#include "OpenGL_Texture.h"

namespace april
{
	unsigned int platformLoadOpenGL_Texture(const char* name, int* w, int* h)
	{
		// mora imat vezu sa javom da bi se dobio kontinuirani niz byteova koji se moze procitati onda
	}
	
	
	AndroidGLES_Texture::AndroidGLES_Texture(chstr filename, bool dynamic)
	{
		mWidth = 0;
		mHeight = 0;
		mFilename = filename;
		mDynamic = dynamic;
		mTexId = 0;
	}

	AndroidGLES_Texture::AndroidGLES_Texture(unsigned char* rgba, int w, int h)
	{
		mWidth = w;
		mHeight = h;
		mDynamic = 0;
		mFilename = "UserTexture";
		glGenTextures(1, &mTexId);
		glBindTexture(GL_TEXTURE_2D, mTexId);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, 4, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba);
	}

	AndroidGLES_Texture::~AndroidGLES_Texture()
	{
		unload();
	}

	bool AndroidGLES_Texture::load()
	{
		mUnusedTimer = 0;
		if (mTexId)
		{
			return true;
		}
		april::log("loading GL texture '" + mFilename + "'");
		mTexId = platformLoadOpenGL_Texture(mFilename.c_str(), &mWidth, &mHeight);
		if (!mTexId)
		{
			april::log("Failed to load texture: " + mFilename);
			return false;
		}
		foreach (Texture*, it, mDynamicLinks)
		{
			((AndroidGLES_Texture*)(*it))->load();
		}
		return true;
	}

	bool AndroidGLES_Texture::isLoaded()
	{
		return (mTexId != 0);
	}

	void AndroidGLES_Texture::unload()
	{
		if (mTexId != 0)
		{
			april::log("unloading GL texture '" + mFilename + "'");
			glDeleteTextures(1, &mTexId);
			mTexId = 0;
		}
	}

	int AndroidGLES_Texture::getSizeInBytes()
	{
		return (mWidth * mHeight * 3);
	}

}

#endif
