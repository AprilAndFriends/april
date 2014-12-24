/// @file
/// @version 3.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

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

#define SAFE_TEXTURE_UPLOAD_CHECK(glError, call) \
	if (glError == GL_OUT_OF_MEMORY) \
	{ \
		if (!_preventRecursion) \
		{ \
			this->_resetCurrentTexture(); \
			_preventRecursion = true; \
			hlog::warnf(april::logTag, "Not enough VRAM for %s! Calling low memory warning.", this->_getInternalName().c_str()); \
			april::window->handleLowMemoryWarning(); \
			_preventRecursion = false; \
			this->_setCurrentTexture(); \
			call; \
			glError = glGetError(); \
		} \
		if (glError == GL_OUT_OF_MEMORY) \
		{ \
			hlog::error(april::logTag, "Failed to upload texture data: Not enough VRAM!"); \
		} \
	}

namespace april
{
	static bool _preventRecursion = false;

	OpenGL_Texture::OpenGL_Texture(bool fromResource) : Texture(fromResource), textureId(0), glFormat(0), internalFormat(0), previousTextureId(0)
	{
	}

	OpenGL_Texture::~OpenGL_Texture()
	{

	}

	bool OpenGL_Texture::_createInternalTexture(unsigned char* data, int size, Type type)
	{
		glGenTextures(1, &this->textureId);
		if (this->textureId == 0)
		{
			return false;
		}
		this->firstUpload = true;
		// required first call of glTexImage2D() to prevent problems
#if TARGET_OS_IPHONE
		if (this->dataFormat == GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG || this->dataFormat == GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG)
		{
			this->_setCurrentTexture();
			glCompressedTexImage2D(GL_TEXTURE_2D, 0, this->dataFormat, this->width, this->height, 0, size, data);
			GLenum glError = glGetError();
			SAFE_TEXTURE_UPLOAD_CHECK(glError, glCompressedTexImage2D(GL_TEXTURE_2D, 0, this->dataFormat, this->width, this->height, 0, size, data));
			this->_resetCurrentTexture();
			this->firstUpload = false;
		}
#endif
		return true;
	}
	
	void OpenGL_Texture::_assignFormat()
	{
		switch (this->format)
		{
		case Image::FORMAT_ARGB:
		case Image::FORMAT_XRGB:
		case Image::FORMAT_RGBA:
		case Image::FORMAT_RGBX:
		case Image::FORMAT_ABGR:
		case Image::FORMAT_XBGR:
			this->glFormat = this->internalFormat = GL_RGBA;
			break;
		// for optimizations
		case Image::FORMAT_BGRA:
		case Image::FORMAT_BGRX:
#if !defined(_ANDROID) && !defined(_WIN32)
#ifndef __APPLE__
			this->glFormat = GL_BGRA;
#else
			this->glFormat = GL_BGRA_EXT;
#endif
#else
			this->glFormat = GL_RGBA;
#endif
			this->internalFormat = GL_RGBA;
			break;
		case Image::FORMAT_RGB:
			this->glFormat = this->internalFormat = GL_RGB;
			break;
		case Image::FORMAT_BGR:
#if !defined(_ANDROID) && !defined(_WIN32)
#ifndef __APPLE__
			this->glFormat = GL_BGR;
#else
			this->glFormat = GL_BGRA_EXT; // iOS doesn't accept BGR. this option hasn't been tested since the last refactor
#endif
#else
			this->glFormat = GL_RGB;
#endif
			this->internalFormat = GL_RGB;
			break;
		case Image::FORMAT_ALPHA:
			this->glFormat = this->internalFormat = GL_ALPHA;
			break;
		case Image::FORMAT_GRAYSCALE:
			this->glFormat = this->internalFormat = GL_LUMINANCE;
			break;
		case Image::FORMAT_PALETTE: // TODOaa - does palette use RGBA?
			this->glFormat = this->internalFormat = GL_RGBA;
			break;
		default:
			this->glFormat = this->internalFormat = GL_RGBA;
			break;
		}
	}

	void OpenGL_Texture::_setCurrentTexture()
	{
		if (this->previousTextureId == 0)
		{
			this->previousTextureId = APRIL_OGL_RENDERSYS->deviceState.textureId;
			this->previousFilter = APRIL_OGL_RENDERSYS->deviceState.textureFilter;
			this->perviousAddressMode = APRIL_OGL_RENDERSYS->deviceState.textureAddressMode;
		}
		APRIL_OGL_RENDERSYS->deviceState.textureId = this->textureId;
		glBindTexture(GL_TEXTURE_2D, this->textureId);
		APRIL_OGL_RENDERSYS->deviceState.textureFilter = this->filter;
		APRIL_OGL_RENDERSYS->_setTextureFilter(this->filter);
		APRIL_OGL_RENDERSYS->deviceState.textureAddressMode = this->addressMode;
		APRIL_OGL_RENDERSYS->_setTextureAddressMode(this->addressMode);
	}

