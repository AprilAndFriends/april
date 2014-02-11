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
			Image* image = Image::load(this->filename);
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

	void Texture::clear()
	{
		if (this->data != NULL)
		{
			// TODOaa - check if this works for palette formatting as well
			memset(this->data, 0, this->getByteSize());
			this->_uploadDataToGpu(0, 0, this->width, this->height);
		}
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

	void Texture::setPixel(int x, int y, Color color)
	{
		if (this->data != NULL && Image::setPixel(x, y, color, this->data, this->width, this->height, this->format))
		{
			this->_uploadDataToGpu(x, y, 1, 1);
		}
	}

	void Texture::fillRect(int x, int y, int w, int h, Color color)
	{
		if (this->data != NULL && Image::fillRect(x, y, w, h, color, this->data, this->width, this->height, this->format))
		{
			this->_uploadDataToGpu(x, y, w, h);
		}
	}

	void Texture::write(int sx, int sy, int sw, int sh, int dx, int dy, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat)
	{
		if (this->data != NULL && Image::write(sx, sy, sw, sh, dx, dy, srcData, srcWidth, srcHeight, srcFormat, this->data, this->width, this->height, this->format))
		{
			this->_uploadDataToGpu(dx, dy, sw, sh);
		}
	}

	bool Texture::copyPixelData(unsigned char** output, Image::Format format)
	{
		return (this->data != NULL && Image::convertToFormat(this->width, this->height, this->data, this->format, output, format, false));
	}

	void Texture::blit(int x, int y, Texture* texture, int sx, int sy, int sw, int sh, unsigned char alpha)
	{
		hlog::warnf(april::logTag, "Rendersystem '%s' does not implement blit()!", april::rendersys->getName().c_str());
	}

	void Texture::blit(int x, int y, unsigned char* data, int dataWidth, int dataHeight, int dataBpp, int sx, int sy, int sw, int sh, unsigned char alpha)
	{
		hlog::warnf(april::logTag, "Rendersystem '%s' does not implement blit()!", april::rendersys->getName().c_str());
	}

	void Texture::stretchBlit(int x, int y, int w, int h, Texture* texture, int sx, int sy, int sw, int sh, unsigned char alpha)
	{
		hlog::warnf(april::logTag, "Rendersystem '%s' does not implement stretchBlit()!", april::rendersys->getName().c_str());
	}

	void Texture::stretchBlit(int x, int y, int w, int h, unsigned char* data,int dataWidth, int dataHeight, int dataBpp, int sx, int sy, int sw, int sh, unsigned char alpha)
	{
		hlog::warnf(april::logTag, "Rendersystem '%s' does not implement stretchBlit()!", april::rendersys->getName().c_str());
	}

	void Texture::rotateHue(float degrees)
	{
		hlog::warnf(april::logTag, "Rendersystem '%s' does not implement rotateHue()!", april::rendersys->getName().c_str());
	}

	void Texture::saturate(float factor)
	{
		hlog::warnf(april::logTag, "Rendersystem '%s' does not implement saturate()!", april::rendersys->getName().c_str());
	}

	void Texture::insertAsAlphaMap(Texture* texture, unsigned char median, int ambiguity)
	{
		hlog::warnf(april::logTag, "Rendersystem '%s' does not implement insertAsAlphaMap()!", april::rendersys->getName().c_str());
	}

	Color Texture::getPixel(gvec2 position)
	{
		return this->getPixel(hround(position.x), hround(position.y));
	}

	void Texture::setPixel(gvec2 position, Color color)
	{
		this->setPixel(hround(position.x), hround(position.y), color);
	}

	Color Texture::getInterpolatedPixel(float x, float y)
	{
		// TODOaa - refactor and make faster?
		Color result;
		int x0 = (int) x;
		int y0 = (int) y;
		int x1 = x0 + 1;
		int y1 = y0 + 1;
		float rx0 = x - x0;
		float ry0 = y - y0;
		float rx1 = 1.0f - rx0;
		float ry1 = 1.0f - ry0;
		if (rx0 != 0.0f && ry0 != 0.0f)
		{
			Color tl = this->getPixel(x0, y0);
			Color tr = this->getPixel(x1, y0);
			Color bl = this->getPixel(x0, y1);
			Color br = this->getPixel(x1, y1);
			result = (tl * ry1 + bl * ry0) * rx1 + (tr * ry1 + br * ry0) * rx0;
		}
		else if (rx0 != 0.0f)
		{
			Color tl = this->getPixel(x0, y0);
			Color tr = this->getPixel(x1, y0);
			result = tl * rx1 + tr * rx0;
		}
		else if (ry0 != 0.0f)
		{
			Color tl = this->getPixel(x0, y0);
			Color bl = this->getPixel(x0, y1);
			result = tl * ry1 + bl * ry0;
		}
		else
		{
			result = this->getPixel(x0, y0);
		}
		return result;
	}
	
	Color Texture::getInterpolatedPixel(gvec2 position)
	{
		return this->getInterpolatedPixel(position.x, position.y);
	}

	void Texture::fillRect(grect rect, Color color)
	{
		this->fillRect(hround(rect.x), hround(rect.y), hround(rect.w), hround(rect.h), color);
	}

	void Texture::write(grect srcRect, gvec2 destPosition, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat)
	{
		this->write(hround(srcRect.x), hround(srcRect.y), hround(srcRect.w), hround(srcRect.h), hround(destPosition.x), hround(destPosition.y), srcData, srcWidth, srcHeight, srcFormat);
	}

	bool Texture::copyPixelData(unsigned char** output)
	{
		return this->copyPixelData(output, this->format);
	}

	void Texture::blit(int x, int y, Image* image, int sx, int sy, int sw, int sh, unsigned char alpha)
	{
		this->blit(x, y, image->data, image->w, image->h, image->getBpp(), sx, sy, sw, sh, alpha);
	}

	void Texture::blit(gvec2 position, Texture* texture, grect source, unsigned char alpha)
	{
		this->blit(hround(position.x), hround(position.y), texture, hround(source.x), hround(source.y), hround(source.w), hround(source.h), alpha);
	}

	void Texture::blit(gvec2 position, Image* image, grect source, unsigned char alpha)
	{
		this->blit(hround(position.x), hround(position.y), image->data, image->w, image->h, image->getBpp(), hround(source.x), hround(source.y), hround(source.w), hround(source.h), alpha);
	}

	void Texture::blit(gvec2 position, unsigned char* data, int dataWidth, int dataHeight, int dataBpp, grect source, unsigned char alpha)
	{
		this->blit(hround(position.x), hround(position.y), data, dataWidth, dataHeight, dataBpp, hround(source.x), hround(source.y), hround(source.w), hround(source.h), alpha);
	}

	void Texture::stretchBlit(int x, int y, int w, int h, Image* image, int sx, int sy, int sw, int sh, unsigned char alpha)
	{
		this->stretchBlit(x, y, w, h, image->data, image->w, image->h, image->getBpp(), sx, sy, sw, sh, alpha);
	}

	void Texture::stretchBlit(grect destination, Texture* texture, grect source, unsigned char alpha)
	{
		this->stretchBlit(hround(destination.x), hround(destination.y), hround(destination.w), hround(destination.h), texture,
			hround(source.x), hround(source.y), hround(source.w), hround(source.h), alpha);
	}

	void Texture::stretchBlit(grect destination, Image* image, grect source, unsigned char alpha)
	{
		this->stretchBlit(hround(destination.x), hround(destination.y), hround(destination.w), hround(destination.h), image->data, image->w, image->h, image->getBpp(),
			hround(source.x), hround(source.y), hround(source.w), hround(source.h), alpha);
	}

	void Texture::stretchBlit(grect destination, unsigned char* data, int dataWidth, int dataHeight, int dataBpp, grect source, unsigned char alpha)
	{
		this->stretchBlit(hround(destination.x), hround(destination.y), hround(destination.w), hround(destination.h), data, dataWidth, dataHeight, dataBpp,
			hround(source.x), hround(source.y), hround(source.w), hround(source.h), alpha);
	}

	void Texture::_blit(unsigned char* thisData, int x, int y, unsigned char* srcData, int dataWidth, int dataHeight, int dataBpp, int sx, int sy, int sw, int sh, unsigned char alpha)
	{
		x = hclamp(x, 0, this->width - 1);
		y = hclamp(y, 0, this->height - 1);
		sx = hclamp(sx, 0, dataWidth - 1);
		sy = hclamp(sy, 0, dataHeight - 1);
		sw = hmin(sw, hmin(this->width - x, dataWidth - sx));
		sh = hmin(sh, hmin(this->height - y, dataHeight - sy));
		if (sw < 1 || sh < 1)
		{
			return;
		}
		unsigned char* c;
		unsigned char* sc;
		unsigned char a0;
		unsigned char a1;
		int i;
		int j;
		// TODOaa - should be checked on different BPPs and data access better
		// the following iteration blocks are very similar, but for performance reasons they
		// have been duplicated instead of putting everything into one block with if branches
		int thisBpp = this->getBpp();
		if (thisBpp == 4 && dataBpp == 4)
		{
			for_iterx (j, 0, sh)
			{
				for_iterx (i, 0, sw)
				{
					c = &thisData[(i + j * this->width) * 4];
					sc = &srcData[(i + j * dataWidth) * 4];
					if (c[3] > 0)
					{
						a0 = sc[3] * alpha / 255;
						if (a0 > 0)
						{
							a1 = 255 - a0;
							c[0] = (sc[0] * a0 + c[0] * a1) / 255;
							c[1] = (sc[1] * a0 + c[1] * a1) / 255;
							c[2] = (sc[2] * a0 + c[2] * a1) / 255;
							c[3] = a0 + c[3] * a1 / 255;
						}
					}
					else
					{
						c[0] = sc[0];
						c[1] = sc[1];
						c[2] = sc[2];
						c[3] = sc[3] * alpha / 255;
					}
				}
			}
		}
		else if (thisBpp == 3 && dataBpp == 4)
		{
			for_iterx (j, 0, sh)
			{
				for_iterx (i, 0, sw)
				{
					c = &thisData[(i + j * this->width) * 4];
					sc = &srcData[(i + j * dataWidth) * 4];
					a0 = sc[3] * alpha / 255;
					if (a0 > 0)
					{
						a1 = 255 - a0;
						c[0] = (sc[0] * a0 + c[0] * a1) / 255;
						c[1] = (sc[1] * a0 + c[1] * a1) / 255;
						c[2] = (sc[2] * a0 + c[2] * a1) / 255;
					}
				}
			}
		}
		else if (thisBpp == 4 && dataBpp == 3)
		{
			if (alpha > 0)
			{
				a0 = alpha;
				a1 = 255 - a0;
				for_iterx (j, 0, sh)
				{
					for_iterx (i, 0, sw)
					{
						c = &thisData[(i + j * this->width) * 4];
						sc = &srcData[(i + j * dataWidth) * 4];
						c[0] = (sc[0] * a0 + c[0] * a1) / 255;
						c[1] = (sc[1] * a0 + c[1] * a1) / 255;
						c[2] = (sc[2] * a0 + c[2] * a1) / 255;
						c[3] = a0 + c[3] * a1 / 255;
					}
				}
			}
		}
		else if (thisBpp == 1 && dataBpp == 1)
		{
			if (alpha > 0)
			{
				a0 = alpha;
				a1 = 255 - a0;
				for_iterx (j, 0, sh)
				{
					for_iterx (i, 0, sw)
					{
						c = &thisData[i + j * this->width];
						sc = &srcData[i + j * dataWidth];
						c[0] = (sc[0] * a0 + c[0] * a1) / 255;
					}
				}
			}
		}
		else
		{
			hlog::error(april::logTag, "Unsupported format for blit()!");
		}
	}

	void Texture::_stretchBlit(unsigned char* thisData, int x, int y, int w, int h, unsigned char* srcData, int dataWidth, int dataHeight, int dataBpp, int sx, int sy, int sw, int sh, unsigned char alpha)
	{
		x = hclamp(x, 0, this->width - 1);
		y = hclamp(y, 0, this->height - 1);
		w = hmin(w, this->width - x);
		h = hmin(h, this->height - y);
		sx = hclamp(sx, 0, dataWidth - 1);
		sy = hclamp(sy, 0, dataHeight - 1);
		sw = hmin(sw, dataWidth - sx);
		sh = hmin(sh, dataHeight - sy);
		if (w < 1 || h < 1 || sw < 1 || sh < 1)
		{
			return;
		}
		float fw = (float)sw / w;
		float fh = (float)sh / h;
		unsigned char* c;
		unsigned char* sc;
		int a0;
		int a1;
		unsigned char color[4] = {0};
		unsigned char* ctl;
		unsigned char* ctr;
		unsigned char* cbl;
		unsigned char* cbr;
		float cx;
		float cy;
		float rx0;
		float ry0;
		float rx1;
		float ry1;
		int x0;
		int y0;
		int x1;
		int y1;
		int i;
		int j;
		// TODOaa - should be checked on different BPPs and data access better
		// the following iteration blocks are very similar, but for performance reasons they
		// have been duplicated instead of putting everything into one block with if branches
		int thisBpp = this->getBpp();
		if (thisBpp == 4 && dataBpp == 4)
		{
			for_iterx (j, 0, h)
			{
				for_iterx (i, 0, w)
				{
					c = &thisData[(i + j * this->width) * 4];
					cx = sx + i * fw;
					cy = sy + j * fh;
					x0 = (int)cx;
					y0 = (int)cy;
					x1 = hmin((int)cx + 1, dataWidth - 1);
					y1 = hmin((int)cy + 1, dataHeight - 1);
					rx0 = cx - x0;
					ry0 = cy - y0;
					rx1 = 1.0f - rx0;
					ry1 = 1.0f - ry0;
					if (rx0 != 0.0f || ry0 != 0.0f)
					{
						ctl = &srcData[(x0 + y0 * dataWidth) * 4];
						ctr = &srcData[(x1 + y0 * dataWidth) * 4];
						cbl = &srcData[(x0 + y1 * dataWidth) * 4];
						cbr = &srcData[(x1 + y1 * dataWidth) * 4];
						color[0] = (unsigned char)(((ctl[0] * ry1 + cbl[0] * ry0) * rx1 + (ctr[0] * ry1 + cbr[0] * ry0) * rx0));
						color[1] = (unsigned char)(((ctl[1] * ry1 + cbl[1] * ry0) * rx1 + (ctr[1] * ry1 + cbr[1] * ry0) * rx0));
						color[2] = (unsigned char)(((ctl[2] * ry1 + cbl[2] * ry0) * rx1 + (ctr[2] * ry1 + cbr[2] * ry0) * rx0));
						color[3] = (unsigned char)(((ctl[3] * ry1 + cbl[3] * ry0) * rx1 + (ctr[3] * ry1 + cbr[3] * ry0) * rx0));
						sc = color;
					}
					else if (rx0 != 0.0f)
					{
						ctl = &srcData[(x0 + y0 * dataWidth) * 4];
						ctr = &srcData[(x1 + y0 * dataWidth) * 4];
						color[0] = (unsigned char)((ctl[0] * rx1 + ctr[0] * rx0));
						color[1] = (unsigned char)((ctl[1] * rx1 + ctr[1] * rx0));
						color[2] = (unsigned char)((ctl[2] * rx1 + ctr[2] * rx0));
						color[3] = (unsigned char)((ctl[3] * rx1 + ctr[3] * rx0));
						sc = color;
					}
					else if (ry0 != 0.0f)
					{
						ctl = &srcData[(x0 + y0 * dataWidth) * 4];
						cbl = &srcData[(x0 + y1 * dataWidth) * 4];
						color[0] = (unsigned char)((ctl[0] * ry1 + cbl[0] * ry0));
						color[1] = (unsigned char)((ctl[1] * ry1 + cbl[1] * ry0));
						color[2] = (unsigned char)((ctl[2] * ry1 + cbl[2] * ry0));
						color[3] = (unsigned char)((ctl[3] * ry1 + cbl[3] * ry0));
						sc = color;
					}
					else
					{
						sc = &srcData[(x0 + y0 * dataWidth) * 4];
					}
					a0 = sc[3] * (int)alpha / 255;
					if (a0 > 0)
					{
						if (c[3] > 0)
						{
							a1 = 255 - a0;
							c[0] = (unsigned char)((sc[0] * a0 + c[0] * a1) / 255);
							c[1] = (unsigned char)((sc[1] * a0 + c[1] * a1) / 255);
							c[2] = (unsigned char)((sc[2] * a0 + c[2] * a1) / 255);
							c[3] = (unsigned char)(a0 + c[3] * a1 / 255);
						}
						else
						{
							c[0] = sc[0];
							c[1] = sc[1];
							c[2] = sc[2];
							c[3] = a0;
						}
					}
				}
			}
		}
		else if (thisBpp == 3 && dataBpp == 4)
		{
			for_iterx (j, 0, h)
			{
				for_iterx (i, 0, w)
				{
					c = &thisData[(i + j * this->width) * 4];
					cx = sx + i * fw;
					cy = sy + j * fh;
					x0 = (int)cx;
					y0 = (int)cy;
					x1 = hmin((int)cx + 1, dataWidth - 1);
					y1 = hmin((int)cy + 1, dataHeight - 1);
					rx0 = cx - x0;
					ry0 = cy - y0;
					rx1 = 1.0f - rx0;
					ry1 = 1.0f - ry0;
					if (rx0 != 0.0f || ry0 != 0.0f)
					{
						ctl = &srcData[(x0 + y0 * dataWidth) * 4];
						ctr = &srcData[(x1 + y0 * dataWidth) * 4];
						cbl = &srcData[(x0 + y1 * dataWidth) * 4];
						cbr = &srcData[(x1 + y1 * dataWidth) * 4];
						color[0] = (unsigned char)(((ctl[0] * ry1 + cbl[0] * ry0) * rx1 + (ctr[0] * ry1 + cbr[0] * ry0) * rx0));
						color[1] = (unsigned char)(((ctl[1] * ry1 + cbl[1] * ry0) * rx1 + (ctr[1] * ry1 + cbr[1] * ry0) * rx0));
						color[2] = (unsigned char)(((ctl[2] * ry1 + cbl[2] * ry0) * rx1 + (ctr[2] * ry1 + cbr[2] * ry0) * rx0));
						color[3] = (unsigned char)(((ctl[3] * ry1 + cbl[3] * ry0) * rx1 + (ctr[3] * ry1 + cbr[3] * ry0) * rx0));
						sc = color;
					}
					else if (rx0 != 0.0f)
					{
						ctl = &srcData[(x0 + y0 * dataWidth) * 4];
						ctr = &srcData[(x1 + y0 * dataWidth) * 4];
						color[0] = (unsigned char)((ctl[0] * rx1 + ctr[0] * rx0));
						color[1] = (unsigned char)((ctl[1] * rx1 + ctr[1] * rx0));
						color[2] = (unsigned char)((ctl[2] * rx1 + ctr[2] * rx0));
						color[3] = (unsigned char)((ctl[3] * rx1 + ctr[3] * rx0));
						sc = color;
					}
					else if (ry0 != 0.0f)
					{
						ctl = &srcData[(x0 + y0 * dataWidth) * 4];
						cbl = &srcData[(x0 + y1 * dataWidth) * 4];
						color[0] = (unsigned char)((ctl[0] * ry1 + cbl[0] * ry0));
						color[1] = (unsigned char)((ctl[1] * ry1 + cbl[1] * ry0));
						color[2] = (unsigned char)((ctl[2] * ry1 + cbl[2] * ry0));
						color[3] = (unsigned char)((ctl[3] * ry1 + cbl[3] * ry0));
						sc = color;
					}
					else
					{
						sc = &srcData[(x0 + y0 * dataWidth) * 4];
					}
					a0 = sc[3] * (int)alpha / 255;
					if (a0 > 0)
					{
						a1 = 255 - a0;
						c[0] = (unsigned char)((sc[0] * a0 + c[0] * a1) / 255);
						c[1] = (unsigned char)((sc[1] * a0 + c[1] * a1) / 255);
						c[2] = (unsigned char)((sc[2] * a0 + c[2] * a1) / 255);
					}
				}
			}
		}
		else if (thisBpp == 4 && dataBpp == 3)
		{
			if (alpha > 0)
			{
				for_iterx (j, 0, h)
				{
					for_iterx (i, 0, w)
					{
						c = &thisData[(i + j * this->width) * 4];
						cx = sx + i * fw;
						cy = sy + j * fh;
						x0 = (int)cx;
						y0 = (int)cy;
						x1 = hmin((int)cx + 1, dataWidth - 1);
						y1 = hmin((int)cy + 1, dataHeight - 1);
						rx0 = cx - x0;
						ry0 = cy - y0;
						rx1 = 1.0f - rx0;
						ry1 = 1.0f - ry0;
						if (rx0 != 0.0f || ry0 != 0.0f)
						{
							ctl = &srcData[(x0 + y0 * dataWidth) * 4];
							ctr = &srcData[(x1 + y0 * dataWidth) * 4];
							cbl = &srcData[(x0 + y1 * dataWidth) * 4];
							cbr = &srcData[(x1 + y1 * dataWidth) * 4];
							color[0] = (unsigned char)(((ctl[0] * ry1 + cbl[0] * ry0) * rx1 + (ctr[0] * ry1 + cbr[0] * ry0) * rx0));
							color[1] = (unsigned char)(((ctl[1] * ry1 + cbl[1] * ry0) * rx1 + (ctr[1] * ry1 + cbr[1] * ry0) * rx0));
							color[2] = (unsigned char)(((ctl[2] * ry1 + cbl[2] * ry0) * rx1 + (ctr[2] * ry1 + cbr[2] * ry0) * rx0));
							sc = color;
						}
						else if (rx0 != 0.0f)
						{
							ctl = &srcData[(x0 + y0 * dataWidth) * 4];
							ctr = &srcData[(x1 + y0 * dataWidth) * 4];
							color[0] = (unsigned char)((ctl[0] * rx1 + ctr[0] * rx0));
							color[1] = (unsigned char)((ctl[1] * rx1 + ctr[1] * rx0));
							color[2] = (unsigned char)((ctl[2] * rx1 + ctr[2] * rx0));
							sc = color;
						}
						else if (ry0 != 0.0f)
						{
							ctl = &srcData[(x0 + y0 * dataWidth) * 4];
							cbl = &srcData[(x0 + y1 * dataWidth) * 4];
							color[0] = (unsigned char)((ctl[0] * ry1 + cbl[0] * ry0));
							color[1] = (unsigned char)((ctl[1] * ry1 + cbl[1] * ry0));
							color[2] = (unsigned char)((ctl[2] * ry1 + cbl[2] * ry0));
							sc = color;
						}
						else
						{
							sc = &srcData[(x0 + y0 * dataWidth) * 4];
						}
						c[0] = sc[0];
						c[1] = sc[1];
						c[2] = sc[2];
						c[3] = 255;
					}
				}
			}
		}
		else if (thisBpp == 1 && dataBpp == 1)
		{
			if (alpha > 0)
			{
				for_iterx (j, 0, h)
				{
					for_iterx (i, 0, w)
					{
						c = &thisData[i + j * this->width];
						cx = sx + i * fw;
						cy = sy + j * fh;
						x0 = (int)cx;
						y0 = (int)cy;
						x1 = hmin((int)cx + 1, dataWidth - 1);
						y1 = hmin((int)cy + 1, dataHeight - 1);
						rx0 = cx - x0;
						ry0 = cy - y0;
						rx1 = 1.0f - rx0;
						ry1 = 1.0f - ry0;
						if (rx0 != 0.0f || ry0 != 0.0f)
						{
							ctl = &srcData[x0 + y0 * dataWidth];
							ctr = &srcData[x1 + y0 * dataWidth];
							cbl = &srcData[x0 + y1 * dataWidth];
							cbr = &srcData[x1 + y1 * dataWidth];
							color[0] = (unsigned char)(((ctl[0] * ry1 + cbl[0] * ry0) * rx1 + (ctr[0] * ry1 + cbr[0] * ry0) * rx0));
							sc = color;
						}
						else if (rx0 != 0.0f)
						{
							ctl = &srcData[x0 + y0 * dataWidth];
							ctr = &srcData[x1 + y0 * dataWidth];
							color[0] = (unsigned char)((ctl[0] * rx1 + ctr[0] * rx0));
							sc = color;
						}
						else if (ry0 != 0.0f)
						{
							ctl = &srcData[x0 + y0 * dataWidth];
							cbl = &srcData[x0 + y1 * dataWidth];
							color[0] = (unsigned char)((ctl[0] * ry1 + cbl[0] * ry0));
							sc = color;
						}
						else
						{
							sc = &srcData[x0 + y0 * dataWidth];
						}
						c[0] = sc[0];
					}
				}
			}
		}
		else
		{
			hlog::error(april::logTag, "Unsupported format for stretchBlit()!");
		}
	}

}
