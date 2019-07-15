/// @file
/// @version 5.2
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifdef _OPENGL
#define __HL_INCLUDE_PLATFORM_HEADERS
#include <hltypes/hplatform.h>

#include <hltypes/hlog.h>
#include <hltypes/hstring.h>

#include "april.h"
#include "Image.h"
#include "OpenGL_RenderSystem.h"
#include "OpenGL_Texture.h"
#include "RenderState.h"

#define OGL_RENDERSYS ((OpenGL_RenderSystem*)april::rendersys)

namespace april
{
	bool OpenGL_Texture::_preventRecursion = false;

	OpenGL_Texture::OpenGL_Texture(bool fromResource) :
		Texture(fromResource),
		textureId(0),
		glFormat(0),
		internalFormat(0)
	{
	}

	bool OpenGL_Texture::_deviceCreateTexture(unsigned char* data, int size)
	{
		if (!april::rendersys->canUseLowLevelCalls())
		{
			return false;
		}
		GL_SAFE_CALL(glGenTextures, (1, &this->textureId));
		this->firstUpload = true;
		return (this->textureId != 0);
	}
	
	bool OpenGL_Texture::_deviceDestroyTexture()
	{
		if (this->textureId != 0)
		{
			if (april::rendersys->canUseLowLevelCalls())
			{
				glDeleteTextures(1, &this->textureId);
			}
			this->textureId = 0;
			this->firstUpload = true;
			return true;
		}
		return false;
	}

	void OpenGL_Texture::_assignFormat()
	{
		if (this->format == Image::Format::ARGB || this->format == Image::Format::XRGB || this->format == Image::Format::RGBA ||
			this->format == Image::Format::RGBX || this->format == Image::Format::ABGR || this->format == Image::Format::XBGR)
		{
			this->glFormat = this->internalFormat = GL_RGBA;
		}
		else if (this->format == Image::Format::BGRA || this->format == Image::Format::BGRX)
		{
#if !defined(__ANDROID__) && !defined(_WIN32)
#ifndef __APPLE__
			this->glFormat = GL_BGRA; // for optimizations
#else
			this->glFormat = GL_BGRA_EXT; // iOS doesn't accept BGR. This option hasn't been tested.
#endif
#else
			this->glFormat = GL_RGBA;
#endif
			this->internalFormat = GL_RGBA;
		}
		else if (this->format == Image::Format::RGB)
		{
			this->glFormat = this->internalFormat = GL_RGB;
		}
		else if (this->format == Image::Format::BGR)
		{
#if !defined(__ANDROID__) && !defined(_WIN32)
#ifndef __APPLE__
			this->glFormat = GL_BGR; // for optimizations
#else
			this->glFormat = GL_BGRA_EXT; // iOS doesn't accept BGR. This option hasn't been tested.
#endif
#else
			this->glFormat = GL_RGB;
#endif
			this->internalFormat = GL_RGB;
		}
		else if (this->format == Image::Format::Alpha)
		{
			this->glFormat = this->internalFormat = GL_ALPHA;
		}
		else if (this->format == Image::Format::Greyscale)
		{
			this->glFormat = this->internalFormat = GL_LUMINANCE;
		}
		else if (this->format == Image::Format::Compressed)
		{
			this->glFormat = this->internalFormat = 0; // compressed image formats will set these values as they need to
		}
		else if (this->format == Image::Format::Palette)
		{
			this->glFormat = this->internalFormat = 0; // paletted image formats will set these values as they need to
		}
		else
		{
			this->glFormat = this->internalFormat = GL_RGBA;
		}
	}

	void OpenGL_Texture::_setCurrentTexture()
	{
		// filtering and address mode applied before loading texture data, some systems are optimized to work like this (e.g. iOS OpenGLES guidelines suggest it)
		OGL_RENDERSYS->_setDeviceTextureFilter(this->filter);
		OGL_RENDERSYS->_setDeviceTextureAddressMode(this->addressMode);
		OGL_RENDERSYS->_setDeviceTexture(this);
		OGL_RENDERSYS->deviceState->texture = this;
	}

	Texture::Lock OpenGL_Texture::_tryLockSystem(int x, int y, int w, int h)
	{
		Lock lock;
		Image::Format nativeFormat = april::rendersys->getNativeTextureFormat(this->format);
		lock.activateLock(0, 0, w, h, x, y, new unsigned char[w * h * nativeFormat.getBpp()], w, h, nativeFormat);
		lock.systemBuffer = lock.data;
		return lock;
	}

