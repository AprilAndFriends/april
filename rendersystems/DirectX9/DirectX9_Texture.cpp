/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @version 2.42
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _DIRECTX9
#include <d3d9.h>
#include <IL/il.h>

#include <hltypes/hlog.h>

#include "april.h"
#include "DirectX9_RenderSystem.h"
#include "DirectX9_Texture.h"
#include "ImageSource.h"

#define APRIL_D3D_DEVICE (((DirectX9_RenderSystem*)april::rendersys)->d3dDevice)
#define _CREATE_RECT(name, x, y, w, h) \
	RECT name; \
	name.left = x; \
	name.top = y; \
	name.right = x + w - 1; \
	name.bottom = y + h - 1;

namespace april
{
	// TODO - refactor
	extern harray<DirectX9_Texture*> gRenderTargets;

	DirectX9_Texture::DirectX9_Texture(chstr filename) : Texture()
	{
		this->filename = filename;
		this->format = FORMAT_ARGB;
		this->width = 0;
		this->height = 0;
		this->bpp = 4;
		this->renderTarget = false;
		this->d3dTexture = NULL;
		this->d3dSurface = NULL;
		hlog::write(april::logTag, "Creating DX9 texture: " + _getInternalName());
	}

	DirectX9_Texture::DirectX9_Texture(int w, int h, unsigned char* rgba) : Texture()
	{
		this->filename = "";
		this->format = FORMAT_ARGB;
		this->width = w;
		this->height = h;
		this->bpp = 4;
		this->renderTarget = false;
		this->d3dSurface = NULL;
		hlog::write(april::logTag, "Creating user-defined DX9 texture.");
		HRESULT hr = APRIL_D3D_DEVICE->CreateTexture(this->width, this->height, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &this->d3dTexture, NULL);
		if (hr != D3D_OK)
		{
			hlog::error(april::logTag, "Failed to create DX9 texture!");
			return;
		}
		// TODO - this will be removed once format/native format enums have been implemented
		unsigned char* bgra = new unsigned char[this->width * this->height * this->bpp];
		memcpy(bgra, rgba, this->width * this->height * this->bpp * sizeof(unsigned char)); // so alpha doesn't have to be copied in each iteration
		int offset;
		int i;
		int j;
		for_iterx (j, 0, this->height)
		{
			for_iterx (i, 0, this->width)
			{
				offset = (i + j * this->width) * this->bpp;
				bgra[offset + 2] = rgba[offset + 0];
				//bgra[offset + 1] = rgba[offset + 1]; // not necessary to be executed because of previous memcpy call
				bgra[offset + 0] = rgba[offset + 2];
			}
		}
		this->blit(0, 0, bgra, this->width, this->height, this->bpp, 0, 0, this->width, this->height);
		delete [] bgra;
	}
	
	DirectX9_Texture::DirectX9_Texture(int w, int h, Texture::Format format, Texture::Type type, Color color) : Texture()
	{
		this->filename = "";
		this->format = format;
		this->width = w;
		this->height = h;
		this->renderTarget = false;
		this->d3dSurface = NULL;
		hlog::writef(april::logTag, "Creating empty DX9 texture [ %dx%d ].", w, h);
		D3DFORMAT d3dformat = D3DFMT_X8R8G8B8;
		this->bpp = 3;
		switch (format)
		{
		case FORMAT_ARGB:
			d3dformat = D3DFMT_A8R8G8B8;
			this->bpp = 4;
			break;
		case FORMAT_RGB:
			d3dformat = D3DFMT_X8R8G8B8;
			this->bpp = 3;
			break;
		case FORMAT_ALPHA:
			d3dformat = D3DFMT_A8;
			this->bpp = 1;
			break;
		default:
			d3dformat = D3DFMT_X8R8G8B8;
			this->bpp = 3;
			break;
		}
		D3DPOOL d3dpool = D3DPOOL_MANAGED;
		DWORD d3dusage = 0;
		if (type == TYPE_RENDER_TARGET)
		{
			d3dusage = D3DUSAGE_RENDERTARGET;
			d3dpool = D3DPOOL_DEFAULT;
			this->renderTarget = true;
			gRenderTargets += this;
		}
		HRESULT hr = APRIL_D3D_DEVICE->CreateTexture(this->width, this->height, 1, d3dusage, d3dformat, d3dpool, &this->d3dTexture, NULL);
		if (hr != D3D_OK)
		{
			hlog::error(april::logTag, "Failed to create DX9 texture!");
			return;
		}
		if (color != APRIL_COLOR_CLEAR)
		{
			this->fillRect(0, 0, this->width, this->height, color);
		}
	}
	
