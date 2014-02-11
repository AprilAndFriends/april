/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @version 3.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _DIRECTX9
#include <d3d9.h>

#include <hltypes/hlog.h>
#include <hltypes/hstring.h>

#include "april.h"
#include "Color.h"
#include "DirectX9_RenderSystem.h"
#include "DirectX9_Texture.h"
#include "Image.h"

#define APRIL_D3D_DEVICE (((DirectX9_RenderSystem*)april::rendersys)->d3dDevice)
#define _CREATE_RECT(name, x, y, w, h) \
	RECT name; \
	name.left = x; \
	name.top = y; \
	name.right = x + w - 1; \
	name.bottom = y + h - 1;

namespace april
{
	// TODOaa - refactor
	extern harray<DirectX9_Texture*> gRenderTargets;

	DirectX9_Texture::DirectX9_Texture() : DirectX_Texture(), d3dTexture(NULL), d3dSurface(NULL), d3dFormat(D3DFMT_UNKNOWN), renderTarget(false)
	{
	}

	DirectX9_Texture::~DirectX9_Texture()
	{
		if (this->renderTarget)
		{
			gRenderTargets -= this;
		}
		this->unload();
	}

	void DirectX9_Texture::restore()
	{
		// TODOaa - needs refactoring
		if (!this->renderTarget)
		{
			return;
		}
		this->unload();
		this->_assignFormat();
		this->_createInternalTexture(this->data, this->getByteSize());
	}

	bool DirectX9_Texture::_createInternalTexture(unsigned char* data, int size)
	{
		// TODOaa - change pool to save memory
		if (this->renderTarget)
		{
			this->d3dPool = D3DPOOL_MANAGED;
			this->d3dUsage = 0;
			/*
			if (type == TYPE_RENDER_TARGET)
			{
				d3dusage = D3DUSAGE_RENDERTARGET;
				d3dpool = D3DPOOL_DEFAULT;
				this->renderTarget = true;
				gRenderTargets += this;
			}
			*/
		}
		else
		{
			this->d3dPool = D3DPOOL_DEFAULT;
			this->d3dUsage = D3DUSAGE_RENDERTARGET;
		}
		HRESULT hr = APRIL_D3D_DEVICE->CreateTexture(this->width, this->height, 1, this->d3dUsage, this->d3dFormat, this->d3dPool, &this->d3dTexture, NULL);
		if (hr != D3D_OK)
		{
			hlog::error(april::logTag, "Failed to create DX9 texture!");
			return false;
		}
		return true;
	}
	
	void DirectX9_Texture::_assignFormat()
	{
		Image::Format nativeFormat = april::rendersys->getNativeTextureFormat(this->format);
		switch (nativeFormat)
		{
		case Image::FORMAT_BGRA:
			this->d3dFormat = D3DFMT_A8R8G8B8;
			break;
		case Image::FORMAT_BGRX:
			this->d3dFormat = D3DFMT_X8R8G8B8;
			break;
		case Image::FORMAT_ALPHA:
			this->d3dFormat = D3DFMT_A8;
			break;
		case Image::FORMAT_GRAYSCALE:
			this->d3dFormat = D3DFMT_A8;
			break;
		case Image::FORMAT_PALETTE: // TODOaa - needs changing, ARGB shouldn't be here
			this->d3dFormat = D3DFMT_A8R8G8B8;
			break;
		}
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
		if (this->data != NULL)
		{
			Texture::clear();
			return;
		}
		D3DLOCKED_RECT lockRect;
		IDirect3DSurface9* buffer = NULL;
		LOCK_RESULT lockResult = this->_tryLock(&buffer, &lockRect, NULL);
		if (lockResult == LR_FAILED)
		{
			return;
		}
		memset(lockRect.pBits, 0, this->getByteSize());
		this->_unlock(buffer, lockResult, true);
	}

	Color DirectX9_Texture::getPixel(int x, int y)
	{
		if (this->data != NULL) // get from RAM if possible to avoid downloading from the GPU
		{
			return Texture::getPixel(x, y);
		}
		Color color = Color::Clear;
		D3DLOCKED_RECT lockRect;
		_CREATE_RECT(rect, x, y, 1, 1);
		IDirect3DSurface9* buffer = NULL;
		LOCK_RESULT lockResult = this->_tryLock(&buffer, &lockRect, &rect);
		if (lockResult == LR_FAILED)
		{
			return color;
		}
		color = Image::getPixel(x, y, (unsigned char*)lockRect.pBits, this->width, this->height, april::rendersys->getNativeTextureFormat(this->format));
		this->_unlock(buffer, lockResult, false);
		return color;
	}

