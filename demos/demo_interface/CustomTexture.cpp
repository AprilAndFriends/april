/// @file
/// @version 3.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <hltypes/hplatform.h>

#include <gl/GL.h>

#include <hltypes/hlog.h>
#include <hltypes/hstring.h>

#include "CustomRenderSystem.h"
#include "CustomTexture.h"

CustomTexture::CustomTexture(bool fromResource) : Texture(fromResource), textureId(0), glFormat(0)
{
}

CustomTexture::~CustomTexture()
{
	// this needs to be called here to clean up the texture
	this->unload();
}

// sometimes it's necessary to upload data right away (e.g. when using compressed textures, "data" would have to be uploaded with glCompressedTexImage2D() here)
bool CustomTexture::_createInternalTexture(unsigned char* data, int size, Type type)
{
	glGenTextures(1, &this->textureId);
	if (this->textureId == 0)
	{
		return false;
	}
	this->firstUpload = true;
	return true;
}

void CustomTexture::_destroyInternalTexture()
{
	if (this->textureId != 0)
	{
		glDeleteTextures(1, &this->textureId);
		this->textureId = 0;
	}
}

void CustomTexture::_assignFormat()
{
	switch (this->format)
	{
	case april::Image::FORMAT_ARGB:
	case april::Image::FORMAT_XRGB:
	case april::Image::FORMAT_RGBA:
	case april::Image::FORMAT_RGBX:
	case april::Image::FORMAT_ABGR:
	case april::Image::FORMAT_XBGR:
	case april::Image::FORMAT_BGRA:
	case april::Image::FORMAT_BGRX:
		this->glFormat = GL_RGBA;
		break;
	case april::Image::FORMAT_RGB:
	case april::Image::FORMAT_BGR:
		this->glFormat = GL_RGB;
		break;
	case april::Image::FORMAT_ALPHA:
		this->glFormat = GL_ALPHA;
		break;
	case april::Image::FORMAT_GRAYSCALE:
		this->glFormat = GL_LUMINANCE;
		break;
	case april::Image::FORMAT_PALETTE: // let's just say palette uses RGBA
		this->glFormat = GL_RGBA;
		break;
	default:
		this->glFormat = GL_RGBA;
		break;
	}
}

april::Texture::Lock CustomTexture::_tryLockSystem(int x, int y, int w, int h)
{
	april::Texture::Lock lock;
	april::Image::Format nativeFormat = april::rendersys->getNativeTextureFormat(this->format);
	int gpuBpp = april::Image::getFormatBpp(nativeFormat);
	lock.activateLock(0, 0, w, h, x, y, new unsigned char[w * h * gpuBpp], w, h, nativeFormat);
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
		if (this->format != april::Image::FORMAT_PALETTE)
		{
			glBindTexture(GL_TEXTURE_2D, this->textureId);
			// a few optimizations to avoid uploading all data or avoid uploading data multiple times
			if (this->width == lock.w && this->height == lock.h)
			{
				glTexImage2D(GL_TEXTURE_2D, 0, this->glFormat, this->width, this->height, 0, this->glFormat, GL_UNSIGNED_BYTE, data);
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
		delete [] lock.data;
		this->firstUpload = false;
	}
	return update;
}

bool CustomTexture::_uploadToGpu(int sx, int sy, int sw, int sh, int dx, int dy, unsigned char* srcData, int srcWidth, int srcHeight, april::Image::Format srcFormat)
{
	if (this->format == april::Image::FORMAT_PALETTE)
	{
		return false;
	}
	this->load(); // making sure the texture is loaded properly
	glBindTexture(GL_TEXTURE_2D, this->textureId);
	// a few optimizations to avoid uploading all data or avoid uploading data multiple times
	if (sx == 0 && dx == 0 && sy == 0 && dy == 0 && sw == this->width && srcWidth == this->width && sh == this->height && srcHeight == this->height)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, this->glFormat, this->width, this->height, 0, this->glFormat, GL_UNSIGNED_BYTE, srcData);
	}
	else
	{
		if (this->firstUpload)
		{
			this->_uploadClearData();
		}
		int srcBpp = april::Image::getFormatBpp(srcFormat);
		// if data is sequential (uses full width of the texture)
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
	this->firstUpload = false;
	return true;
}

void CustomTexture::_uploadClearData()
{
	int size = this->getByteSize();
	unsigned char* clearColor = new unsigned char[size];
	memset(clearColor, 0, size);
	glBindTexture(GL_TEXTURE_2D, this->textureId);
	glTexImage2D(GL_TEXTURE_2D, 0, this->glFormat, this->width, this->height, 0, this->glFormat, GL_UNSIGNED_BYTE, clearColor);
}