	void DirectX9_Texture::restore()
	{
		if (!this->renderTarget)
		{
			return;
		}
		this->unload();
		D3DFORMAT d3dformat = D3DFMT_X8R8G8B8;
		if (this->bpp == 4)
		{
			d3dformat = D3DFMT_A8R8G8B8;
		}
		D3DPOOL d3dpool = D3DPOOL_DEFAULT;
		DWORD d3dusage = D3DUSAGE_RENDERTARGET;
		HRESULT hr = APRIL_D3D_DEVICE->CreateTexture(this->width, this->height, 1, d3dusage, d3dformat, d3dpool, &this->d3dTexture, NULL);
		if (hr != D3D_OK)
		{
			hlog::error(april::logTag, "Failed to restore user-defined DX9 texture!");
			return;
		}
	}

	DirectX9_Texture::~DirectX9_Texture()
	{
		this->unload();
		if (this->renderTarget)
		{
			gRenderTargets -= this;
		}
	}

	bool DirectX9_Texture::load()
	{
		if (this->isLoaded())
		{
			return true;
		}
		hlog::write(april::logTag, "Loading DX9 texture: " + this->_getInternalName());
		ImageSource* image = NULL;
		if (this->filename != "")
		{
			image = april::loadImage(this->filename);
			if (image == NULL)
			{
				hlog::error(april::logTag, "Failed to load texture: " + this->_getInternalName());
				return false;
			}
			this->width = image->w;
			this->height = image->h;
			this->bpp = image->bpp;
		}
		if (image == NULL)
		{
			hlog::error(april::logTag, "Image source does not exist!");
			return false;
		}
		D3DFORMAT d3dformat = D3DFMT_X8R8G8B8;
		switch (image->format)
		{
		case AF_RGBA:
		case AF_BGRA:
			d3dformat = D3DFMT_A8R8G8B8;
			this->format = FORMAT_ARGB;
			break;
		case AF_RGB:
		case AF_BGR:
			d3dformat = D3DFMT_X8R8G8B8;
			this->format = FORMAT_RGB;
			break;
		case AF_GRAYSCALE:
			d3dformat = D3DFMT_A8;
			this->format = FORMAT_ALPHA;
			break;
		case AF_PALETTE:
			d3dformat = D3DFMT_A8R8G8B8;
			this->format = FORMAT_ARGB; // TODO - should be changed
			break;
		default:
			d3dformat = D3DFMT_X8R8G8B8;
			this->format = FORMAT_ARGB;
			break;
		}
		HRESULT hr = APRIL_D3D_DEVICE->CreateTexture(this->width, this->height, 1, 0, d3dformat, D3DPOOL_MANAGED, &this->d3dTexture, NULL);
		if (hr != D3D_OK)
		{
			hlog::error(april::logTag, "Failed to create DX9 texture!");
			delete image;
			return false;
		}
		// write texels
		if (image != NULL)
		{
			D3DLOCKED_RECT rect;
			this->d3dTexture->LockRect(0, &rect, NULL, D3DLOCK_DISCARD);
			// TODO - format handling like this has to be fixed/refactored
			if (image->format == AF_RGBA)
			{
				image->copyPixels(rect.pBits, AF_BGRA);
			}
			else if (image->format == AF_RGB)
			{
				image->copyPixels(rect.pBits, AF_BGR);
			}
			else if (image->format  == AF_GRAYSCALE)
			{
				image->copyPixels(rect.pBits, AF_GRAYSCALE);
			}
			else
			{
				ImageSource* tempImg = april::createEmptyImage(image->w, image->h);
				tempImg->copyImage(image);
				tempImg->copyPixels(rect.pBits, AF_BGRA);
				delete tempImg;
			}
			this->d3dTexture->UnlockRect(0);
			delete image;
		}
		_notifyLoadingListener(this);
		return true;
	}

