/// @file
/// @author  Kresimir Spes
/// @author  Ivan Vucica
/// @author  Boris Mikic
/// @version 3.2
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _OPENGL
#include <hltypes/hplatform.h>
#ifdef __APPLE__
#include <TargetConditionals.h>
#endif
#if TARGET_OS_IPHONE
#include <OpenGLES/ES1/gl.h>
#include <OpenGLES/ES1/glext.h>
#elif _OPENGLES
#include <GLES/gl.h>
#else
#ifndef __APPLE__
#include <gl/GL.h>
#else
#include <OpenGL/gl.h>
#endif
#endif

#include <hltypes/hlog.h>
#include <hltypes/hstring.h>

#include "april.h"
#include "Image.h"
#include "OpenGL_RenderSystem.h"
#include "OpenGL_State.h"
#include "OpenGL_Texture.h"

#define APRIL_OGL_RENDERSYS ((OpenGL_RenderSystem*)april::rendersys)

namespace april
{
    static inline bool isPower2(int x)
    {
        return (x > 0) && ((x & (x - 1)) == 0);
    }
    
	OpenGL_Texture::OpenGL_Texture(chstr filename) : Texture()
	{
		this->format = FORMAT_ARGB;
		this->width = 0;
		this->height = 0;
		this->bpp = 4;
		this->filename = filename;
		this->textureId = 0;
		this->manualBuffer = NULL;
		hlog::write(april::logTag, "Creating GL texture: " + this->_getInternalName());
	}

	OpenGL_Texture::OpenGL_Texture(int w, int h, unsigned char* rgba) : Texture()
	{
		hlog::write(april::logTag, "Creating user-defined GL texture.");
		this->format = FORMAT_ARGB;
		this->width = w;
		this->height = h;
		this->filename = "";
		this->manualBuffer = NULL;
#ifdef _ANDROID // currently user texture caching works for Android only
		this->manualBuffer = new unsigned char[w * h * 4];
		memcpy(this->manualBuffer, rgba, w * h * 4);
#endif
		glGenTextures(1, &this->textureId);
		this->_setCurrentTexture();
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba);
        