	bool OpenGL_Texture::_unlockSystem(Lock& lock, bool update)
	{
		if (lock.systemBuffer == NULL)
		{
			return false;
		}
		if (update)
		{
			if (this->format != Image::Format::Compressed && this->format != Image::Format::Palette)
			{
				this->_setCurrentTexture();
				if (this->width == lock.w && this->height == lock.h)
				{
					this->_uploadPotSafeData(lock.data);
				}
				else
				{
					if (this->firstUpload)
					{
						this->_uploadPotSafeClearData();
					}
					GL_SAFE_CALL(glTexSubImage2D, (GL_TEXTURE_2D, 0, lock.dx, lock.dy, lock.w, lock.h, this->glFormat, GL_UNSIGNED_BYTE, lock.data));
				}
				this->firstUpload = false;
			}
		}
		delete[] lock.data;
		return update;
	}

	bool OpenGL_Texture::_uploadToGpu(int sx, int sy, int sw, int sh, int dx, int dy, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat)
	{
		if (this->format == Image::Format::Compressed || this->format == Image::Format::Palette)
		{
			return false;
		}
		this->_setCurrentTexture();
		if (sx == 0 && dx == 0 && sy == 0 && dy == 0 && sw == this->width && srcWidth == this->width && sh == this->height && srcHeight == this->height)
		{
			this->_uploadPotSafeData(srcData);
		}
		else
		{
			if (this->firstUpload)
			{
				this->_uploadPotSafeClearData();
			}
			int srcBpp = srcFormat.getBpp();
			if (sx == 0 && dx == 0 && sw == this->width && srcWidth == this->width)
			{
				GL_SAFE_CALL(glTexSubImage2D, (GL_TEXTURE_2D, 0, dx, dy, sw, sh, this->glFormat, GL_UNSIGNED_BYTE, &srcData[(sx + sy * srcWidth) * srcBpp]));
			}
			else
			{
				for_iter (j, 0, sh)
				{
					GL_SAFE_CALL(glTexSubImage2D, (GL_TEXTURE_2D, 0, dx, (dy + j), sw, 1, this->glFormat, GL_UNSIGNED_BYTE, &srcData[(sx + (sy + j) * srcWidth) * srcBpp]));
				}
			}
		}
		this->firstUpload = false;
		return true;
	}

	void OpenGL_Texture::_uploadPotSafeData(unsigned char* data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, this->internalFormat, this->width, this->height, 0, this->glFormat, GL_UNSIGNED_BYTE, data);
		GLenum glError = glGetError();
		SAFE_TEXTURE_UPLOAD_CHECK(glError, glTexImage2D(GL_TEXTURE_2D, 0, this->internalFormat, this->width, this->height, 0, this->glFormat, GL_UNSIGNED_BYTE, data));
		RenderSystem::Caps caps = april::rendersys->getCaps();
		if (glError == GL_INVALID_VALUE && !caps.npotTexturesLimited && !caps.npotTextures)
		{
			int w = this->width;
			int h = this->height;
			unsigned char* newData = this->_createPotData(w, h, data);
			this->_setCurrentTexture(); // has to call this again after _createPotData(), because some internal properties could have changed
			glTexImage2D(GL_TEXTURE_2D, 0, this->internalFormat, w, h, 0, this->glFormat, GL_UNSIGNED_BYTE, newData);
			glError = glGetError();
			SAFE_TEXTURE_UPLOAD_CHECK(glError, glTexImage2D(GL_TEXTURE_2D, 0, this->internalFormat, w, h, 0, this->glFormat, GL_UNSIGNED_BYTE, newData));
			delete[] newData;
		}
	}

	void OpenGL_Texture::_uploadPotSafeClearData()
	{
		int size = this->getByteSize();
		unsigned char* clearColor = new unsigned char[size];
		memset(clearColor, 0, size);
		glTexImage2D(GL_TEXTURE_2D, 0, this->internalFormat, this->width, this->height, 0, this->glFormat, GL_UNSIGNED_BYTE, clearColor);
		GLenum glError = glGetError();
		SAFE_TEXTURE_UPLOAD_CHECK(glError, glTexImage2D(GL_TEXTURE_2D, 0, this->internalFormat, this->width, this->height, 0, this->glFormat, GL_UNSIGNED_BYTE, clearColor));
		delete[] clearColor;
		RenderSystem::Caps caps = april::rendersys->getCaps();
		if (glError == GL_INVALID_VALUE && !caps.npotTexturesLimited && !caps.npotTextures)
		{
			int w = this->width;
			int h = this->height;
			clearColor = this->_createPotClearData(w, h); // can create POT sized data
			this->_setCurrentTexture(); // has to call this again after _createPotData(), because some internal properties could have changed
			glTexImage2D(GL_TEXTURE_2D, 0, this->internalFormat, this->width, this->height, 0, this->glFormat, GL_UNSIGNED_BYTE, clearColor);
			glError = glGetError();
			SAFE_TEXTURE_UPLOAD_CHECK(glError, glTexImage2D(GL_TEXTURE_2D, 0, this->internalFormat, this->width, this->height, 0, this->glFormat, GL_UNSIGNED_BYTE, clearColor));
			delete[] clearColor;
		}
	}

}
#endif
