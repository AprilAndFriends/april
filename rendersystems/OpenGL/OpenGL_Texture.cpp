/// @file
/// @author  Kresimir Spes
/// @author  Ivan Vucica
/// @author  Boris Mikic
/// @version 2.0
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
	OpenGL_Texture::OpenGL_Texture(chstr filename, bool dynamic) : Texture()
	{
		this->width = 0;
		this->height = 0;
		this->bpp = 4;
		this->filename = filename;
		this->dynamic = dynamic;
		this->textureId = 0;
		this->manualBuffer = NULL;
		if (!this->dynamic)
		{
			this->load();
		}
		else
		{
			april::log("creating dynamic GL texture");
		}
	}

	OpenGL_Texture::OpenGL_Texture(int w, int h, unsigned char* rgba) : Texture()
	{
		april::log("creating user-defined GL texture");
		this->width = w;
		this->height = h;
		this->dynamic = false;
		this->filename = "";
		this->manualBuffer = NULL;
#ifdef _ANDROID // currently user texture caching works for Android only
		this->manualBuffer = new unsigned char[w * h * 4];
		memcpy(this->manualBuffer, rgba, w * h * 4 * sizeof(unsigned char));
#endif
		glGenTextures(1, &this->textureId);
		glBindTexture(GL_TEXTURE_2D, this->textureId);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba);
	}

	OpenGL_Texture::OpenGL_Texture(int w, int h, Format format, Type type, Color color) : Texture()
	{
		april::log("creating empty GL texture [ " + hstr(w) + "x" + hstr(h) + " ]");
		this->width = w;
		this->height = h;
		this->bpp = 4;
		this->dynamic = false;
		this->filename = "";
		this->manualBuffer = NULL;
		glGenTextures(1, &this->textureId);
		glBindTexture(GL_TEXTURE_2D, this->textureId);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		if (color != APRIL_COLOR_CLEAR)
		{
			this->fillRect(0, 0, this->width, this->height, color);
		}
		else
		{
			// TODO - use as base for ::clear
			int glFormat = GL_RGB;
			this->bpp = 3;
			switch (format)
			{
			case FORMAT_ARGB:
				glFormat = GL_RGBA;
				this->bpp = 4;
				break;
			case FORMAT_RGB:
				glFormat = GL_RGB;
				this->bpp = 3;
				break;
			case FORMAT_ALPHA:
				glFormat = GL_ALPHA;
				this->bpp = 1;
				break;
			default:
				glFormat = GL_RGB;
				this->bpp = 3;
				break;
			}
			unsigned char* clearColor = new unsigned char[w * h * this->bpp];
			memset(clearColor, 0, sizeof(unsigned char) * w * h * this->bpp);
			glTexImage2D(GL_TEXTURE_2D, 0, glFormat, w, h, 0, glFormat, GL_UNSIGNED_BYTE, clearColor);
			delete [] clearColor;
		}
	}

	OpenGL_Texture::~OpenGL_Texture()
	{
		this->unload();
		if (this->manualBuffer != NULL)
		{
			delete [] this->manualBuffer;
		}
	}

	bool OpenGL_Texture::isLoaded()
	{
		return (this->textureId != 0);
	}

	bool OpenGL_Texture::load()
	{
		this->unusedTime = 0.0f;
		if (this->textureId != 0)
		{
			return true;
		}
		april::log("loading GL texture '" + this->_getInternalName() + "'");
		ImageSource* image = NULL;
		if (this->filename != "")
		{
			image = april::loadImage(this->filename);
			if (image == NULL)
			{
				april::log("Failed to load texture '" + this->_getInternalName() + "'!");
				return false;
			}
			this->width = image->w;
			this->height = image->h;
			this->bpp = image->bpp;
		}
		glGenTextures(1, &this->textureId);
		if (this->textureId == 0)
		{
			april::log("failed to create GL texture");
			return false;
		}
		// write texels
		glBindTexture(GL_TEXTURE_2D, this->textureId);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		if (image != NULL)
		{
			switch (image->format)
			{
#if TARGET_OS_IPHONE
			case GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG:
			case GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG:
				glCompressedTexImage2D(GL_TEXTURE_2D, 0, image->format, image->w, image->h, 0, image->compressedLength, image->data);
				break;
#endif
			case AF_RGBA:
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image->w, image->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image->data);
				break;
			case AF_RGB:
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image->w, image->h, 0, GL_RGB, GL_UNSIGNED_BYTE, image->data);
				break;
			case AF_GRAYSCALE:
				glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, image->w, image->h, 0, GL_ALPHA, GL_UNSIGNED_BYTE, image->data);
				break;
			default:
				glTexImage2D(GL_TEXTURE_2D, 0, image->bpp == 4 ? GL_RGBA : GL_RGB, image->w, image->h, 0, image->format == AF_RGBA ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, image->data);
				break;
			}
			delete image;
		}
		else if (this->manualBuffer != NULL)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, this->width, this->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, this->manualBuffer);
		}
		foreach (Texture*, it, this->dynamicLinks)
		{
			(*it)->load();
		}
		return true;
	}

	void OpenGL_Texture::unload()
	{
		if (this->textureId != 0)
		{
			april::log("unloading GL texture '" + this->_getInternalName() + "'");
			glDeleteTextures(1, &this->textureId);
			this->textureId = 0;
		}
	}

	/////////////////////////////////////////////////////////////

	void OpenGL_Texture::clear()
	{
		// TODO - can be improved by directly using memset
		this->fillRect(0, 0, this->width, this->height, APRIL_COLOR_CLEAR);
	}

	Color OpenGL_Texture::getPixel(int x, int y)
	{
		// TODO
		return APRIL_COLOR_CLEAR;
	}

	void OpenGL_Texture::setPixel(int x, int y, Color color)
	{
		unsigned char writeData[4] = {color.r, color.g, color.b, color.a};
		glBindTexture(GL_TEXTURE_2D, this->textureId);
		glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, writeData);
	}
	
	void OpenGL_Texture::fillRect(int x, int y, int w, int h, Color color)
	{
		x = hclamp(x, 0, this->width - 1);
		y = hclamp(y, 0, this->height - 1);
		w = hclamp(w, 1, this->width - x);
		h = hclamp(h, 1, this->height - y);
		// TODO - find a better and faster way to do this
		unsigned char* writeData = new unsigned char[w * h * this->bpp];
		memset(writeData, 0, sizeof(unsigned char) * w * h * this->bpp);
		if (this->bpp == 4 || this->bpp == 3)
		{
			int i;
			int j;
			for_iterx (j, 0, h)
			{
				for_iterx (i, 0, w)
				{
					writeData[(i + j * w) * this->bpp + 0] = color.r;
					writeData[(i + j * w) * this->bpp + 1] = color.g;
					writeData[(i + j * w) * this->bpp + 2] = color.b;
					if (this->bpp == 4)
					{
						writeData[(i + j * w) * this->bpp + 3] = color.a;
					}
				}
			}
		}
		else if (this->bpp == 1)
		{
			unsigned char value = (color.r + color.g + color.b) / 3;
			memset(writeData, value, sizeof(unsigned char) * w * h * this->bpp);
		}
		glBindTexture(GL_TEXTURE_2D, this->textureId);
		int glFormat = GL_RGBA;
		if (this->bpp == 4)
		{
			glFormat = GL_RGBA;
		}
		else if (this->bpp == 3)
		{
			glFormat = GL_RGB;
		}
		else if (this->bpp == 1)
		{
			glFormat = GL_ALPHA;
		}
		glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h, glFormat, GL_UNSIGNED_BYTE, writeData);
		delete [] writeData;
	}
	
	void OpenGL_Texture::blit(int x, int y, Texture* texture, int sx, int sy, int sw, int sh, unsigned char alpha)
	{
		OpenGL_Texture* source = (OpenGL_Texture*)texture;
		x = hclamp(x, 0, this->width - 1);
		y = hclamp(y, 0, this->height - 1);
		sx = hclamp(sx, 0, source->width - 1);
		sy = hclamp(sy, 0, source->height - 1);
		sw = hmin(sw, hmin(this->width - x, source->width - sx));
		sh = hmin(sh, hmin(this->height - y, source->height - sy));
		if (sw == 1 && sh == 1)
		{
			this->setPixel(x, y, source->getPixel(sx, sy));
			return;
		}
		texture->load();
		unsigned char* readData = new unsigned char[source->width * source->height * source->bpp];
		glBindTexture(GL_TEXTURE_2D, source->textureId);
		int glFormat = GL_RGBA;
		if (source->bpp == 4)
		{
			glFormat = GL_RGBA;
		}
		else if (source->bpp == 3)
		{
			glFormat = GL_RGB;
		}
		else if (source->bpp == 1)
		{
			glFormat = GL_ALPHA;
		}
#ifndef _OPENGLES1 // TODO - temp until we figure out how to handle this on OpenGLES. added by kspes on May 21st 2012
		glGetTexImage(GL_TEXTURE_2D, 0, glFormat, GL_UNSIGNED_BYTE, readData);
#endif
		blit(x, y, readData, source->width, source->height, source->bpp, sx, sy, sw, sh, alpha);
		delete [] readData;
	}

	void OpenGL_Texture::blit(int x, int y, unsigned char* data, int dataWidth, int dataHeight, int dataBpp, int sx, int sy, int sw, int sh, unsigned char alpha)
	{
		x = hclamp(x, 0, this->width - 1);
		y = hclamp(y, 0, this->height - 1);
		sx = hclamp(sx, 0, dataWidth - 1);
		sy = hclamp(sy, 0, dataHeight - 1);
		sw = hmin(sw, hmin(this->width - x, dataWidth - sx));
		sh = hmin(sh, hmin(this->height - y, dataHeight - sy));
		// TODO - improve this
		unsigned char* writeData = new unsigned char[sw * sh * this->bpp];
		memset(writeData, 255, sizeof(unsigned char) * sw * sh * this->bpp);
		int i;
		int j;
		int k;
		int minBpp = hmin(this->bpp, dataBpp);
		if (this->bpp >= 3 && dataBpp >= 3)
		{
			for_iterx (j, 0, sh)
			{
				for_iterx (i, 0, sw)
				{
					for_iterx (k, 0, minBpp)
					{
						writeData[(i + j * sw) * this->bpp + k] = data[(sx + i + (sy + j) * dataWidth) * dataBpp + k];
					}
				}
			}
		}
		else
		{
			for_iterx (j, 0, sh)
			{
				for_iterx (i, 0, sw)
				{
					writeData[i + j * sw] = data[sx + i + (sy + j) * dataWidth];
				}
			}
		}
		glBindTexture(GL_TEXTURE_2D, this->textureId);
		int glFormat = GL_RGBA;
		if (this->bpp == 4)
		{
			glFormat = GL_RGBA;
		}
		else if (this->bpp == 3)
		{
			glFormat = GL_RGB;
		}
		else if (this->bpp == 1)
		{
			glFormat = GL_ALPHA;
		}
		glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, sw, sh, glFormat, GL_UNSIGNED_BYTE, writeData);
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
		this->load();
		glBindTexture(GL_TEXTURE_2D, this->textureId);
		*output = new unsigned char[this->width * this->height * this->bpp];
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, *output);
		return true;
#else
		return false;
#endif
	}
	
}

#endif