	void OpenGL_Texture::_resetCurrentTexture()
	{
		if (this->previousTextureId != 0)
		{
			APRIL_OGL_RENDERSYS->deviceState.textureId = this->previousTextureId;
			glBindTexture(GL_TEXTURE_2D, this->previousTextureId);
			APRIL_OGL_RENDERSYS->deviceState.textureFilter = this->previousFilter;
			APRIL_OGL_RENDERSYS->_setTextureFilter(this->previousFilter);
			APRIL_OGL_RENDERSYS->deviceState.textureAddressMode = this->perviousAddressMode;
			APRIL_OGL_RENDERSYS->_setTextureAddressMode(this->perviousAddressMode);
			this->previousTextureId = 0;
		}
	}

	void OpenGL_Texture::_destroyInternalTexture()
	{
		if (this->textureId != 0)
		{
			glDeleteTextures(1, &this->textureId);
			this->textureId = 0;
		}
	}

	Texture::Lock OpenGL_Texture::_tryLockSystem(int x, int y, int w, int h)
	{
		Lock lock;
		Image::Format nativeFormat = april::rendersys->getNativeTextureFormat(this->format);
		int gpuBpp = Image::getFormatBpp(nativeFormat);
		lock.activateLock(0, 0, w, h, x, y, new unsigned char[w * h * gpuBpp], w, h, nativeFormat);
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
			if (this->format != Image::FORMAT_PALETTE)
			{
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
					else
					{
						this->_setCurrentTexture();
					}
					glTexSubImage2D(GL_TEXTURE_2D, 0, lock.dx, lock.dy, lock.w, lock.h, this->glFormat, GL_UNSIGNED_BYTE, lock.data);
				}
				this->_resetCurrentTexture();
			}
			delete[] lock.data;
			this->firstUpload = false;
		}
		return update;
	}

	void OpenGL_Texture::_uploadPotSafeData(unsigned char* data)
	{
		this->_setCurrentTexture();
		glTexImage2D(GL_TEXTURE_2D, 0, this->internalFormat, this->width, this->height, 0, this->glFormat, GL_UNSIGNED_BYTE, data);
		GLenum glError = glGetError();
		SAFE_TEXTURE_UPLOAD_CHECK(glError, glTexImage2D(GL_TEXTURE_2D, 0, this->internalFormat, this->width, this->height, 0, this->glFormat, GL_UNSIGNED_BYTE, data));
		RenderSystem::Caps caps = april::rendersys->getCaps();
		if (glError == GL_INVALID_VALUE && !caps.npotTexturesLimited && !caps.npotTextures)
		{
			int w = this->width;
			int h = this->height;
			unsigned char* newData = this->_createPotData(w, h, data);
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
		this->_setCurrentTexture();
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
			glTexImage2D(GL_TEXTURE_2D, 0, this->internalFormat, this->width, this->height, 0, this->glFormat, GL_UNSIGNED_BYTE, clearColor);
			glError = glGetError();
			SAFE_TEXTURE_UPLOAD_CHECK(glError, glTexImage2D(GL_TEXTURE_2D, 0, this->internalFormat, this->width, this->height, 0, this->glFormat, GL_UNSIGNED_BYTE, clearColor));
			delete[] clearColor;
		}
	}

	bool OpenGL_Texture::_uploadToGpu(int sx, int sy, int sw, int sh, int dx, int dy, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat)
	{
		if (this->format == Image::FORMAT_PALETTE)
		{
			return false;
		}
		this->load();
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
			else
			{
				this->_setCurrentTexture();
			}
			int srcBpp = Image::getFormatBpp(srcFormat);
			if (sx == 0 && dx == 0 && srcWidth == this->width && sw == this->width)
			{
				glTexSubImage2D(GL_TEXTURE_2D, 0, dx, dy, sw, sh, this->glFormat, GL_UNSIGNED_BYTE, &srcData[(sx + sy * srcWidth) * srcBpp]);
			}
			else
			{
				for_iter (j, 0, sh)
				{
					glTexSubImage2D(GL_TEXTURE_2D, 0, dx, (dy + j), sw, 1, this->glFormat, GL_UNSIGNED_BYTE, &srcData[(sx + (sy + j) * srcWidth) * srcBpp]);
				}
			}
		}
		this->_resetCurrentTexture();
		this->firstUpload = false;
		return true;
	}

}
#endif