	void DirectX9_Texture::unload()
	{
		if (this->d3dTexture != NULL)
		{
			hlog::write(april::logTag, "Unloading DX9 texture: " + this->_getInternalName());
			this->d3dTexture->Release();
			this->d3dTexture = NULL;
			if (this->d3dSurface != NULL)
			{
				this->d3dSurface->Release();
				this->d3dSurface = NULL;
			}
		}
	}

	bool DirectX9_Texture::isLoaded()
	{
		return (this->d3dTexture != NULL);
	}

	void DirectX9_Texture::clear()
	{
		D3DLOCKED_RECT lockRect;
		IDirect3DSurface9* buffer = NULL;
		LOCK_RESULT result = this->_tryLock(&buffer, &lockRect, NULL);
		if (result == LR_FAILED)
		{
			return;
		}
		memset(lockRect.pBits, 0, this->getByteSize() * sizeof(unsigned char));
		this->_unlock(buffer, result, true);
	}

	Color DirectX9_Texture::getPixel(int x, int y)
	{
		Color color = APRIL_COLOR_CLEAR;
		D3DLOCKED_RECT lockRect;
		_CREATE_RECT(rect, x, y, 1, 1);
		IDirect3DSurface9* buffer = NULL;
		LOCK_RESULT result = this->_tryLock(&buffer, &lockRect, &rect);
		if (result == LR_FAILED)
		{
			return color;
		}
		unsigned char* p = (unsigned char*)lockRect.pBits;
		if (this->bpp == 4)
		{
			color.r = p[2];
			color.g = p[1];
			color.b = p[0];
			color.a = p[3];
		}
		else if (this->bpp == 3)
		{
			color.r = p[2];
			color.g = p[1];
			color.b = p[0];
			color.a = 255;
		}
		else if (this->bpp == 1)
		{
			color.r = 255;
			color.g = 255;
			color.b = 255;
			color.a = p[0];
		}
		else
		{
			hlog::error(april::logTag, "Unsupported format for getPixel()!");
		}
		this->_unlock(buffer, result, false);
		return color;
	}

	void DirectX9_Texture::setPixel(int x, int y, Color color)
	{
		x = hclamp(x, 0, this->width - 1);
		y = hclamp(y, 0, this->height - 1);
		D3DLOCKED_RECT lockRect;
		_CREATE_RECT(rect, x, y, 1, 1);
		IDirect3DSurface9* buffer = NULL;
		LOCK_RESULT result = this->_tryLock(&buffer, &lockRect, &rect);
		if (result == LR_FAILED)
		{
			return;
		}
		unsigned char* p = (unsigned char*)lockRect.pBits;
		if (this->bpp == 4)
		{
			p[2] = color.r;
			p[1] = color.g;
			p[0] = color.b;
			p[3] = color.a;
		}
		else if (this->bpp == 3)
		{
			p[2] = color.r;
			p[1] = color.g;
			p[0] = color.b;
		}
		else if (this->bpp == 1)
		{
			p[0] = (color.r + color.g + color.b) / 3;
		}
		else
		{
			hlog::error(april::logTag, "Unsupported format for setPixel()!");
		}
		this->_unlock(buffer, result, true);
	}

