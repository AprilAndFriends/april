/// @file
/// @author  Kresimir Spes
/// @author  Ivan Vucica
/// @author  Boris Mikic
/// @version 1.33
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _OPENGL

#ifdef IPHONE_PLATFORM
#include <OpenGLES/ES1/gl.h>
#elif _OPENGLES1
#include <GLES/gl.h>
#else
#ifdef _WIN32
#include <windows.h>
#endif
#ifndef __APPLE__
#if HAVE_GLUT
#include <gl/GLUT.h>
#endif
#include <gl/GL.h>
#include <gl/GLU.h>
#else // __APPLE__
#include <TargetConditionals.h>
#if TARGET_OS_IPHONE
#include <OpenGLES/ES1/gl.h>
#include <OpenGLES/ES1/glext.h>
#else
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#endif
#endif // __APPLE__
#endif

#include <hltypes/hstring.h>

#include "ImageSource.h"
#include "OpenGL_Texture.h"

namespace april
{
	unsigned int platformLoadOpenGL_Texture(const char* name, int* w, int* h)
	{
		GLuint texid = 0;
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
#if TARGET_OS_IPHONE
		case GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG:
		case GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG:
		{
			int mipmaplevel = 0;
			glCompressedTexImage2D(GL_TEXTURE_2D, mipmaplevel, img->format, img->w, img->h, 0, img->compressedLength, img->data);
			break;
		}
#endif
		default:
			glTexImage2D(GL_TEXTURE_2D, 0, img->bpp == 4 ? GL_RGBA : GL_RGB, img->w, img->h, 0, img->format, GL_UNSIGNED_BYTE, img->data);
			break;
		}
		delete img;
		
		return texid;
	}
	
	
	OpenGL_Texture::OpenGL_Texture(chstr filename, bool dynamic) : Texture()
	{
		mWidth = 0;
		mHeight = 0;
		mFilename = filename;
		mDynamic = dynamic;
		mTexId = 0;
	}

	OpenGL_Texture::OpenGL_Texture(unsigned char* rgba, int w, int h) : Texture()
	{
		april::log("Creating user-defined GL texture");
		mWidth = w;
		mHeight = h;
		mDynamic = false;
		mFilename = "UserTexture";
		glGenTextures(1, &mTexId);
		glBindTexture(GL_TEXTURE_2D, mTexId);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba);
	}

	OpenGL_Texture::OpenGL_Texture(int w, int h) : Texture()
	{
		april::log("Creating empty GL texture");
		mWidth = w;
		mHeight = h;
		mDynamic = false;
		mFilename = "UserTexture";
		glGenTextures(1, &mTexId);
		glBindTexture(GL_TEXTURE_2D, mTexId);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		unsigned char* clearColor = new unsigned char[w * h * 4];
		memset(clearColor, 0, sizeof(unsigned char) * w * h * 4);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, clearColor);
		delete clearColor;
	}

	OpenGL_Texture::~OpenGL_Texture()
	{
		unload();
	}

	Color OpenGL_Texture::getPixel(int x, int y)
	{
		// TODO
		return APRIL_COLOR_CLEAR;
	}

	void OpenGL_Texture::setPixel(int x, int y, Color color)
	{
		// TODO
	}
	
	void OpenGL_Texture::fillRect(int x, int y, int w, int h, Color color)
	{
		// TODO
	}
	
	void OpenGL_Texture::blit(int x, int y, Texture* texture, int sx, int sy, int sw, int sh, unsigned char alpha)
	{
		// TODO
	}

	void OpenGL_Texture::blit(int x, int y, unsigned char* data, int dataWidth, int dataHeight, int dataBpp, int sx, int sy, int sw, int sh, unsigned char alpha)
	{
		unsigned char* writeData = new unsigned char[sw * sh * 4];
		memset(writeData, 0, sizeof(unsigned char) * sw * sh * 4);
		for (int j = 0; j < sh; j++)
		{
			for (int i = 0; i < sw; i++)
			{
				for (int k = 0; k < dataBpp; k++)
				{
					writeData[(i + j * sw) * 4 + k] = data[(sx + i + (sy + j * dataWidth)) * dataBpp + k];
				}
				if (dataBpp < 4)
				{
					writeData[(i + j * sw) * 4 + 3] = alpha;
				}
				else
				{
					writeData[(i + j * sw) * 4 + 3] = alpha * writeData[(i + j * sw) * 4 + 3] / 255;
				}
			}
		}
		glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, sw, sh, GL_RGBA, GL_UNSIGNED_BYTE, writeData);
		delete writeData;
	}

	void OpenGL_Texture::stretchBlit(int x, int y, int w, int h, Texture* texture, int sx, int sy, int sw, int sh, unsigned char alpha)
	{
		// TODO
	}

	void OpenGL_Texture::stretchBlit(int x, int y, int w, int h, unsigned char* data, int dataWidth, int dataHeight, int dataBpp, int sx, int sy, int sw, int sh, unsigned char alpha)
	{
		// TODO
	}

	void OpenGL_Texture::clear()
	{
		// TODO
	}

	void OpenGL_Texture::rotateHue(float degrees)
	{
		// TODO
	}

	void OpenGL_Texture::saturate(float factor)
	{
		// TODO
	}
	
	bool OpenGL_Texture::load()
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
			((OpenGL_Texture*)(*it))->load();
		}
		return true;
	}

	bool OpenGL_Texture::isLoaded()
	{
		return (mTexId != 0);
	}

	void OpenGL_Texture::unload()
	{
		if (mTexId != 0)
		{
			april::log("unloading GL texture '" + mFilename + "'");
			glDeleteTextures(1, &mTexId);
			mTexId = 0;
		}
	}

	int OpenGL_Texture::getSizeInBytes()
	{
		return (mWidth * mHeight * 3);
	}

}

#endif