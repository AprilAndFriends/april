/************************************************************************************\
This source file is part of the Awesome Portable Rendering Interface Library         *
For latest info, see http://libapril.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes, Ivan Vucica                                        *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#ifdef HAVE_MARMELADE

#include <hltypes/hstring.h>

#include "ImageSource.h"
#include "Marmelade_Texture.h"

namespace april
{
	unsigned int platformLoadOpenGL_Texture(const char* name, int* w, int* h)
	{
		GLuint texid;
		ImageSource* img = loadImage(name);
		if (!img)
		{
			return 0;
		}
		*w = img->w;
		*h = img->h;
		glGenTextures(1, &texid);
		glBindTexture(GL_TEXTURE_2D, texid);
		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		
		switch (img->format)
		{
			glTexImage2D(GL_TEXTURE_2D,
				0,
				img->bpp == 4 ? GL_RGBA : GL_RGB,
				img->w,
				img->h, 
				0, 
				img->format, 
				GL_UNSIGNED_BYTE,
				img->data);
			break;
		}
		delete img;
		
		return texid;
	}
	
	
	Marmelade_Texture::Marmelade_Texture(chstr filename, bool dynamic)
	{
		mWidth = 0;
		mHeight = 0;
		mFilename = filename;
		mDynamic = dynamic;
		mTexId = 0;
	}

	Marmelade_Texture::Marmelade_Texture(unsigned char* rgba, int w, int h)
	{
		mWidth = w;
		mHeight = h;
		mDynamic = 0;
		mFilename = "UserTexture";
		glGenTextures(1, &mTexId);
		glBindTexture(GL_TEXTURE_2D, mTexId);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba);
	}

	Marmelade_Texture::~Marmelade_Texture()
	{
		unload();
	}

	bool Marmelade_Texture::load()
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
			((Marmelade_Texture*)(*it))->load();
		}
		return true;
	}

	bool Marmelade_Texture::isLoaded()
	{
		return (mTexId != 0);
	}

	void Marmelade_Texture::unload()
	{
		if (mTexId != 0)
		{
			april::log("unloading GL texture '" + mFilename + "'");
			glDeleteTextures(1, &mTexId);
			mTexId = 0;
		}
	}

	int Marmelade_Texture::getSizeInBytes()
	{
		return (mWidth * mHeight * 3);
	}

}

#endif