	void DirectX9_Texture::fillRect(int x, int y, int w, int h, Color color)
	{
		x = hclamp(x, 0, this->width - 1);
		y = hclamp(y, 0, this->height - 1);
		w = hclamp(w, 1, this->width - x);
		h = hclamp(h, 1, this->height - y);
		if (w == 1 && h == 1)
		{
			this->setPixel(x, y, color);
			return;
		}
		D3DLOCKED_RECT lockRect;
		_CREATE_RECT(rect, x, y, w, h);
		IDirect3DSurface9* buffer = NULL;
		LOCK_RESULT result = this->_tryLock(&buffer, &lockRect, &rect);
		if (result == LR_FAILED)
		{
			return;
		}
		unsigned char* p = (unsigned char*)lockRect.pBits;
		int i;
		int j;
		int offset;
		if (this->bpp == 4)
		{
			for_iterx (j, 0, h)
			{
				for_iterx (i, 0, w)
				{
					offset = (j * this->width + i) * this->bpp;
					p[offset + 2] = color.r;
					p[offset + 1] = color.g;
					p[offset + 0] = color.b;
					p[offset + 3] = color.a;
				}
			}
		}
		else if (this->bpp == 3)
		{
			for_iterx (j, 0, h)
			{
				for_iterx (i, 0, w)
				{
					offset = (i + j * this->width) * this->bpp;
					p[offset + 2] = color.r;
					p[offset + 1] = color.g;
					p[offset + 0] = color.b;
				}
			}
		}
		else if (this->bpp == 1)
		{
			for_iterx (j, 0, h)
			{
				for_iterx (i, 0, w)
				{
					offset = (i + j * this->width) * this->bpp;
					p[offset] = (color.r + color.g + color.b) / 3;
				}
			}
		}
		else
		{
			hlog::error(april::logTag, "Unsupported format for setPixel()!");
		}
		this->_unlock(buffer, result, true);
	}

	void DirectX9_Texture::blit(int x, int y, Texture* texture, int sx, int sy, int sw, int sh, unsigned char alpha)
	{
		DirectX9_Texture* source = (DirectX9_Texture*)texture;
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
		D3DLOCKED_RECT lockRect;
		_CREATE_RECT(rect, sx, sy, sw, sh);
		IDirect3DSurface9* buffer = NULL;
		LOCK_RESULT result = source->_tryLock(&buffer, &lockRect, &rect);
		if (result == LR_FAILED)
		{
			return;
		}
		this->blit(x, y, (unsigned char*)lockRect.pBits, source->width, source->height, source->bpp, sx, sy, sw, sh, alpha);
		source->_unlock(buffer, result, false);
	}

	void DirectX9_Texture::blit(int x, int y, unsigned char* data, int dataWidth, int dataHeight, int dataBpp, int sx, int sy, int sw, int sh, unsigned char alpha)
	{
		x = hclamp(x, 0, this->width - 1);
		y = hclamp(y, 0, this->height - 1);
		sx = hclamp(sx, 0, dataWidth - 1);
		sy = hclamp(sy, 0, dataHeight - 1);
		sw = hmin(sw, hmin(this->width - x, dataWidth - sx));
		sh = hmin(sh, hmin(this->height - y, dataHeight - sy));
		D3DLOCKED_RECT lockRect;
		_CREATE_RECT(rect, x, y, sw, sh);
		IDirect3DSurface9* buffer = NULL;
		LOCK_RESULT result = this->_tryLock(&buffer, &lockRect, &rect);
		if (result == LR_FAILED)
		{
			return;
		}
		this->_blit((unsigned char*)lockRect.pBits, x, y, data, dataWidth, dataHeight, dataBpp, sx, sy, sw, sh, alpha);
		this->_unlock(buffer, result, true);
	}

	void DirectX9_Texture::stretchBlit(int x, int y, int w, int h, Texture* texture, int sx, int sy, int sw, int sh, unsigned char alpha)
	{
		DirectX9_Texture* source = (DirectX9_Texture*)texture;
		x = hclamp(x, 0, this->width - 1);
		y = hclamp(y, 0, this->height - 1);
		w = hmin(w, this->width - x);
		h = hmin(h, this->height - y);
		sx = hclamp(sx, 0, source->width - 1);
		sy = hclamp(sy, 0, source->height - 1);
		sw = hmin(sw, source->width - sx);
		sh = hmin(sh, source->height - sy);
		D3DLOCKED_RECT lockRect;
		HRESULT result = source->_getTexture()->LockRect(0, &lockRect, NULL, D3DLOCK_DISCARD);
		if (result != D3D_OK)
		{
			return;
		}
		this->stretchBlit(x, y, w, h, (unsigned char*)lockRect.pBits, source->width, source->height, source->bpp, sx, sy, sw, sh, alpha);
		source->_getTexture()->UnlockRect(0);
	}