	void DirectX9_Texture::setPixel(int x, int y, Color color)
	{
		x = hclamp(x, 0, this->width - 1);
		y = hclamp(y, 0, this->height - 1);
		if (this->data != NULL)
		{
			Texture::setPixel(x, y, color);
			return;
		}
		D3DLOCKED_RECT lockRect;
		_CREATE_RECT(rect, x, y, 1, 1);
		IDirect3DSurface9* buffer = NULL;
		LOCK_RESULT lockResult = this->_tryLock(&buffer, &lockRect, &rect);
		if (lockResult == LR_FAILED)
		{
			return;
		}
		bool result =  Image::setPixel(x, y, color, (unsigned char*)lockRect.pBits, this->width, this->height, april::rendersys->getNativeTextureFormat(this->format));
		this->_unlock(buffer, lockResult, result);
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
		if (this->data != NULL)
		{
			Texture::fillRect(x, y, w, h, color);
			return;
		}
		D3DLOCKED_RECT lockRect;
		_CREATE_RECT(rect, x, y, w, h);
		IDirect3DSurface9* buffer = NULL;
		LOCK_RESULT lockResult = this->_tryLock(&buffer, &lockRect, &rect);
		if (lockResult == LR_FAILED)
		{
			return;
		}
		unsigned char* p = (unsigned char*)lockRect.pBits;
		Image::Format nativeFormat = april::rendersys->getNativeTextureFormat(this->format);
		p -= (x + y * this->width) * Image::getFormatBpp(nativeFormat); // Image::fillRect expects data from the beginning so this shift back was implemented, but will never be accessed
		bool result = Image::fillRect(x, y, w, h, color, p, this->width, this->height, nativeFormat);
		this->_unlock(buffer, lockResult, result);
	}

	void DirectX9_Texture::write(int sx, int sy, int sw, int sh, int dx, int dy, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat)
	{
		if (this->data != NULL)
		{
			DirectX_Texture::write(sx, sy, sw, sh, dx, dy, srcData, srcWidth, srcHeight, srcFormat);
			return;
		}
		D3DLOCKED_RECT lockRect;
		_CREATE_RECT(rect, dx, dy, sw, sh);
		IDirect3DSurface9* buffer = NULL;
		LOCK_RESULT lockResult = this->_tryLock(&buffer, &lockRect, &rect);
		if (lockResult == LR_FAILED)
		{
			return;
		}
		unsigned char* p = (unsigned char*)lockRect.pBits;
		Image::Format nativeFormat = april::rendersys->getNativeTextureFormat(this->format);
		p -= (dx + dy * this->width) * Image::getFormatBpp(nativeFormat); // Image::write expects data from the beginning so this shift back was implemented, but will never be accessed
		bool result = Image::write(sx, sy, sw, sh, dx, dy, srcData, srcWidth, srcHeight, srcFormat, p, this->width, this->height, nativeFormat);
		this->_unlock(buffer, lockResult, result);
	}

	bool DirectX9_Texture::copyPixelData(unsigned char** output, Image::Format format)
	{
		if (this->data != NULL)
		{
			return Texture::copyPixelData(output);
		}
		if (!this->isLoaded())
		{
			return false;
		}
		D3DLOCKED_RECT lockRect;
		IDirect3DSurface9* buffer = NULL;
		LOCK_RESULT lockResult = this->_tryLock(&buffer, &lockRect, NULL);
		if (lockResult == LR_FAILED)
		{
			return false;
		}
		unsigned char* p = (unsigned char*)lockRect.pBits;
		bool result = Image::convertToFormat(this->width, this->height, p, april::rendersys->getNativeTextureFormat(this->format), output, format, false); // will just perform a copy
		this->_unlock(buffer, lockResult, result);
		return result;
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
		LOCK_RESULT lockResult = source->_tryLock(&buffer, &lockRect, &rect);
		if (lockResult == LR_FAILED)
		{
			return;
		}
		this->blit(x, y, (unsigned char*)lockRect.pBits, source->width, source->height, source->getBpp(), sx, sy, sw, sh, alpha);
		source->_unlock(buffer, lockResult, false);
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
		LOCK_RESULT lockResult = this->_tryLock(&buffer, &lockRect, &rect);
		if (lockResult == LR_FAILED)
		{
			return;
		}
		this->_blit((unsigned char*)lockRect.pBits, x, y, data, dataWidth, dataHeight, dataBpp, sx, sy, sw, sh, alpha);
		this->_unlock(buffer, lockResult, true);
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
		_CREATE_RECT(rect, x, y, w, h);
		IDirect3DSurface9* buffer = NULL;
		LOCK_RESULT lockResult = this->_tryLock(&buffer, &lockRect, &rect);
		if (lockResult == LR_FAILED)
		{
			return;
		}
		this->stretchBlit(x, y, w, h, (unsigned char*)lockRect.pBits, source->width, source->height, source->getBpp(), sx, sy, sw, sh, alpha);
		source->_unlock(buffer, lockResult, false);
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
		LOCK_RESULT lockResult = this->_tryLock(&buffer, &lockRect, &rect);
		if (lockResult == LR_FAILED)
		{
			return;
		}
		this->_stretchBlit((unsigned char*)lockRect.pBits, x, y, w, h, data, dataWidth, dataHeight, dataBpp, sx, sy, sw, sh, alpha);
		this->_unlock(buffer, lockResult, true);
	}