        // Non power of 2 textures in OpenGL, must have addressing mode set to clamp, otherwise they won't work.
        if (!isPower2(w) || !isPower2(h)) setAddressMode(ADDRESS_CLAMP);
	}

	OpenGL_Texture::OpenGL_Texture(int w, int h, Format format, Type type, Color color) : Texture()
	{
		hlog::writef(april::logTag, "Creating empty GL texture [ %dx%d ].", w, h);
		this->format = format;
		this->width = w;
		this->height = h;
		switch (format)
		{
		case FORMAT_ARGB:
			this->bpp = 4;
			break;
		case FORMAT_RGB:
			this->bpp = 3;
			break;
		case FORMAT_ALPHA:
			this->bpp = 1;
			break;
#if !defined(_ANDROID) && !defined(_WIN32)
		case FORMAT_BGRA:
			this->bpp = 4;
			break;
#endif
		default:
			this->bpp = 3;
			break;
		}
		this->filename = "";
		this->manualBuffer = NULL;
		glGenTextures(1, &this->textureId);
		this->_setCurrentTexture();
		if (color != Color::Clear)
		{
			this->fillRect(0, 0, this->width, this->height, color);
		}
		else
		{
			this->clear();
		}
        
        // Non power of 2 textures in OpenGL, must have addressing mode set to clamp, otherwise they won't work.
        if (!isPower2(w) || !isPower2(h)) setAddressMode(ADDRESS_CLAMP);
	}

	OpenGL_Texture::~OpenGL_Texture()
	{
		this->unload();
		if (this->manualBuffer != NULL)
		{
			delete [] this->manualBuffer;
		}
	}

	void OpenGL_Texture::_setCurrentTexture()
	{
		APRIL_OGL_RENDERSYS->currentState.textureId = APRIL_OGL_RENDERSYS->deviceState.textureId = this->textureId;
		glBindTexture(GL_TEXTURE_2D, this->textureId);
		APRIL_OGL_RENDERSYS->currentState.textureFilter = APRIL_OGL_RENDERSYS->deviceState.textureFilter = this->filter;
		APRIL_OGL_RENDERSYS->_setTextureFilter(this->filter);
		APRIL_OGL_RENDERSYS->currentState.textureAddressMode = APRIL_OGL_RENDERSYS->deviceState.textureAddressMode = this->addressMode;
		APRIL_OGL_RENDERSYS->_setTextureAddressMode(this->addressMode);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}

	bool OpenGL_Texture::isLoaded()
	{
		return (this->textureId != 0);
	}

	bool OpenGL_Texture::load()
	{
		if (this->textureId != 0)
		{
			return true;
		}
		hlog::write(april::logTag, "Loading GL texture: " + this->_getInternalName());
		Image* image = NULL;
		if (this->filename != "")
		{
			image = Image::load(this->filename);
			if (image == NULL)
			{
				hlog::error(april::logTag, "Failed to load texture: " + this->_getInternalName());
				return false;
			}
			this->width = image->w;
			this->height = image->h;
			this->bpp = image->bpp;
		}
		glGenTextures(1, &this->textureId);
		if (this->textureId == 0)
		{
			hlog::error(april::logTag, "Failed to create GL texture!");
			return false;
		}
		// write texels
		glBindTexture(GL_TEXTURE_2D, this->textureId);
		APRIL_OGL_RENDERSYS->currentState.textureId = APRIL_OGL_RENDERSYS->deviceState.textureId = this->textureId;
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		if (image != NULL)
		{
#if TARGET_OS_IPHONE
			if (((unsigned int) image->format) == GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG || ((unsigned int) image->format) == GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG)
			{
				glCompressedTexImage2D(GL_TEXTURE_2D, 0, image->format, image->w, image->h, 0, image->compressedSize, image->data);
				this->format = FORMAT_ARGB; // TODO - not really a format
			}
			else
#endif
			{
				switch (image->format)
				{
				case Image::FORMAT_RGBA:
					glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image->w, image->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image->data);
					this->format = FORMAT_ARGB;
					break;
				case Image::FORMAT_RGB:
					glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image->w, image->h, 0, GL_RGB, GL_UNSIGNED_BYTE, image->data);
					this->format = FORMAT_RGB;
					break;
				case Image::FORMAT_GRAYSCALE:
					glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, image->w, image->h, 0, GL_ALPHA, GL_UNSIGNED_BYTE, image->data);
					this->format = FORMAT_ALPHA;
					break;
				default:
					glTexImage2D(GL_TEXTURE_2D, 0, image->bpp == 4 ? GL_RGBA : GL_RGB, image->w, image->h, 0, image->format == Image::FORMAT_RGBA ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, image->data);
					this->format = FORMAT_ARGB;
					break;
				}
			}
			delete image;
		}
		else if (this->manualBuffer != NULL)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, this->width, this->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, this->manualBuffer);
		}
		else
		{
			this->clear();
		}
        // Non power of 2 textures in OpenGL, must have addressing mode set to clamp, otherwise they won't work.
        if (!isPower2(this->width) || !isPower2(this->height)) setAddressMode(ADDRESS_CLAMP);
		return true;
	}

	void OpenGL_Texture::unload()
	{
		if (this->textureId != 0)
		{
			hlog::write(april::logTag, "Unloading GL texture: " + this->_getInternalName());
			glDeleteTextures(1, &this->textureId);
			this->textureId = 0;
		}
	}

	void OpenGL_Texture::clear()
	{
		// TODO - use as base for ::clear
		int glFormat = GL_RGB;
		int internalFormat = GL_RGB;
		switch (this->format)
		{
		case FORMAT_ARGB:
			glFormat = internalFormat = GL_RGBA;
			break;
		case FORMAT_RGB:
			glFormat = internalFormat = GL_RGB;
			break;
		case FORMAT_ALPHA:
			glFormat = internalFormat = GL_ALPHA;
			break;
#if !defined(_ANDROID) && !defined(_WIN32)
		case FORMAT_BGRA:
#ifndef __APPLE__
			glFormat = GL_BGRA;
#else
			glFormat = GL_BGRA_EXT;
#endif
			internalFormat = GL_RGBA;
			break;
#endif
		default:
			glFormat = internalFormat = GL_RGB;
			break;
		}
		unsigned char* clearColor = new unsigned char[this->width * this->height * this->bpp];
		memset(clearColor, 0, this->width * this->height * this->bpp);
		glBindTexture(GL_TEXTURE_2D, this->textureId);
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, this->width, this->height, 0, glFormat, GL_UNSIGNED_BYTE, clearColor);
		delete [] clearColor;
	}

	Color OpenGL_Texture::getPixel(int x, int y)
	{
		// TODO
		return Color::Clear;
	}

	void OpenGL_Texture::setPixel(int x, int y, Color color)
	{
		unsigned char writeData[4] = {color.r, color.g, color.b, color.a};
		glBindTexture(GL_TEXTURE_2D, this->textureId);
		APRIL_OGL_RENDERSYS->currentState.textureId = APRIL_OGL_RENDERSYS->deviceState.textureId = this->textureId;
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
		memset(writeData, 0, w * h * this->bpp); // TODO: kspes: i don't think this call is redundant, check all similar situations
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
			memset(writeData, value, w * h * this->bpp);
		}
		glBindTexture(GL_TEXTURE_2D, this->textureId);
		APRIL_OGL_RENDERSYS->currentState.textureId = APRIL_OGL_RENDERSYS->deviceState.textureId = this->textureId;
		int glFormat = GL_RGBA;
		if (this->bpp == 4)
		{
#if !defined(_ANDROID) && !defined(_WIN32)
			if (this->format == FORMAT_BGRA)
			{
#ifndef __APPLE__
				glFormat = GL_BGRA;
#else
				glFormat = GL_BGRA_EXT;
#endif
			}
			else
#endif
			{
				glFormat = GL_RGBA;
			}
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
		glBindTexture(GL_TEXTURE_2D, source->textureId);
		APRIL_OGL_RENDERSYS->currentState.textureId = APRIL_OGL_RENDERSYS->deviceState.textureId = this->textureId;
		int glFormat = GL_RGBA;
		if (source->bpp == 4)
		{
#if !defined(_ANDROID) && !defined(_WIN32)
			if (this->format == FORMAT_BGRA)
			{
#ifndef __APPLE__
				glFormat = GL_BGRA;
#else
				glFormat = GL_BGRA_EXT;
#endif
			}
			else
#endif
			{
				glFormat = GL_RGBA;
			}
		}
		else if (source->bpp == 3)
		{
			glFormat = GL_RGB;
		}
		else if (source->bpp == 1)
		{
			glFormat = GL_ALPHA;
		}
		unsigned char* readData = new unsigned char[source->width * source->height * source->bpp];
#ifndef _OPENGLES // TODO - temp until we figure out how to handle this on OpenGLES. added by kspes on May 21st 2012
		glGetTexImage(GL_TEXTURE_2D, 0, glFormat, GL_UNSIGNED_BYTE, readData);
#else
		memset(readData, 0, source->width * source->height * source->bpp);
#endif
		this->blit(x, y, readData, source->width, source->height, source->bpp, sx, sy, sw, sh, alpha);
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
		memset(writeData, 255, sw * sh * this->bpp);
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
		APRIL_OGL_RENDERSYS->currentState.textureId = APRIL_OGL_RENDERSYS->deviceState.textureId = this->textureId;
		int glFormat = GL_RGBA;
		if (this->bpp == 4)
		{
#if !defined(_ANDROID) && !defined(_WIN32)
			if (this->format == FORMAT_BGRA)
			{
#ifndef __APPLE__
				glFormat = GL_BGRA;
#else
				glFormat = GL_BGRA_EXT;
#endif
			}
			else
#endif
			{
				glFormat = GL_RGBA;
			}
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
	
	void OpenGL_Texture::write(int x, int y, unsigned char* data, int dataWidth, int dataHeight, int dataBpp)
	{
		x = hclamp(x, 0, this->width - 1);
		y = hclamp(y, 0, this->height - 1);

		glBindTexture(GL_TEXTURE_2D, this->textureId);
		APRIL_OGL_RENDERSYS->currentState.textureId = APRIL_OGL_RENDERSYS->deviceState.textureId = this->textureId;
		int glFormat = GL_RGBA;

		if (this->bpp == 4)
		{
#if !defined(_ANDROID) && !defined(_WIN32)
			if (this->format == FORMAT_BGRA)
			{
#ifndef __APPLE__
				glFormat = GL_BGRA;
#else
				glFormat = GL_BGRA_EXT;
#endif
			}
			else
#endif
			{
				glFormat = GL_RGBA;
			}
		}
		else if (this->bpp == 3)
		{
			glFormat = GL_RGB;
		}
		else if (this->bpp == 1)
		{
			glFormat = GL_ALPHA;
		}
		glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, dataWidth, dataHeight, glFormat, GL_UNSIGNED_BYTE, data);
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

}
#endif