	void DirectX9_Texture::stretchBlit(int x, int y, int w, int h, unsigned char* data, int dataWidth, int dataHeight, int dataBpp, int sx, int sy, int sw, int sh, unsigned char alpha)
	{
		x = hclamp(x, 0, this->width - 1);
		y = hclamp(y, 0, this->height - 1);
		w = hmin(w, this->width - x);
		h = hmin(h, this->height - y);
		sx = hclamp(sx, 0, dataWidth - 1);
		sy = hclamp(sy, 0, dataHeight - 1);
		sw = hmin(sw, dataWidth - sx);
		sh = hmin(sh, dataHeight - sy);
		D3DLOCKED_RECT lockRect;
		_CREATE_RECT(rect, x, y, w, h);
		IDirect3DSurface9* buffer = NULL;
		LOCK_RESULT result = this->_tryLock(&buffer, &lockRect, &rect);
		if (result == LR_FAILED)
		{
			return;
		}
		this->_stretchBlit((unsigned char*)lockRect.pBits, x, y, w, h, data, dataWidth, dataHeight, dataBpp, sx, sy, sw, sh, alpha);
		this->_unlock(buffer, result, true);
	}

	void DirectX9_Texture::rotateHue(float degrees)
	{
		if (degrees == 0.0f)
		{
			return;
		}
		D3DLOCKED_RECT lockRect;
		IDirect3DSurface9* buffer = NULL;
		LOCK_RESULT result = this->_tryLock(&buffer, &lockRect, NULL);
		if (result == LR_FAILED)
		{
			return;
		}
		int size = this->getByteSize();
		float range = hmodf(degrees, 360.0f) / 360.0f;
		float h;
		float s;
		float l;
		unsigned char* data = (unsigned char*)lockRect.pBits;
		for_iter_step (i, 0, size, this->bpp)
		{
			april::rgbToHsl(data[i + 2], data[i + 1], data[i], &h, &s, &l);
			april::hslToRgb(hmodf(h + range, 1.0f), s, l, &data[i + 2], &data[i + 1], &data[i]);
		}
		this->_unlock(buffer, result, true);
	}

	void DirectX9_Texture::saturate(float factor)
	{
		D3DLOCKED_RECT lockRect;
		IDirect3DSurface9* buffer = NULL;
		LOCK_RESULT result = this->_tryLock(&buffer, &lockRect, NULL);
		if (result == LR_FAILED)
		{
			return;
		}
		int size = this->getByteSize();
		float h;
		float s;
		float l;
		unsigned char* data = (unsigned char*)lockRect.pBits;
		for_iter_step (i, 0, size, this->bpp)
		{
			april::rgbToHsl(data[i + 2], data[i + 1], data[i], &h, &s, &l);
			april::hslToRgb(h, hmin(s * factor, 1.0f), l, &data[i + 2], &data[i + 1], &data[i]);
		}
		this->_unlock(buffer, result, true);
	}

	bool DirectX9_Texture::copyPixelData(unsigned char** output)
	{
		D3DLOCKED_RECT lockRect;
		IDirect3DSurface9* buffer = NULL;
		LOCK_RESULT result = this->_tryLock(&buffer, &lockRect, NULL);
		if (result == LR_FAILED)
		{
			return false;
		}
		unsigned char* p = (unsigned char*)lockRect.pBits;
		int i;
		int j;
		int offset;
		*output = new unsigned char[this->width * this->height * 4];
		if (this->bpp == 4)
		{
			memcpy(*output, p, this->getByteSize() * sizeof(unsigned char));
			for_iterx (j, 0, this->height)
			{
				for_iterx (i, 0, this->width)
				{
					offset = (j * this->width + i) * 4;
					(*output)[offset + 0] = p[offset + 2];
					//(*output)[offset + 1] = p[offset + 1]; // not necessary to be executed because of previous memcpy call
					(*output)[offset + 2] = p[offset + 0];
				}
			}
		}
		else if (this->bpp == 3)
		{
			for_iterx (j, 0, this->height)
			{
				for_iterx (i, 0, this->width)
				{
					offset = (j * this->width + i) * 4;
					(*output)[offset + 0] = p[offset + 2];
					(*output)[offset + 1] = p[offset + 1];
					(*output)[offset + 2] = p[offset + 0];
				}
			}
		}
		else if (this->bpp == 1)
		{
			memcpy(*output, p, this->getByteSize() * sizeof(unsigned char));
		}
		this->_unlock(buffer, result, false);
		return true;
	}