	void DirectX9_Texture::rotateHue(float degrees)
	{
		// TODOaa - optimize and improve
		if (degrees == 0.0f)
		{
			return;
		}
		D3DLOCKED_RECT lockRect;
		IDirect3DSurface9* buffer = NULL;
		LOCK_RESULT lockResult = this->_tryLock(&buffer, &lockRect, NULL);
		if (lockResult == LR_FAILED)
		{
			return;
		}
		int size = this->getByteSize();
		float range = hmodf(degrees, 360.0f) / 360.0f;
		float h;
		float s;
		float l;
		unsigned char* data = (unsigned char*)lockRect.pBits;
		int bpp = this->getBpp();
		for_iter_step (i, 0, size, bpp)
		{
			april::rgbToHsl(data[i + 2], data[i + 1], data[i], &h, &s, &l);
			april::hslToRgb(hmodf(h + range, 1.0f), s, l, &data[i + 2], &data[i + 1], &data[i]);
		}
		this->_unlock(buffer, lockResult, true);
	}

	void DirectX9_Texture::saturate(float factor)
	{
		// TODOaa - optimize and improve
		D3DLOCKED_RECT lockRect;
		IDirect3DSurface9* buffer = NULL;
		LOCK_RESULT lockResult = this->_tryLock(&buffer, &lockRect, NULL);
		if (lockResult == LR_FAILED)
		{
			return;
		}
		int size = this->getByteSize();
		float h;
		float s;
		float l;
		unsigned char* data = (unsigned char*)lockRect.pBits;
		int bpp = this->getBpp();
		for_iter_step (i, 0, size, bpp)
		{
			april::rgbToHsl(data[i + 2], data[i + 1], data[i], &h, &s, &l);
			april::hslToRgb(h, hmin(s * factor, 1.0f), l, &data[i + 2], &data[i + 1], &data[i]);
		}
		this->_unlock(buffer, lockResult, true);
	}

	void DirectX9_Texture::insertAsAlphaMap(Texture* texture, unsigned char median, int ambiguity)
	{
		// TODOaa - use this->data from source if available
		if (this->width != texture->getWidth() || this->height != texture->getHeight() ||
			this->format != Image::FORMAT_RGBA && this->format != Image::FORMAT_ARGB && this->format != Image::FORMAT_BGRA && this->format != Image::FORMAT_ABGR)
		{
			return;
		}
		DirectX9_Texture* source = (DirectX9_Texture*)texture;
		D3DLOCKED_RECT lockRect;
		IDirect3DSurface9* buffer = NULL;
		LOCK_RESULT lockResult = this->_tryLock(&buffer, &lockRect, NULL);
		if (lockResult == LR_FAILED)
		{
			return;
		}
		D3DLOCKED_RECT srcLockRect;
		IDirect3DSurface9* srcBuffer = NULL;
		LOCK_RESULT srcResult = source->_tryLock(&srcBuffer, &srcLockRect, NULL);
		if (srcResult == LR_FAILED)
		{
			this->_unlock(buffer, lockResult, false);
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
		source->_unlock(srcBuffer, srcResult, false);
		this->_unlock(buffer, lockResult, true);
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
		result = APRIL_D3D_DEVICE->CreateOffscreenPlainSurface(this->width, this->height, this->d3dFormat, D3DPOOL_SYSTEMMEM, buffer, NULL);
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

	bool DirectX9_Texture::_uploadDataToGpu(int x, int y, int w, int h)
	{
		if (this->data == NULL)
		{
			return false;
		}
		D3DLOCKED_RECT lockRect;
		_CREATE_RECT(rect, x, y, w, h);
		IDirect3DSurface9* buffer = NULL;
		LOCK_RESULT lockResult = this->_tryLock(&buffer, &lockRect, &rect);
		if (lockResult == LR_FAILED)
		{
			return false;
		}
		unsigned char* p = (unsigned char*)lockRect.pBits;
		Image::Format nativeFormat = april::rendersys->getNativeTextureFormat(this->format);
		p -= (x + y * this->width) * Image::getFormatBpp(nativeFormat); // Image::write expects data from the beginning so this shift back was implemented, but will never be accessed
		bool result = Image::write(x, y, w, h, x, y, this->data, this->width, this->height, this->format, p, this->width, this->height, nativeFormat);
		this->_unlock(buffer, lockResult, result);
		return result;
	}

}

#endif
