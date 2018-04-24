/// @file
/// @version 4.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#define __HL_INCLUDE_PLATFORM_HEADERS
#include <hltypes/hplatform.h>

#include <gl/GL.h>
#define GL_GLEXT_PROTOTYPES
#include <gl/glext.h>

#include <hltypes/hlog.h>
#include <hltypes/hstring.h>
#include <april/RenderState.h>

#include "CustomRenderSystem.h"
#include "CustomTexture.h"

#define CUSTOM_RENDERSYS ((CustomRenderSystem*)april::rendersys)

CustomTexture::CustomTexture(bool fromResource) : Texture(fromResource), textureId(0), glFormat(0), internalFormat(0)
{
}

CustomTexture::~CustomTexture()
{
}

bool CustomTexture::_deviceCreateTexture(unsigned char* data, int size)
{
	glGenTextures(1, &this->textureId);
	if (this->textureId == 0)
	{
		return false;
	}
	this->firstUpload = true;
	return true;
}

bool CustomTexture::_deviceDestroyTexture()
{
	if (this->textureId != 0)
	{
		glDeleteTextures(1, &this->textureId);
		this->textureId = 0;
		return true;
	}
	return false;
}

void CustomTexture::_assignFormat()
{
	if (this->format == april::Image::Format::ARGB || this->format == april::Image::Format::XRGB || this->format == april::Image::Format::RGBA ||
		this->format == april::Image::Format::RGBX || this->format == april::Image::Format::ABGR || this->format == april::Image::Format::XBGR)
	{
		this->glFormat = this->internalFormat = GL_RGBA;
	}
	else if (this->format == april::Image::Format::BGRA || this->format == april::Image::Format::BGRX)
	{
		this->glFormat = GL_RGBA;
		this->internalFormat = GL_RGBA;
	}
	else if (this->format == april::Image::Format::RGB)
	{
		this->glFormat = this->internalFormat = GL_RGB;
	}
	else if (this->format == april::Image::Format::BGR)
	{
		this->glFormat = GL_RGB;
		this->internalFormat = GL_RGB;
	}
	else if (this->format == april::Image::Format::Alpha)
	{
		this->glFormat = this->internalFormat = GL_ALPHA;
	}
	else if (this->format == april::Image::Format::Greyscale)
	{
		this->glFormat = this->internalFormat = GL_LUMINANCE;
	}
	else if (this->format == april::Image::Format::Compressed)
	{
		this->glFormat = this->internalFormat = 0; // compressed image formats will set these values as they need to
	}
	else if (this->format == april::Image::Format::Palette)
	{
		this->glFormat = this->internalFormat = 0; // paletted image formats will set these values as they need to
	}
	else
	{
		this->glFormat = this->internalFormat = GL_RGBA;
	}
}

void CustomTexture::_setCurrentTexture()
{
	CUSTOM_RENDERSYS->deviceState->texture = this;
	CUSTOM_RENDERSYS->_setDeviceTexture(this);
	CUSTOM_RENDERSYS->_setDeviceTextureFilter(this->filter);
	CUSTOM_RENDERSYS->_setDeviceTextureAddressMode(this->addressMode);
}

april::Texture::Lock CustomTexture::_tryLockSystem(int x, int y, int w, int h)
{
	april::Texture::Lock lock;
	april::Image::Format nativeFormat = april::rendersys->getNativeTextureFormat(this->format);
	lock.activateLock(0, 0, w, h, x, y, new unsigned char[w * h * nativeFormat.getBpp()], w, h, nativeFormat);
	lock.systemBuffer = lock.data;
	return lock;
}

bool CustomTexture::_unlockSystem(Lock& lock, bool update)
{
	if (lock.systemBuffer == NULL)
	{
		return false;
	}
	if (update)
	{
		if (this->format != april::Image::Format::Compressed && this->format != april::Image::Format::Palette)
		{
			this->_setCurrentTexture();
			if (this->width == lock.w && this->height == lock.h)
			{
				glTexImage2D(GL_TEXTURE_2D, 0, this->internalFormat, this->width, this->height, 0, this->glFormat, GL_UNSIGNED_BYTE, lock.data);
			}
			else
			{
				if (this->firstUpload)
				{
					this->_uploadClearData();
				}
				glTexSubImage2D(GL_TEXTURE_2D, 0, lock.dx, lock.dy, lock.w, lock.h, this->glFormat, GL_UNSIGNED_BYTE, lock.data);
			}
		}
		delete[] lock.data;
		this->firstUpload = false;
	}
	return update;
}

bool CustomTexture::_uploadToGpu(int sx, int sy, int sw, int sh, int dx, int dy, unsigned char* srcData, int srcWidth, int srcHeight, april::Image::Format srcFormat)
{
	if (this->format == april::Image::Format::Compressed || this->format == april::Image::Format::Palette)
	{
		return false;
	}
	this->_setCurrentTexture();
	if (sx == 0 && dx == 0 && sy == 0 && dy == 0 && sw == this->width && srcWidth == this->width && sh == this->height && srcHeight == this->height)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, this->internalFormat, this->width, this->height, 0, this->glFormat, GL_UNSIGNED_BYTE, srcData);
	}
	else
	{
		if (this->firstUpload)
		{
			this->_uploadClearData();
		}
		int srcBpp = srcFormat.getBpp();
		if (sx == 0 && dx == 0 && srcWidth == this->width && sw == this->width)
		{
			glTexSubImage2D(GL_TEXTURE_2D, 0, dx, dy, sw, sh, this->glFormat, GL_UNSIGNED_BYTE, &srcData[(sx + sy * srcWidth) * srcBpp]);
		}
		else
		{
			for_iter(j, 0, sh)
			{
				glTexSubImage2D(GL_TEXTURE_2D, 0, dx, (dy + j), sw, 1, this->glFormat, GL_UNSIGNED_BYTE, &srcData[(sx + (sy + j) * srcWidth) * srcBpp]);
			}
		}
	}
	this->firstUpload = false;
	return true;
}

void CustomTexture::_uploadClearData()
{
	int size = this->getByteSize();
	unsigned char* clearColor = new unsigned char[size];
	memset(clearColor, 0, size);
	glTexImage2D(GL_TEXTURE_2D, 0, this->internalFormat, this->width, this->height, 0, this->glFormat, GL_UNSIGNED_BYTE, clearColor);
	delete[] clearColor;
}
