/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @author  Ivan Vucica
/// @version 3.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <hltypes/harray.h>
#include <hltypes/hlog.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hresource.h>
#include <hltypes/hstring.h>

#include "april.h"
#include "Color.h"
#include "Image.h"
#include "Texture.h"
#include "RenderSystem.h"

namespace april
{
	Image::Format Texture::FORMAT_ALPHA = Image::FORMAT_ALPHA; // DEPRECATED
	Image::Format Texture::FORMAT_ARGB = Image::FORMAT_RGBA; // DEPRECATED

	Texture::Texture()
	{
		this->filename = "";
		this->type = TYPE_IMMUTABLE;
		this->format = Image::FORMAT_INVALID;
		this->dataFormat = 0;
		this->width = 0;
		this->height = 0;
		this->filter = FILTER_LINEAR;
		this->addressMode = ADDRESS_WRAP;
		this->data = NULL;
		april::rendersys->_registerTexture(this);
	}

	bool Texture::_create(chstr filename, Texture::Type type)
	{
		this->filename = filename;
		this->type = type;
		this->width = 0;
		this->height = 0;
		this->type = type;
		this->format = Image::FORMAT_INVALID;
		this->dataFormat = 0;
		this->data = NULL;
		hlog::write(april::logTag, "Creating texture: " + this->_getInternalName());
		return true;
	}

	bool Texture::_create(int w, int h, unsigned char* data, Image::Format format, Texture::Type type)
	{
		if (w == 0 || h == 0)
		{
			hlog::errorf(april::logTag, "Cannot create texture with dimentions %d,%d!", w, h);
			return false;
		}
		this->filename = "";
		this->width = w;
		this->height = h;
		this->type = type;
		int size = 0;
		if (type != TYPE_VOLATILE)
		{
			this->format = format;
			size = this->getByteSize();
			this->data = new unsigned char[size];
		}
		else
		{
			this->format = april::rendersys->getNativeTextureFormat(format);
			size = this->getByteSize();
		}
		hlog::write(april::logTag, "Creating texture: " + this->_getInternalName());
		this->dataFormat = 0;
		this->_assignFormat();
		if (!this->_createInternalTexture(data, size))
		{
			return false;
		}
		this->write(0, 0, this->width, this->height, 0, 0, data, this->width, this->height, format);
		return true;
	}

	bool Texture::_create(int w, int h, Color color, Image::Format format, Texture::Type type)
	{
		if (w == 0 || h == 0)
		{
			hlog::errorf(april::logTag, "Cannot create texture with dimensions %d,%d!", w, h);
			return false;
		}
		this->filename = "";
		this->width = w;
		this->height = h;
		this->type = type;
		int size = 0;
		if (type != TYPE_VOLATILE)
		{
			this->format = format;
			size = this->getByteSize();
			this->data = new unsigned char[size];
		}
		else
		{
			this->format = april::rendersys->getNativeTextureFormat(format);
			size = this->getByteSize();
		}
		hlog::write(april::logTag, "Creating texture: " + this->_getInternalName());
		this->dataFormat = 0;
		this->_assignFormat();
		if (!this->_createInternalTexture(this->data, size))
		{
			return false;
		}
		if (color != april::Color::Clear)
		{
			this->fillRect(0, 0, this->width, this->height, color);
		}
		return true;
	}

	Texture::~Texture()
	{
		april::rendersys->_unregisterTexture(this);
		if (this->data != NULL)
		{
			delete this->data;
		}
	}

	int Texture::getWidth()
	{
		if (this->width == 0)
		{
			hlog::warnf(april::logTag, "Texture '%s' has width = 0 (possibly not loaded yet?)", this->filename.c_str());
		}
		return this->width;
	}

	int Texture::getHeight()
	{
		if (this->height == 0)
		{
			hlog::warnf(april::logTag, "Texture '%s' has height = 0 (possibly not loaded yet?)", this->filename.c_str());
		}
		return this->height;
	}

	int Texture::getBpp()
	{
		if (this->format == Image::FORMAT_INVALID)
		{
			hlog::warnf(april::logTag, "Texture '%s' has bpp = 0 (possibly not loaded yet?)", this->filename.c_str());
		}
		return Image::getFormatBpp(this->format);
	}

	int Texture::getByteSize()
	{
		if (this->width == 0 || this->height == 0 || this->format == Image::FORMAT_INVALID)
		{
			hlog::warnf(april::logTag, "Texture '%s' has byteSize = 0 (possibly not loaded yet?)", this->filename.c_str());
		}
		return (this->width * this->height * Image::getFormatBpp(this->format));
	}