	void DirectX9_Texture::insertAsAlphaMap(Texture* texture, unsigned char median, int ambiguity)
	{
		if (this->width != texture->getWidth() || this->height != texture->getHeight() || this->bpp != 4)
		{
			return;
		}
		DirectX9_Texture* source = (DirectX9_Texture*)texture;
		D3DLOCKED_RECT lockRect;
		IDirect3DSurface9* buffer = NULL;
		LOCK_RESULT result = this->_tryLock(&buffer, &lockRect, NULL);
		if (result == LR_FAILED)
		{
			return;
		}
		D3DLOCKED_RECT srcLockRect;
		IDirect3DSurface9* srcBuffer = NULL;
		LOCK_RESULT srcResult = source->_tryLock(&srcBuffer, &srcLockRect, NULL);
		if (srcResult == LR_FAILED)
		{
			this->_unlock(buffer, result, false);
			return;
		}
		unsigned char* thisData = (unsigned char*)lockRect.pBits;
		unsigned char* srcData = (unsigned char*)srcLockRect.pBits;
		unsigned char* c;
		unsigned char* sc;
		int i;
		int j;
		int alpha;
		int min = (int)median - ambiguity / 2;
		int max = (int)median + ambiguity / 2;
		for_iterx (j, 0, this->height)
		{
			for_iterx (i, 0, this->width)
			{
				c = &thisData[(i + j * this->width) * 4];
				sc = &srcData[(i + j * this->width) * 4];
				alpha = (sc[0] + sc[1] + sc[2]) / 3;
				if (alpha < min)
				{
					c[3] = 255;
				}
				else if (alpha >= max)
				{
					c[3] = 0;
				}
				else
				{
					c[3] = (max - alpha) * 255 / ambiguity;
				}
			}
		}
		this->_unlock(buffer, result, true);
		source->_unlock(srcBuffer, srcResult, false);
	}

	IDirect3DSurface9* DirectX9_Texture::_getSurface()
	{
		if (this->d3dSurface == NULL)
		{
			this->d3dTexture->GetSurfaceLevel(0, &this->d3dSurface);
		}
		return this->d3dSurface;
	}

	DirectX9_Texture::LOCK_RESULT DirectX9_Texture::_tryLock(IDirect3DSurface9** buffer, D3DLOCKED_RECT* lockRect, RECT* rect)
	{
		HRESULT result = this->d3dTexture->LockRect(0, lockRect, rect, D3DLOCK_DISCARD);
		if (result == D3D_OK)
		{
			return LR_LOCKED;
		}
		// could be a render target
		result = APRIL_D3D_DEVICE->CreateOffscreenPlainSurface(this->width, this->height,
			(this->bpp == 4 ? D3DFMT_A8R8G8B8 : D3DFMT_X8R8G8B8), D3DPOOL_SYSTEMMEM, buffer, NULL);
		if (result != D3D_OK)
		{
			hlog::error(april::logTag, "Failed to get pixel data, CreateOffscreenPlainSurface() call failed!");
			return LR_FAILED;
		}
		result = APRIL_D3D_DEVICE->GetRenderTargetData(this->_getSurface(), *buffer);
		if (result != D3D_OK)
		{
			hlog::error(april::logTag, "Failed to get pixel data, GetRenderTargetData() call failed!");
			return LR_FAILED;
		}
		result = (*buffer)->LockRect(lockRect, rect, D3DLOCK_DISCARD);
		if (result != D3D_OK)
		{
			hlog::error(april::logTag, "Failed to get pixel data, surface lock failed!");
			return LR_FAILED;
		}
		return LR_RENDERTARGET;
	}

	void DirectX9_Texture::_unlock(IDirect3DSurface9* buffer, DirectX9_Texture::LOCK_RESULT lock, bool update)
	{
		switch (lock)
		{
		case LR_LOCKED:
			this->d3dTexture->UnlockRect(0);
			break;
		case LR_RENDERTARGET:
			buffer->UnlockRect();
			if (update)
			{
				APRIL_D3D_DEVICE->UpdateSurface(buffer, NULL, this->_getSurface(), NULL);
			}
			buffer->Release();
			break;
		default:
			break;
		}
	}

}

#endif
