/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @author  Ivan Vucica
/// @version 3.2
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
		this->format = FORMAT_INVALID;
		this->width = 0;
		this->height = 0;
		this->bpp = 4;
		this->filter = FILTER_LINEAR;
		this->addressMode = ADDRESS_WRAP;
		april::rendersys->_registerTexture(this);
	}

	Texture::~Texture()
	{
		april::rendersys->_unregisterTexture(this);
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
		if (this->bpp == 0)
		{
			hlog::warnf(april::logTag, "Texture '%s' has bpp = 0 (possibly not loaded yet?)", this->filename.c_str());
		}
		return this->bpp;
	}

	int Texture::getByteSize()
	{
		if (this->width == 0 || this->height == 0 || this->bpp == 0)
		{
			hlog::warnf(april::logTag, "Texture '%s' has byteSize = 0 (possibly not loaded yet?)", this->filename.c_str());
		}
		return (this->width * this->height * this->bpp);
	}

	hstr Texture::_getInternalName()
	{
		return (this->filename != "" ? this->filename : "UserTexture");
	}

	void Texture::clear()
	{
		hlog::warnf(april::logTag, "Rendersystem '%s' does not implement clear()!", april::rendersys->getName().c_str());
	}

	Color Texture::getPixel(int x, int y)
	{
		hlog::warnf(april::logTag, "Rendersystem '%s' does not implement getPixel()!", april::rendersys->getName().c_str());
		return Color::Clear;
	}

	void Texture::setPixel(int x, int y, Color color)
	{
		hlog::warnf(april::logTag, "Rendersystem '%s' does not implement setPixel()!", april::rendersys->getName().c_str());
	}

	void Texture::fillRect(int x, int y, int w, int h, Color color)
	{
		hlog::warnf(april::logTag, "Rendersystem '%s' does not implement fillRect()!", april::rendersys->getName().c_str());
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

	void Texture::blit(int x, int y, Image* image, int sx, int sy, int sw, int sh, unsigned char alpha)
	{
		this->blit(x, y, image->data, image->w, image->h, image->bpp, sx, sy, sw, sh, alpha);
	}

	void Texture::blit(gvec2 position, Texture* texture, grect source, unsigned char alpha)
	{
		this->blit(hround(position.x), hround(position.y), texture, hround(source.x), hround(source.y), hround(source.w), hround(source.h), alpha);
	}

	void Texture::blit(gvec2 position, Image* image, grect source, unsigned char alpha)
	{
		this->blit(hround(position.x), hround(position.y), image->data, image->w, image->h, image->bpp, hround(source.x), hround(source.y), hround(source.w), hround(source.h), alpha);
	}

	void Texture::blit(gvec2 position, unsigned char* data, int dataWidth, int dataHeight, int dataBpp, grect source, unsigned char alpha)
	{
		this->blit(hround(position.x), hround(position.y), data, dataWidth, dataHeight, dataBpp, hround(source.x), hround(source.y), hround(source.w), hround(source.h), alpha);
	}

	void Texture::stretchBlit(int x, int y, int w, int h, Image* image, int sx, int sy, int sw, int sh, unsigned char alpha)
	{
		this->stretchBlit(x, y, w, h, image->data, image->w, image->h, image->bpp, sx, sy, sw, sh, alpha);
	}

	void Texture::stretchBlit(grect destination, Texture* texture, grect source, unsigned char alpha)
	{
		this->stretchBlit(hround(destination.x), hround(destination.y), hround(destination.w), hround(destination.h), texture,
			hround(source.x), hround(source.y), hround(source.w), hround(source.h), alpha);
	}

	void Texture::stretchBlit(grect destination, Image* image, grect source, unsigned char alpha)
	{
		this->stretchBlit(hround(destination.x), hround(destination.y), hround(destination.w), hround(destination.h), image->data, image->w, image->h, image->bpp,
			hround(source.x), hround(source.y), hround(source.w), hround(source.h), alpha);
	}

	void Texture::stretchBlit(grect destination, unsigned char* data, int dataWidth, int dataHeight, int dataBpp, grect source, unsigned char alpha)
	{
		this->stretchBlit(hround(destination.x), hround(destination.y), hround(destination.w), hround(destination.h), data, dataWidth, dataHeight, dataBpp,
			hround(source.x), hround(source.y), hround(source.w), hround(source.h), alpha);
	}

	void Texture::write(int x, int y, unsigned char* data, int dataWidth, int dataHeight, int dataBpp)
	{
	
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
		// TODOa - should be checked on different BPPs and data access better
		// the following iteration blocks are very similar, but for performance reasons they
		// have been duplicated instead of putting everything into one block with if branches
		if (this->bpp == 4 && dataBpp == 4)
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
		else if (this->bpp == 3 && dataBpp == 4)
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
		else if (this->bpp == 4 && dataBpp == 3)
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
		else if (this->bpp == 1 && dataBpp == 1)
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
		// TODOa - should be checked on different BPPs and data access better
		// the following iteration blocks are very similar, but for performance reasons they
		// have been duplicated instead of putting everything into one block with if branches
		if (this->bpp == 4 && dataBpp == 4)
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
		else if (this->bpp == 3 && dataBpp == 4)
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
		else if (this->bpp == 4 && dataBpp == 3)
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
		else if (this->bpp == 1 && dataBpp == 1)
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
