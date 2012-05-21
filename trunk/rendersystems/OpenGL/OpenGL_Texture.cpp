/// @file
/// @author  Kresimir Spes
/// @author  Ivan Vucica
/// @author  Boris Mikic
/// @version 1.8
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _OPENGL

#ifdef __APPLE__
#include <TargetConditionals.h>
#endif
#if TARGET_OS_IPHONE
#include <OpenGLES/ES1/gl.h>
#include <OpenGLES/ES1/glext.h>
#elif _OPENGLES1
#include <GLES/gl.h>
#else
#ifdef _WIN32
#include <windows.h>
#endif
#ifndef __APPLE__
#include <gl/GL.h>
#else
#include <OpenGL/gl.h>
#endif
#endif

#include <hltypes/hstring.h>

#include "april.h"
#include "ImageSource.h"
#include "OpenGL_Texture.h"

namespace april
{
	unsigned int platformLoadOpenGL_Texture(chstr name, int* w, int* h, int* bpp)
	{
		GLuint textureId = 0;
		ImageSource* img = loadImage(name);
		if (img == NULL)
		{
			return 0;
		}
		*w = img->w;
		*h = img->h;
		*bpp = img->bpp;
		glGenTextures(1, &textureId);
		glBindTexture(GL_TEXTURE_2D, textureId);
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
		case AF_RGBA:
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img->w, img->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, img->data);
			break;
		case AF_RGB:
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img->w, img->h, 0, GL_RGB, GL_UNSIGNED_BYTE, img->data);
			break;
		case AF_GRAYSCALE:
			glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, img->w, img->h, 0, GL_ALPHA, GL_UNSIGNED_BYTE, img->data);
			break;
		default:
			glTexImage2D(GL_TEXTURE_2D, 0, img->bpp == 4 ? GL_RGBA : GL_RGB, img->w, img->h, 0, img->format == AF_RGBA ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, img->data);
			break;
		}
		delete img;
		return textureId;
	}
	
	
	OpenGL_Texture::OpenGL_Texture(chstr filename, bool dynamic) : Texture()
	{
		mWidth = 0;
		mHeight = 0;
		mBpp = 4;
		mFilename = filename;
		mDynamic = dynamic;
		mTexId = 0;
		mManualData = NULL;
		if (!dynamic)
		{
			load();
		}
	}

	OpenGL_Texture::OpenGL_Texture(unsigned char* rgba, int w, int h) : Texture()
	{
		april::log("creating user-defined GL texture");
		mWidth = w;
		mHeight = h;
		mDynamic = false;
		mFilename = "";
		mManualData = NULL;
#ifdef _ANDROID // currently user texture caching works for Android only
		mManualData = new unsigned char[w * h * 4];
		memcpy(mManualData, rgba, w * h * 4 * sizeof(unsigned char));
#endif
		glGenTextures(1, &mTexId);
		glBindTexture(GL_TEXTURE_2D, mTexId);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba);
	}

	OpenGL_Texture::OpenGL_Texture(int w, int h, TextureFormat fmt, TextureType type) : Texture()
	{
		april::log("creating empty GL texture [ " + hstr(w) + "x" + hstr(h) + " ]");
		mWidth = w;
		mHeight = h;
		mDynamic = false;
		mFilename = "";
		mManualData = NULL;
		glGenTextures(1, &mTexId);
		glBindTexture(GL_TEXTURE_2D, mTexId);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		int glFormat = GL_RGB;
		mBpp = 3;
		switch (fmt)
		{
		case AT_ARGB:
			glFormat = GL_RGBA;
			mBpp = 4;
			break;
		case AT_RGB:
			glFormat = GL_RGB;
			mBpp = 3;
			break;
		case AT_ALPHA:
			glFormat = GL_ALPHA;
			mBpp = 1;
			break;
		default:
			glFormat = GL_RGB;
			mBpp = 3;
			break;
		}
		unsigned char* clearColor = new unsigned char[w * h * mBpp];
		memset(clearColor, 0, sizeof(unsigned char) * w * h * mBpp);
		glTexImage2D(GL_TEXTURE_2D, 0, glFormat, w, h, 0, glFormat, GL_UNSIGNED_BYTE, clearColor);
		delete [] clearColor;
	}

	OpenGL_Texture::~OpenGL_Texture()
	{
		unload();
		if (mManualData != NULL)
		{
			delete [] mManualData;
		}
	}

	Color OpenGL_Texture::getPixel(int x, int y)
	{
		// TODO
		return APRIL_COLOR_CLEAR;
	}

	void OpenGL_Texture::setPixel(int x, int y, Color color)
	{
		unsigned char writeData[4] = {color.r, color.g, color.b, color.a};
		glBindTexture(GL_TEXTURE_2D, mTexId);
		glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, writeData);
	}
	
	void OpenGL_Texture::fillRect(int x, int y, int w, int h, Color color)
	{
		x = hclamp(x, 0, this->mWidth - 1);
		y = hclamp(y, 0, this->mHeight - 1);
		w = hclamp(w, 1, this->mWidth - x);
		h = hclamp(h, 1, this->mHeight - y);
		// TODO - find a better and faster way to do this
		unsigned char* writeData = new unsigned char[w * h * mBpp];
		memset(writeData, 0, sizeof(unsigned char) * w * h * mBpp);
		if (mBpp == 4 || mBpp == 3)
		{
			int i;
			int j;
			for_iterx (j, 0, h)
			{
				for_iterx (i, 0, w)
				{
					writeData[(i + j * w) * mBpp + 0] = color.r;
					writeData[(i + j * w) * mBpp + 1] = color.g;
					writeData[(i + j * w) * mBpp + 2] = color.b;
					if (mBpp == 4)
					{
						writeData[(i + j * w) * mBpp + 3] = color.a;
					}
				}
			}
		}
		else if (mBpp == 1)
		{
			unsigned char value = (color.r + color.g + color.b) / 3;
			memset(writeData, value, sizeof(unsigned char) * w * h * mBpp);
		}
		glBindTexture(GL_TEXTURE_2D, mTexId);
		int glFormat = GL_RGBA;
		if (mBpp == 4)
		{
			glFormat = GL_RGBA;
		}
		else if (mBpp == 3)
		{
			glFormat = GL_RGB;
		}
		else if (mBpp == 1)
		{
			glFormat = GL_ALPHA;
		}
		glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h, glFormat, GL_UNSIGNED_BYTE, writeData);
		delete [] writeData;
	}
	
	void OpenGL_Texture::blit(int x, int y, Texture* texture, int sx, int sy, int sw, int sh, unsigned char alpha)
	{
		OpenGL_Texture* source = (OpenGL_Texture*)texture;
		x = hclamp(x, 0, mWidth - 1);
		y = hclamp(y, 0, mHeight - 1);
		sx = hclamp(sx, 0, source->mWidth - 1);
		sy = hclamp(sy, 0, source->mHeight - 1);
		sw = hmin(sw, hmin(mWidth - x, source->mWidth - sx));
		sh = hmin(sh, hmin(mHeight - y, source->mHeight - sy));
		if (sw == 1 && sh == 1)
		{
			this->setPixel(x, y, source->getPixel(sx, sy));
			return;
		}
		texture->load();
		unsigned char* readData = new unsigned char[source->mWidth * source->mHeight * source->mBpp];
		glBindTexture(GL_TEXTURE_2D, source->mTexId);
		int glFormat = GL_RGBA;
		if (source->mBpp == 4)
		{
			glFormat = GL_RGBA;
		}
		else if (source->mBpp == 3)
		{
			glFormat = GL_RGB;
		}
		else if (source->mBpp == 1)
		{
			glFormat = GL_ALPHA;
		}
		glGetTexImage(GL_TEXTURE_2D, 0, glFormat, GL_UNSIGNED_BYTE, readData);
		blit(x, y, readData, source->mWidth, source->mHeight, source->mBpp, sx, sy, sw, sh, alpha);
		delete [] readData;
	}

	void OpenGL_Texture::blit(int x, int y, unsigned char* data, int dataWidth, int dataHeight, int dataBpp, int sx, int sy, int sw, int sh, unsigned char alpha)
	{
		x = hclamp(x, 0, mWidth - 1);
		y = hclamp(y, 0, mHeight - 1);
		sx = hclamp(sx, 0, dataWidth - 1);
		sy = hclamp(sy, 0, dataHeight - 1);
		sw = hmin(sw, hmin(mWidth - x, dataWidth - sx));
		sh = hmin(sh, hmin(mHeight - y, dataHeight - sy));
		// TODO - improve this
		unsigned char* writeData = new unsigned char[sw * sh * mBpp];
		memset(writeData, 255, sizeof(unsigned char) * sw * sh * mBpp);
		int i;
		int j;
		int k;
		int minBpp = hmin(mBpp, dataBpp);
		for_iterx (j, 0, sh)
		{
			for_iterx (i, 0, sw)
			{
				for_iterx (k, 0, minBpp)
				{
					writeData[(i + j * sw) * mBpp + k] = data[(sx + i + (sy + j) * dataWidth) * dataBpp + k];
				}
			}
		}
		glBindTexture(GL_TEXTURE_2D, mTexId);
		int glFormat = GL_RGBA;
		if (mBpp == 4)
		{
			glFormat = GL_RGBA;
		}
		else if (mBpp == 3)
		{
			glFormat = GL_RGB;
		}
		else if (mBpp == 1)
		{
			glFormat = GL_ALPHA;
		}
		glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, sw, sh, glFormat, GL_UNSIGNED_BYTE, writeData);
		GLint error = glGetError();
		harray<unsigned char> xxx(writeData, dataWidth * dataHeight * dataBpp);
		delete [] writeData;
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
		fillRect(0, 0, mWidth, mHeight, APRIL_COLOR_CLEAR);
	}

	void OpenGL_Texture::rotateHue(float degrees)
	{
		// TODO
	}

	void OpenGL_Texture::saturate(float factor)
	{
		// TODO
	}

	bool OpenGL_Texture::copyPixelData(unsigned char** output)
	{
#ifndef _OPENGLES1
		load();
		glBindTexture(GL_TEXTURE_2D, mTexId);
		*output = new unsigned char[mWidth * mHeight * 4];
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, *output);
		return true;
