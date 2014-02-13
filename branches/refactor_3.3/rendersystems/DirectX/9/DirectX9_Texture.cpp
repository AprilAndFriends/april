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
		if (!this->renderTarget)
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
			this->d3dFormat = D3DFMT_L8;
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

	bool DirectX9_Texture::clear()
	{
		if (this->data != NULL)
		{
			return Texture::clear();
		}
		D3DLOCKED_RECT lockRect;
		IDirect3DSurface9* buffer = NULL;
		LOCK_RESULT lockResult = this->_tryLock(&buffer, &lockRect, NULL);
		if (lockResult == LR_FAILED)
		{
			return false;
		}
		memset(lockRect.pBits, 0, this->getByteSize());
		this->_unlock(buffer, lockResult, true);
		return true;
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
		unsigned char* p = (unsigned char*)lockRect.pBits;
		Image::Format nativeFormat = april::rendersys->getNativeTextureFormat(this->format);
		p -= (x + y * this->width) * Image::getFormatBpp(nativeFormat); // Image::getPixel expects data from the beginning so this shift back was implemented, but will never be accessed
		color = Image::getPixel(x, y, p, this->width, this->height, nativeFormat);
		this->_unlock(buffer, lockResult, false);
		return color;
	}

	bool DirectX9_Texture::setPixel(int x, int y, Color color)
	{
		if (this->data != NULL)
		{
			return Texture::setPixel(x, y, color);
		}
		D3DLOCKED_RECT lockRect;
		_CREATE_RECT(rect, x, y, 1, 1);
		IDirect3DSurface9* buffer = NULL;
		LOCK_RESULT lockResult = this->_tryLock(&buffer, &lockRect, &rect);
		if (lockResult == LR_FAILED)
		{
			return false;
		}
		unsigned char* p = (unsigned char*)lockRect.pBits;
		Image::Format nativeFormat = april::rendersys->getNativeTextureFormat(this->format);
		p -= (x + y * this->width) * Image::getFormatBpp(nativeFormat); // Image::setPixel expects data from the beginning so this shift back was implemented, but will never be accessed
		bool result = Image::setPixel(x, y, color, p, this->width, this->height, nativeFormat);
		this->_unlock(buffer, lockResult, result);
		return result;
	}

	bool DirectX9_Texture::fillRect(int x, int y, int w, int h, Color color)
	{
		if (w == 1 && h == 1)
		{
			return this->setPixel(x, y, color);
		}
		if (this->data != NULL)
		{
			return Texture::fillRect(x, y, w, h, color);
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
		p -= (x + y * this->width) * Image::getFormatBpp(nativeFormat); // Image::fillRect expects data from the beginning so this shift back was implemented, but will never be accessed
		bool result = Image::fillRect(x, y, w, h, color, p, this->width, this->height, nativeFormat);
		this->_unlock(buffer, lockResult, result);
		return result;
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

	bool DirectX9_Texture::write(int sx, int sy, int sw, int sh, int dx, int dy, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat)
	{
		if (this->data != NULL)
		{
			return DirectX_Texture::write(sx, sy, sw, sh, dx, dy, srcData, srcWidth, srcHeight, srcFormat);
		}
		D3DLOCKED_RECT lockRect;
		_CREATE_RECT(rect, dx, dy, sw, sh);
		IDirect3DSurface9* buffer = NULL;
		LOCK_RESULT lockResult = this->_tryLock(&buffer, &lockRect, &rect);
		if (lockResult == LR_FAILED)
		{
			return false;
		}
		unsigned char* p = (unsigned char*)lockRect.pBits;
		Image::Format nativeFormat = april::rendersys->getNativeTextureFormat(this->format);
		p -= (dx + dy * this->width) * Image::getFormatBpp(nativeFormat); // Image::write expects data from the beginning so this shift back was implemented, but will never be accessed
		bool result = Image::write(sx, sy, sw, sh, dx, dy, srcData, srcWidth, srcHeight, srcFormat, p, this->width, this->height, nativeFormat);
		this->_unlock(buffer, lockResult, result);
		return result;
	}

	bool DirectX9_Texture::writeStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat)
	{
		if (this->data != NULL)
		{
			return DirectX_Texture::writeStretch(sx, sy, sw, sh, dx, dy, dw, dh, srcData, srcWidth, srcHeight, srcFormat);
		}
		D3DLOCKED_RECT lockRect;
		_CREATE_RECT(rect, dx, dy, dw, dh);
		IDirect3DSurface9* buffer = NULL;
		LOCK_RESULT lockResult = this->_tryLock(&buffer, &lockRect, &rect);
		if (lockResult == LR_FAILED)
		{
			return false;
		}
		unsigned char* p = (unsigned char*)lockRect.pBits;
		Image::Format nativeFormat = april::rendersys->getNativeTextureFormat(this->format);
		p -= (dx + dy * this->width) * Image::getFormatBpp(nativeFormat); // Image::write expects data from the beginning so this shift back was implemented, but will never be accessed
		bool result = Image::writeStretch(sx, sy, sw, sh, dx, dy, dw, dh, srcData, srcWidth, srcHeight, srcFormat, p, this->width, this->height, nativeFormat);
		this->_unlock(buffer, lockResult, result);
		return result;
	}

	bool DirectX9_Texture::blit(int sx, int sy, int sw, int sh, int dx, int dy, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat, unsigned char alpha)
	{
		if (this->data != NULL)
		{
			return DirectX_Texture::blit(sx, sy, sw, sh, dx, dy, srcData, srcWidth, srcHeight, srcFormat, alpha);
		}
		D3DLOCKED_RECT lockRect;
		_CREATE_RECT(rect, dx, dy, sw, sh);
		IDirect3DSurface9* buffer = NULL;
		LOCK_RESULT lockResult = this->_tryLock(&buffer, &lockRect, &rect);
		if (lockResult == LR_FAILED)
		{
			return false;
		}
		unsigned char* p = (unsigned char*)lockRect.pBits;
		Image::Format nativeFormat = april::rendersys->getNativeTextureFormat(this->format);
		p -= (dx + dy * this->width) * Image::getFormatBpp(nativeFormat); // Image::blit expects data from the beginning so this shift back was implemented, but will never be accessed
		bool result = Image::blit(sx, sy, sw, sh, dx, dy, srcData, srcWidth, srcHeight, srcFormat, p, this->width, this->height, nativeFormat, alpha);
		this->_unlock(buffer, lockResult, result);
		return result;
	}

	bool DirectX9_Texture::blitStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat, unsigned char alpha)
	{
		if (this->data != NULL)
		{
			return DirectX_Texture::blitStretch(sx, sy, sw, sh, dx, dy, dw, dh, srcData, srcWidth, srcHeight, srcFormat, alpha);
		}
		D3DLOCKED_RECT lockRect;
		_CREATE_RECT(rect, dx, dy, dw, dh);
		IDirect3DSurface9* buffer = NULL;
		LOCK_RESULT lockResult = this->_tryLock(&buffer, &lockRect, &rect);
		if (lockResult == LR_FAILED)
		{
			return false;
		}
		unsigned char* p = (unsigned char*)lockRect.pBits;
		Image::Format nativeFormat = april::rendersys->getNativeTextureFormat(this->format);
		p -= (dx + dy * this->width) * Image::getFormatBpp(nativeFormat); // Image::write expects data from the beginning so this shift back was implemented, but will never be accessed
		bool result = Image::blitStretch(sx, sy, sw, sh, dx, dy, dw, dh, srcData, srcWidth, srcHeight, srcFormat, p, this->width, this->height, nativeFormat, alpha);
		this->_unlock(buffer, lockResult, result);
		return result;
	}

	bool DirectX9_Texture::rotateHue(float degrees)
	{
		degrees = hmodf(degrees, 360.0f);
		// TODOaa - optimize and improve
		if (degrees == 0.0f)
		{
			return true;
		}
		D3DLOCKED_RECT lockRect;
		IDirect3DSurface9* buffer = NULL;
		LOCK_RESULT lockResult = this->_tryLock(&buffer, &lockRect, NULL);
		if (lockResult == LR_FAILED)
		{
			return false;
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
		return true;
	}

	bool DirectX9_Texture::saturate(float factor)
	{
		// TODOaa - optimize and improve
		D3DLOCKED_RECT lockRect;
		IDirect3DSurface9* buffer = NULL;
		LOCK_RESULT lockResult = this->_tryLock(&buffer, &lockRect, NULL);
		if (lockResult == LR_FAILED)
		{
			return false;
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
		return true;
	}

	bool DirectX9_Texture::insertAsAlphaMap(Texture* texture, unsigned char median, int ambiguity)
	{
		// TODOaa - use this->data from source if available
		if (this->width != texture->getWidth() || this->height != texture->getHeight() ||
			this->format != Image::FORMAT_RGBA && this->format != Image::FORMAT_ARGB && this->format != Image::FORMAT_BGRA && this->format != Image::FORMAT_ABGR)
		{
			return false;
		}
		DirectX9_Texture* source = (DirectX9_Texture*)texture;
		D3DLOCKED_RECT lockRect;
		IDirect3DSurface9* buffer = NULL;
		LOCK_RESULT lockResult = this->_tryLock(&buffer, &lockRect, NULL);
		if (lockResult == LR_FAILED)
		{
			return false;
		}
		D3DLOCKED_RECT srcLockRect;
		IDirect3DSurface9* srcBuffer = NULL;
		LOCK_RESULT srcResult = source->_tryLock(&srcBuffer, &srcLockRect, NULL);
		if (srcResult == LR_FAILED)
		{
			this->_unlock(buffer, lockResult, false);
			return false;
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
		return true;
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
		if (!Image::correctRect(x, y, w, h, this->width, this->height))
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
