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

namespace april
{
	DirectX9_Texture::Lock::Lock()
	{
		this->buffer = NULL;
		this->data = NULL;
		this->x = 0;
		this->y = 0;
		this->w = 0;
		this->h = 0;
		this->locked = false;
		this->failed = true;
		this->renderTarget = false;
	}

	DirectX9_Texture::Lock::~Lock()
	{
	}

	void DirectX9_Texture::Lock::activateFail()
	{
		this->locked = false;
		this->failed = true;
		this->renderTarget = false;
	}

	void DirectX9_Texture::Lock::activateLock(int x, int y, int w, int h, unsigned char* data)
	{
		this->x = x;
		this->y = y;
		this->w = w;
		this->h = h;
		this->data = data;
		this->locked = true;
		this->failed = false;
		this->renderTarget = false;
	}

	void DirectX9_Texture::Lock::activateRenderTarget(int x, int y, int w, int h, unsigned char* data)
	{
		this->x = x;
		this->y = y;
		this->w = w;
		this->h = h;
		this->data = data;
		this->locked = false;
		this->failed = false;
		this->renderTarget = true;
	}

	DirectX9_Texture::DirectX9_Texture() : DirectX_Texture(), d3dTexture(NULL), d3dSurface(NULL), d3dFormat(D3DFMT_UNKNOWN), renderTarget(false)
	{
	}

	DirectX9_Texture::~DirectX9_Texture()
	{
		this->unload();
	}