	hstr Texture::_getInternalName()
	{
		hstr result;
		if (this->filename != "")
		{
			result += "'" + this->filename + "'";
		}
		else
		{
			result += hsprintf("<0x%p>", this);
		}
		switch (this->type)
		{
		case TYPE_IMMUTABLE:
			result += " (immutable)";
			break;
		case TYPE_MANAGED:
			result += " (managed)";
			break;
		case TYPE_VOLATILE:
			result += " (volatile)";
			break;
		}
		return result;
	}

	bool Texture::load()
	{
		if (this->isLoaded())
		{
			return true;
		}
		hlog::write(april::logTag, "Loading texture: " + this->_getInternalName());
		int size = 0;
		unsigned char* currentData = NULL;
		if (this->data != NULL) // reload from memory
		{
			currentData = this->data;
			size = this->getByteSize();
		}
		// if no cached data and not a volatile texture that was previously loaded and thus has a width and height
		if (currentData == NULL && (type != TYPE_VOLATILE || this->width != 0 && this->height != 0))
		{
			if (this->filename == "")
			{
				hlog::error(april::logTag, "No filename for texture specified!");
				return false;
			}
			Image* image = Image::create(this->filename);
			if (image == NULL)
			{
				hlog::error(april::logTag, "Failed to load texture: " + this->_getInternalName());
				return false;
			}
			this->width = image->w;
			this->height = image->h;
			this->format = image->format;
			this->dataFormat = image->internalFormat;
			if (this->dataFormat != 0)
			{
				size = image->compressedSize;
			}
			currentData = image->data;
			image->data = NULL;
			delete image;
		}
		this->_assignFormat();
		if (!this->_createInternalTexture(currentData, size))
		{
			hlog::error(april::logTag, "Failed to create DX9 texture!");
			if (currentData != NULL && this->data != currentData)
			{
				delete [] currentData;
			}
			return false;
		}
		if (currentData != NULL)
		{
			this->write(0, 0, this->width, this->height, 0, 0, currentData, this->width, this->height, format);
			if (this->type != TYPE_VOLATILE && (this->type != TYPE_IMMUTABLE || this->filename == ""))
			{
				if (this->data != currentData)
				{
					if (this->data != NULL)
					{
						delete [] this->data;
					}
					this->data = currentData;
				}
			}
			else
			{
				delete [] currentData;
				// the used format will be the native format, because there is no intermediate data
				this->format = april::rendersys->getNativeTextureFormat(this->format);
			}
		}
		return true;
	}

	bool Texture::clear()
	{
		if (this->data != NULL)
		{
			// TODOaa - check if this works for palette formatting as well
			memset(this->data, 0, this->getByteSize());
			return this->_uploadDataToGpu(0, 0, this->width, this->height);
		}
		return false;
	}

	Color Texture::getPixel(int x, int y)
	{
		Color color = Color::Clear;
		if (this->data != NULL)
		{
			color = Image::getPixel(x, y, this->data, this->width, this->height, this->format);
		}
		return color;
	}

	bool Texture::setPixel(int x, int y, Color color)
	{
		return (this->data != NULL && Image::setPixel(x, y, color, this->data, this->width, this->height, this->format) && this->_uploadDataToGpu(x, y, 1, 1));
	}

	Color Texture::getInterpolatedPixel(float x, float y)
	{
		Color color = Color::Clear;
		if (this->data != NULL)
		{
			color = Image::getInterpolatedPixel(x, y, this->data, this->width, this->height, this->format);
		}
		return color;
	}
	
	bool Texture::fillRect(int x, int y, int w, int h, Color color)
	{
		return (this->data != NULL && Image::fillRect(x, y, w, h, color, this->data, this->width, this->height, this->format) && this->_uploadDataToGpu(x, y, w, h));
	}

	bool Texture::copyPixelData(unsigned char** output, Image::Format format)
	{
		return (this->data != NULL && Image::convertToFormat(this->width, this->height, this->data, this->format, output, format, false));
	}

	bool Texture::write(int sx, int sy, int sw, int sh, int dx, int dy, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat)
	{
		return (this->data != NULL && Image::write(sx, sy, sw, sh, dx, dy, srcData, srcWidth, srcHeight, srcFormat, this->data, this->width, this->height, this->format) && this->_uploadDataToGpu(dx, dy, sw, sh));
	}

