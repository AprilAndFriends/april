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
			unsigned char* clearColor = new unsigned char[w * h * 4];
			memset(clearColor, 0, sizeof(unsigned char) * w * h * 4);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, clearColor);
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
			this->bpp = (image->bpp == 4 ? 4 : 3);
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
				glCompressedTexImage2D(GL_TEXTURE_2D, 0, img->format, img->w, img->h, 0, img->compressedLength, img->data);
				break;
#endif
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
		// TODO
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
		// TODO - find a better and faster way to do this
		unsigned char* writeData = new unsigned char[w * h * 4];
		memset(writeData, 0, sizeof(unsigned char) * w * h * 4);
		for_iter (j, 0, h)
		{
			for_iter (i, 0, w)
			{
				writeData[(i + j * w) * 4 + 0] = color.r;
				writeData[(i + j * w) * 4 + 1] = color.g;
				writeData[(i + j * w) * 4 + 2] = color.b;
				writeData[(i + j * w) * 4 + 3] = color.a;
			}
		}
		glBindTexture(GL_TEXTURE_2D, this->textureId);
		glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h, GL_RGBA, GL_UNSIGNED_BYTE, writeData);
		delete [] writeData;
	}
	
	void OpenGL_Texture::blit(int x, int y, Texture* texture, int sx, int sy, int sw, int sh, unsigned char alpha)
	{
		// TODO - find a better and faster way to do this
		/*
		glBindTexture(GL_TEXTURE_2D, ((OpenGL_Texture*)texture)->this->textureId);
		glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, sx, sy, sw, sh);
		unsigned char* writeData = new unsigned char[sw * sh * 4];
		glReadPixels(0, 0, sw, sh, GL_RGBA, GL_UNSIGNED_BYTE, writeData);
		glBindTexture(GL_TEXTURE_2D, this->textureId);
		glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, sw, sh, GL_RGBA, GL_UNSIGNED_BYTE, writeData);
		delete [] writeData;
		//*/
	}

	void OpenGL_Texture::blit(int x, int y, unsigned char* data, int dataWidth, int dataHeight, int dataBpp, int sx, int sy, int sw, int sh, unsigned char alpha)
	{
		// TODO - find a better and faster way to do this
		unsigned char* writeData = new unsigned char[sw * sh * 4];
		memset(writeData, 0, sizeof(unsigned char) * sw * sh * 4);
		for_iter (j, 0, sh)
		{
			for_iter (i, 0, sw)
			{
				for_iter (k, 0, dataBpp)
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
		glBindTexture(GL_TEXTURE_2D, this->textureId);
		glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, sw, sh, GL_RGBA, GL_UNSIGNED_BYTE, writeData);
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
		load();
		glBindTexture(GL_TEXTURE_2D, mTexId);
		*output = new unsigned char[mWidth * mHeight * 4];
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, *output);
		return true;
	}
	
}

#endif