	bool DirectX9_Texture::_createInternalTexture(unsigned char* data, int size, Type type)
	{
		this->d3dPool = D3DPOOL_DEFAULT;
		this->d3dUsage = 0;
		// TODOaa - change pool to save memory
		/*
		if (type == TYPE_RENDER_TARGET)
		{
			this->d3dUsage = D3DUSAGE_RENDERTARGET;
			this->renderTarget = true;
		}
		*/
		HRESULT hr = APRIL_D3D_DEVICE->CreateTexture(this->width, this->height, 1, this->d3dUsage, this->d3dFormat, this->d3dPool, &this->d3dTexture, NULL);
		if (FAILED(hr))
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
			if (this->d3dSurface != NULL)
			{
				this->d3dSurface->Release();
				this->d3dSurface = NULL;
			}
			this->d3dTexture->Release();
			this->d3dTexture = NULL;
		}
	}

	bool DirectX9_Texture::isLoaded()
	{
		return (this->d3dTexture != NULL);
	}

	bool DirectX9_Texture::clear()
	{
		if (this->type == TYPE_IMMUTABLE)
		{
			hlog::warn(april::logTag, "Changing texture not possible: " + this->_getInternalName());
			return false;
		}
		if (this->data != NULL)
		{
			return Texture::clear();
		}
		Lock lock = this->_tryLock();
		if (lock.isFailed())
		{
			return false;
		}
		memset(lock.data, 0, this->getByteSize());
		return this->_unlock(lock, true);
	}

	Color DirectX9_Texture::getPixel(int x, int y)
	{
		if (this->type != TYPE_MANAGED)
		{
			hlog::warn(april::logTag, "Reading texture not possible: " + this->_getInternalName());
			return false;
		}
		if (this->data != NULL) // get from RAM if possible to avoid downloading from the GPU
		{
			return Texture::getPixel(x, y);
		}
		Color color = Color::Clear;
		Lock lock = this->_tryLock(x, y, 1, 1);
		if (lock.isFailed())
		{
			return color;
		}
		color = Image::getPixel(x, y, lock.data, lock.w, lock.h, april::rendersys->getNativeTextureFormat(this->format));
		this->_unlock(lock, false);
		return color;
	}

	bool DirectX9_Texture::setPixel(int x, int y, Color color)
	{
		if (this->type == TYPE_IMMUTABLE)
		{
			hlog::warn(april::logTag, "Changing texture not possible: " + this->_getInternalName());
			return false;
		}
		if (this->data != NULL)
		{
			return Texture::setPixel(x, y, color);
		}
		Lock lock = this->_tryLock(x, y, 1, 1);
		if (lock.isFailed())
		{
			return false;
		}
		return this->_unlock(lock, Image::setPixel(0, 0, color, lock.data, lock.w, lock.h, april::rendersys->getNativeTextureFormat(this->format)));
	}

	bool DirectX9_Texture::fillRect(int x, int y, int w, int h, Color color)
	{
		if (this->type == TYPE_IMMUTABLE)
		{
			hlog::warn(april::logTag, "Changing texture not possible: " + this->_getInternalName());
			return false;
		}
		if (w == 1 && h == 1)
		{
			return this->setPixel(x, y, color);
		}
		if (this->data != NULL)
		{
			return Texture::fillRect(x, y, w, h, color);
		}
		Lock lock = this->_tryLock(x, y, w, h);
		if (lock.isFailed())
		{
			return false;
		}
		return this->_unlock(lock, Image::fillRect(0, 0, w, h, color, lock.data, lock.w, lock.h, april::rendersys->getNativeTextureFormat(this->format)));
	}

	bool DirectX9_Texture::copyPixelData(unsigned char** output, Image::Format format)
	{
		if (this->type != TYPE_MANAGED)
		{
			hlog::warn(april::logTag, "Reading texture not possible: " + this->_getInternalName());
			return false;
		}
		if (this->data != NULL)
		{
			return Texture::copyPixelData(output);
		}
		if (!this->isLoaded())
		{
			return false;
		}
		Lock lock = this->_tryLock();
		if (lock.isFailed())
		{
			return false;
		}
		bool result = Image::convertToFormat(this->width, this->height, lock.data, april::rendersys->getNativeTextureFormat(this->format), output, format, false);
		this->_unlock(lock, false);
		return result;
	}

	bool DirectX9_Texture::write(int sx, int sy, int sw, int sh, int dx, int dy, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat)
	{
		if (this->type == TYPE_IMMUTABLE)
		{
			hlog::warn(april::logTag, "Changing texture not possible: " + this->_getInternalName());
			return false;
		}
		if (this->data != NULL)
		{
			return DirectX_Texture::write(sx, sy, sw, sh, dx, dy, srcData, srcWidth, srcHeight, srcFormat);
		}
		Lock lock = this->_tryLock(dx, dy, sw, sh);
		if (lock.isFailed())
		{
			return false;
		}
		return this->_unlock(lock, Image::write(sx, sy, sw, sh, 0, 0, srcData, srcWidth, srcHeight, srcFormat, lock.data, lock.w, lock.h, april::rendersys->getNativeTextureFormat(this->format)));
	}

	bool DirectX9_Texture::write(int sx, int sy, int sw, int sh, int dx, int dy, Texture* texture)
	{
		DirectX9_Texture* source = dynamic_cast<DirectX9_Texture*>(texture);
		if (source == NULL || !source->isLoaded())
		{
			return false;
		}
		Image::Format srcFormat = source->format;
		Lock lock;
		lock.data = source->data;
		lock.w = source->width;
		lock.h = source->height;
		if (lock.data == NULL)
		{
			lock = source->_tryLock(sx, sy, sw, sh);
			if (lock.isFailed())
			{
				return false;
			}
			srcFormat = april::rendersys->getNativeTextureFormat(srcFormat);
		}
		bool result = this->write(sx, sy, sw, sh, dx, dy, lock.data, lock.w, lock.h, srcFormat);
		if (!lock.isFailed())
		{
			source->_unlock(lock, false);
		}
		return result;
	}

	bool DirectX9_Texture::writeStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat)
	{
		if (this->type == TYPE_IMMUTABLE)
		{
			hlog::warn(april::logTag, "Changing texture not possible: " + this->_getInternalName());
			return false;
		}
		if (this->data != NULL)
		{
			return DirectX_Texture::writeStretch(sx, sy, sw, sh, dx, dy, dw, dh, srcData, srcWidth, srcHeight, srcFormat);
		}
		Lock lock = this->_tryLock(dx, dy, dw, dh);
		if (lock.isFailed())
		{
			return false;
		}
		return this->_unlock(lock, Image::writeStretch(sx, sy, sw, sh, 0, 0, dw, dh, srcData, srcWidth, srcHeight, srcFormat, lock.data, lock.w, lock.h, april::rendersys->getNativeTextureFormat(this->format)));
	}

	bool DirectX9_Texture::writeStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, Texture* texture)
	{
		DirectX9_Texture* source = dynamic_cast<DirectX9_Texture*>(texture);
		if (source == NULL || !source->isLoaded())
		{
			return false;
		}
		Image::Format srcFormat = source->format;
		Lock lock;
		lock.data = source->data;
		lock.w = source->width;
		lock.h = source->height;
		if (lock.data == NULL)
		{
			lock = source->_tryLock(sx, sy, sw, sh);
			if (lock.isFailed())
			{
				return false;
			}
			srcFormat = april::rendersys->getNativeTextureFormat(srcFormat);
		}
		bool result = this->writeStretch(sx, sy, sw, sh, dx, dy, dw, dh, lock.data, lock.w, lock.h, april::rendersys->getNativeTextureFormat(source->format));
		if (!lock.isFailed())
		{
			source->_unlock(lock, false);
		}
		return result;
	}

	bool DirectX9_Texture::blit(int sx, int sy, int sw, int sh, int dx, int dy, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat, unsigned char alpha)
	{
		if (this->type == TYPE_IMMUTABLE)
		{
			hlog::warn(april::logTag, "Changing texture not possible: " + this->_getInternalName());
			return false;
		}
		if (this->data != NULL)
		{
			return DirectX_Texture::blit(sx, sy, sw, sh, dx, dy, srcData, srcWidth, srcHeight, srcFormat, alpha);
		}
		Lock lock = this->_tryLock(dx, dy, sw, sh);
		if (lock.isFailed())
		{
			return false;
		}
		return this->_unlock(lock, Image::blit(sx, sy, sw, sh, 0, 0, srcData, srcWidth, srcHeight, srcFormat, lock.data, lock.w, lock.h, april::rendersys->getNativeTextureFormat(this->format), alpha));
	}

	bool DirectX9_Texture::blit(int sx, int sy, int sw, int sh, int dx, int dy, Texture* texture, unsigned char alpha)
	{
		DirectX9_Texture* source = dynamic_cast<DirectX9_Texture*>(texture);
		if (source == NULL || !source->isLoaded())
		{
			return false;
		}
		Image::Format srcFormat = source->format;
		Lock lock;
		lock.data = source->data;
		lock.w = source->width;
		lock.h = source->height;
		if (lock.data == NULL)
		{
			lock = source->_tryLock(sx, sy, sw, sh);
			if (lock.isFailed())
			{
				return false;
			}
			srcFormat = april::rendersys->getNativeTextureFormat(srcFormat);
		}
		bool result = this->blit(sx, sy, sw, sh, dx, dy, lock.data, lock.w, lock.h, srcFormat, alpha);
		if (!lock.isFailed())
		{
			source->_unlock(lock, false);
		}
		return result;
	}

	bool DirectX9_Texture::blitStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat, unsigned char alpha)
	{
		if (this->type == TYPE_IMMUTABLE)
		{
			hlog::warn(april::logTag, "Changing texture not possible: " + this->_getInternalName());
			return false;
		}
		if (this->data != NULL)
		{
			return DirectX_Texture::blitStretch(sx, sy, sw, sh, dx, dy, dw, dh, srcData, srcWidth, srcHeight, srcFormat, alpha);
		}
		Lock lock = this->_tryLock(dx, dy, dw, dh);
		if (lock.isFailed())
		{
			return false;
		}
		return this->_unlock(lock, Image::blitStretch(sx, sy, sw, sh, 0, 0, dw, dh, srcData, srcWidth, srcHeight, srcFormat, lock.data, lock.w, lock.h, april::rendersys->getNativeTextureFormat(this->format), alpha));
	}

	bool DirectX9_Texture::blitStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, Texture* texture, unsigned char alpha)
	{
		DirectX9_Texture* source = dynamic_cast<DirectX9_Texture*>(texture);
		if (source == NULL || !source->isLoaded())
		{
			return false;
		}
		Image::Format srcFormat = source->format;
		Lock lock;
		lock.data = source->data;
		lock.w = source->width;
		lock.h = source->height;
		if (lock.data == NULL)
		{
			lock = source->_tryLock(sx, sy, sw, sh);
			if (lock.isFailed())
			{
				return false;
			}
			srcFormat = april::rendersys->getNativeTextureFormat(srcFormat);
		}
		bool result = this->blitStretch(sx, sy, sw, sh, dx, dy, dw, dh, lock.data, lock.w, lock.h, srcFormat, alpha);
		if (!lock.isFailed())
		{
			source->_unlock(lock, false);
		}
		return result;
	}

	bool DirectX9_Texture::rotateHue(int x, int y, int w, int h, float degrees)
	{
		if (this->type == TYPE_IMMUTABLE)
		{
			hlog::warn(april::logTag, "Changing texture not possible: " + this->_getInternalName());
			return false;
		}
		if (this->data != NULL)
		{
			return DirectX_Texture::rotateHue(x, y, w, h, degrees);
		}
		Lock lock = this->_tryLock();
		if (lock.isFailed())
		{
			return false;
		}
		return this->_unlock(lock, Image::rotateHue(0, 0, w, h, degrees, lock.data, lock.w, lock.h, april::rendersys->getNativeTextureFormat(this->format)));
	}

	bool DirectX9_Texture::saturate(int x, int y, int w, int h, float factor)
	{
		if (this->type == TYPE_IMMUTABLE)
		{
			hlog::warn(april::logTag, "Changing texture not possible: " + this->_getInternalName());
			return false;
		}
		if (this->data != NULL)
		{
			return DirectX_Texture::saturate(x, y, w, h, factor);
		}
		Lock lock = this->_tryLock();
		if (lock.isFailed())
		{
			return false;
		}
		return this->_unlock(lock, Image::saturate(0, 0, w, h, factor, lock.data, lock.w, lock.h, april::rendersys->getNativeTextureFormat(this->format)));
	}

	bool DirectX9_Texture::invert(int x, int y, int w, int h)
	{
		if (this->type == TYPE_IMMUTABLE)
		{
			hlog::warn(april::logTag, "Changing texture not possible: " + this->_getInternalName());
			return false;
		}
		if (this->data != NULL)
		{
			return DirectX_Texture::invert(x, y, w, h);
		}
		Lock lock = this->_tryLock();
		if (lock.isFailed())
		{
			return false;
		}
		return this->_unlock(lock, Image::invert(0, 0, w, h, lock.data, lock.w, lock.h, april::rendersys->getNativeTextureFormat(this->format)));
	}

	bool DirectX9_Texture::insertAlphaMap(unsigned char* srcData, Image::Format srcFormat, unsigned char median, int ambiguity)
	{
		if (this->type == TYPE_IMMUTABLE)
		{
			hlog::warn(april::logTag, "Changing texture not possible: " + this->_getInternalName());
			return false;
		}
		if (this->data != NULL)
		{
			return DirectX_Texture::insertAlphaMap(srcData, srcFormat, median, ambiguity);
		}
		Lock lock = this->_tryLock();
		if (lock.isFailed())
		{
			return false;
		}
		return this->_unlock(lock, Image::insertAlphaMap(this->width, this->height, srcData, srcFormat, lock.data, april::rendersys->getNativeTextureFormat(this->format), median, ambiguity));
	}

	bool DirectX9_Texture::insertAlphaMap(Texture* texture, unsigned char median, int ambiguity)
	{
		DirectX9_Texture* source = dynamic_cast<DirectX9_Texture*>(texture);
		if (source == NULL || !source->isLoaded() || source->width != this->width || source->height != this->height)
		{
			return false;
		}
		Image::Format srcFormat = source->format;
		Lock lock;
		lock.data = source->data;
		lock.w = source->width;
		lock.h = source->height;
		if (lock.data == NULL)
		{
			lock = source->_tryLock();
			if (lock.isFailed())
			{
				return false;
			}
			srcFormat = april::rendersys->getNativeTextureFormat(srcFormat);
		}
		bool result = this->insertAlphaMap(lock.data, srcFormat, median, ambiguity);
		if (!lock.isFailed())
		{
			source->_unlock(lock, false);
		}
		return result;
	}

	IDirect3DSurface9* DirectX9_Texture::_getSurface()
	{
		if (this->d3dSurface == NULL)
		{
			this->d3dTexture->GetSurfaceLevel(0, &this->d3dSurface);
		}
		return this->d3dSurface;
	}

	DirectX9_Texture::Lock DirectX9_Texture::_tryLock()
	{
		return this->_tryLock(0, 0, this->width, this->height);
		// TODOaa - render target locking
		/*
		Lock lock;
		D3DLOCKED_RECT lockRect;
		HRESULT hr = this->d3dTexture->LockRect(0, &lockRect, NULL, D3DLOCK_DISCARD);
		if (!FAILED(hr))
		{
			lock.buffer = this->_getSurface();
			lock.activateLock((unsigned char*)lockRect.pBits);
			return lock;
		}
		// could be a render target
		hr = APRIL_D3D_DEVICE->CreateOffscreenPlainSurface(this->width, this->height, this->d3dFormat, D3DPOOL_SYSTEMMEM, &lock.buffer, NULL);
		if (FAILED(hr))
		{
			hlog::error(april::logTag, "Failed to get pixel data, CreateOffscreenPlainSurface() call failed!");
			return lock;
		}
		hr = APRIL_D3D_DEVICE->GetRenderTargetData(this->_getSurface(), lock.buffer);
		if (FAILED(hr))
		{
			hlog::error(april::logTag, "Failed to get pixel data, GetRenderTargetData() call failed!");
			return lock;
		}
		hr = lock.buffer->LockRect(&lockRect, NULL, D3DLOCK_DISCARD);
		if (FAILED(hr))
		{
			hlog::error(april::logTag, "Failed to get pixel data, surface lock failed!");
			return lock;
		}
		lock.activateRenderTarget((unsigned char*)lockRect.pBits);
		return lock;
		*/
	}

	DirectX9_Texture::Lock DirectX9_Texture::_tryLock(int x, int y, int w, int h)
	{
		Lock lock;
		HRESULT hr = APRIL_D3D_DEVICE->CreateOffscreenPlainSurface(w, h, this->d3dFormat, D3DPOOL_SYSTEMMEM, &lock.buffer, NULL);
		if (FAILED(hr))
		{
			return lock;
		}
		D3DLOCKED_RECT lockRect;
		hr = lock.buffer->LockRect(&lockRect, NULL, D3DLOCK_DISCARD);
		if (FAILED(hr))
		{
			lock.buffer->Release();
			lock.buffer = NULL;
			return lock;
		}
		lock.activateLock(x, y, w, h, (unsigned char*)lockRect.pBits);
		return lock;
	}

	bool DirectX9_Texture::_unlock(Lock lock, bool update)
	{
		if (lock.isLocked())
		{
			lock.buffer->UnlockRect();
			POINT dest;
			dest.x = lock.x;
			dest.y = lock.y;
			APRIL_D3D_DEVICE->UpdateSurface(lock.buffer, NULL, this->_getSurface(), &dest);
			lock.buffer->Release();
			lock.buffer = NULL;
		}
		else if (lock.isRenderTarget())
		{
			// TODOaa - implement
			/*
			lock.buffer->UnlockRect();
			if (update)
			{
				APRIL_D3D_DEVICE->UpdateSurface(lock.buffer, NULL, this->_getSurface(), NULL);
			}
			lock.buffer->Release();
			lock.buffer = NULL;
			*/
		}
		return update;
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
		Lock lock = this->_tryLock(x, y, w, h);
		if (lock.isFailed())
		{
			return false;
		}
		return this->_unlock(lock, Image::write(x, y, w, h, 0, 0, this->data, this->width, this->height, this->format, lock.data, lock.w, lock.h, april::rendersys->getNativeTextureFormat(this->format)));
	}

}

#endif