	bool Texture::writeStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat)
	{
		return (this->data != NULL && Image::writeStretch(sx, sy, sw, sh, dx, dy, dw, dh, srcData, srcWidth, srcHeight, srcFormat, this->data, this->width, this->height, this->format) && this->_uploadDataToGpu(dx, dy, dw, dh));
	}

	bool Texture::blit(int sx, int sy, int sw, int sh, int dx, int dy, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat, unsigned char alpha)
	{
		return (this->data != NULL && Image::blit(sx, sy, sw, sh, dx, dy, srcData, srcWidth, srcHeight, srcFormat, this->data, this->width, this->height, this->format, alpha) && this->_uploadDataToGpu(dx, dy, sw, sh));
	}

	bool Texture::blitStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat, unsigned char alpha)
	{
		return (this->data != NULL && Image::blitStretch(sx, sy, sw, sh, dx, dy, dw, dh, srcData, srcWidth, srcHeight, srcFormat, this->data, this->width, this->height, this->format, alpha) && this->_uploadDataToGpu(dx, dy, dw, dh));
	}

	bool Texture::rotateHue(float degrees)
	{
		// TODOaa - implement
		hlog::warnf(april::logTag, "Rendersystem '%s' does not implement rotateHue()!", april::rendersys->getName().c_str());
		return false;
	}

	bool Texture::saturate(float factor)
	{
		// TODOaa - implement
		hlog::warnf(april::logTag, "Rendersystem '%s' does not implement saturate()!", april::rendersys->getName().c_str());
		return false;
	}

	bool Texture::insertAlphaMap(unsigned char* srcData, Image::Format srcFormat)
	{
		return (this->data != NULL && Image::insertAlphaMap(this->width, this->height, srcData, srcFormat, this->data, this->format) && this->_uploadDataToGpu(0, 0, this->width, this->height));
	}

	bool Texture::insertAlphaMap(unsigned char* srcData, Image::Format srcFormat, unsigned char median, int ambiguity)
	{
		return (this->data != NULL && Image::insertAlphaMap(this->width, this->height, srcData, srcFormat, this->data, this->format, median, ambiguity) && this->_uploadDataToGpu(0, 0, this->width, this->height));
	}

	// overloads

	Color Texture::getPixel(gvec2 position)
	{
		return this->getPixel(hround(position.x), hround(position.y));
	}

	bool Texture::setPixel(gvec2 position, Color color)
	{
		return this->setPixel(hround(position.x), hround(position.y), color);
	}

	Color Texture::getInterpolatedPixel(gvec2 position)
	{
		return this->getInterpolatedPixel(position.x, position.y);
	}

	bool Texture::fillRect(grect rect, Color color)
	{
		return this->fillRect(hround(rect.x), hround(rect.y), hround(rect.w), hround(rect.h), color);
	}

	bool Texture::copyPixelData(unsigned char** output)
	{
		return this->copyPixelData(output, this->format);
	}

	bool Texture::write(grect srcRect, gvec2 destPosition, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat)
	{
		return this->write(hround(srcRect.x), hround(srcRect.y), hround(srcRect.w), hround(srcRect.h), hround(destPosition.x), hround(destPosition.y), srcData, srcWidth, srcHeight, srcFormat);
	}

	/*
	bool Texture::write(grect srcRect, gvec2 destPosition, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat)
	{
		return this->write(hround(srcRect.x), hround(srcRect.y), hround(srcRect.w), hround(srcRect.h), hround(destPosition.x), hround(destPosition.y), srcData, srcWidth, srcHeight, srcFormat);
	}

	bool Texture::write(grect srcRect, gvec2 destPosition, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat)
	{
		return this->write(hround(srcRect.x), hround(srcRect.y), hround(srcRect.w), hround(srcRect.h), hround(destPosition.x), hround(destPosition.y), srcData, srcWidth, srcHeight, srcFormat);
	}
	*/

	bool Texture::writeStretch(grect srcRect, grect destRect, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat)
	{
		return this->writeStretch(hround(srcRect.x), hround(srcRect.y), hround(srcRect.w), hround(srcRect.h), hround(destRect.x), hround(destRect.y), hround(destRect.w), hround(destRect.h), srcData, srcWidth, srcHeight, srcFormat);
	}

	// TODOaa - blit goes here
	// TODOaa - blitStretch goes here

}