#else
		return false;
#endif
	}
	
	bool OpenGL_Texture::load()
	{
		mUnusedTimer = 0.0f;
		if (mTexId != 0)
		{
			return true;
		}
		if (mFilename != "")
		{
			april::log("loading GL texture '" + _getInternalName() + "'");
			mTexId = platformLoadOpenGL_Texture(mFilename, &mWidth, &mHeight, &mBpp);
		}
		else
		{
			april::log("creating user-defined GL texture");
			glGenTextures(1, &mTexId);
			glBindTexture(GL_TEXTURE_2D, mTexId);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			if (mManualData != NULL)
			{
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mWidth, mHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, mManualData);
			}
		}
		if (mTexId == 0)
		{
			april::log("Failed to load texture: " + _getInternalName());
			return false;
		}
		notifyLoadingListener(this);
		foreach (Texture*, it, mDynamicLinks)
		{
			(*it)->load();
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
			april::log("unloading GL texture '" + _getInternalName() + "'");
			glDeleteTextures(1, &mTexId);
			mTexId = 0;
		}
	}

	int OpenGL_Texture::getSizeInBytes()
	{
		return (mWidth * mHeight * mBpp);
	}

	bool OpenGL_Texture::isValid()
	{
		return (mTexId != 0);
	}

}

#endif
