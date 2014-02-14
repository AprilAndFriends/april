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
	// TODOaa - refactor
	extern harray<DirectX9_Texture*> gRenderTargets;

	DirectX9_Texture::Lock::Lock()
	{
		this->buffer = NULL;
		this->data = NULL;
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

	void DirectX9_Texture::Lock::activateLock(unsigned char* data)
	{
		this->data = data;
		this->locked = true;
		this->failed = false;
		this->renderTarget = false;
	}

	void DirectX9_Texture::Lock::activateRenderTarget(unsigned char* data)
	{
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
		color = Image::getPixel(x, y, lock.data, this->width, this->height, april::rendersys->getNativeTextureFormat(this->format));
		this->_unlock(lock, false);
		return color;
	}

	bool DirectX9_Texture::setPixel(int x, int y, Color color)
	{
		if (this->data != NULL)
		{
			return Texture::setPixel(x, y, color);
		}
		Lock lock = this->_tryLock(x, y, 1, 1);
		if (lock.isFailed())
		{
			return false;
		}
		return this->_unlock(lock, Image::setPixel(x, y, color, lock.data, this->width, this->height, april::rendersys->getNativeTextureFormat(this->format)));
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
		Lock lock = this->_tryLock(x, y, w, h);
		if (lock.isFailed())
		{
			return false;
		}
		return this->_unlock(lock, Image::fillRect(x, y, w, h, color, lock.data, this->width, this->height, april::rendersys->getNativeTextureFormat(this->format)));
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
		if (this->data != NULL)
		{
			return DirectX_Texture::write(sx, sy, sw, sh, dx, dy, srcData, srcWidth, srcHeight, srcFormat);
		}
		Lock lock = this->_tryLock(dx, dy, sw, sh);
		if (lock.isFailed())
		{
			return false;
		}
		return this->_unlock(lock, Image::write(sx, sy, sw, sh, dx, dy, srcData, srcWidth, srcHeight, srcFormat, lock.data, this->width, this->height, april::rendersys->getNativeTextureFormat(this->format)));
	}

	bool DirectX9_Texture::write(int sx, int sy, int sw, int sh, int dx, int dy, Texture* texture)
	{
		DirectX9_Texture* source = dynamic_cast<DirectX9_Texture*>(texture);
		if (source == NULL || !source->isLoaded())
		{
			return false;
		}
		Lock lock;
		lock.data = source->data;
		if (lock.data == NULL)
		{
			lock = source->_tryLock(sx, sy, sw, sh);
			if (lock.isFailed())
			{
				return false;
			}
		}
		bool result = this->write(sx, sy, sw, sh, dx, dy, lock.data, source->width, source->height, april::rendersys->getNativeTextureFormat(source->format));
		if (!lock.isFailed())
		{
			source->_unlock(lock, false);
		}
		return result;
	}

	bool DirectX9_Texture::writeStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat)
	{
		if (this->data != NULL)
		{
			return DirectX_Texture::writeStretch(sx, sy, sw, sh, dx, dy, dw, dh, srcData, srcWidth, srcHeight, srcFormat);
		}
		Lock lock = this->_tryLock(dx, dy, dw, dh);
		if (lock.isFailed())
		{
			return false;
		}
		return this->_unlock(lock, Image::writeStretch(sx, sy, sw, sh, dx, dy, dw, dh, srcData, srcWidth, srcHeight, srcFormat, lock.data, this->width, this->height, april::rendersys->getNativeTextureFormat(this->format)));
	}

	bool DirectX9_Texture::writeStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, Texture* texture)
	{
		DirectX9_Texture* source = dynamic_cast<DirectX9_Texture*>(texture);
		if (source == NULL || !source->isLoaded())
		{
			return false;
		}
		Lock lock;
		lock.data = source->data;
		if (lock.data == NULL)
		{
			lock = source->_tryLock(sx, sy, sw, sh);
			if (lock.isFailed())
			{
				return false;
			}
		}
		bool result = this->writeStretch(sx, sy, sw, sh, dx, dy, dw, dh, lock.data, source->width, source->height, april::rendersys->getNativeTextureFormat(source->format));
		if (!lock.isFailed())
		{
			source->_unlock(lock, false);
		}
		return result;
	}

	bool DirectX9_Texture::blit(int sx, int sy, int sw, int sh, int dx, int dy, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat, unsigned char alpha)
	{
		if (this->data != NULL)
		{
			return DirectX_Texture::blit(sx, sy, sw, sh, dx, dy, srcData, srcWidth, srcHeight, srcFormat, alpha);
		}
		Lock lock = this->_tryLock(dx, dy, sw, sh);
		if (lock.isFailed())
		{
			return false;
		}
		return this->_unlock(lock, Image::blit(sx, sy, sw, sh, dx, dy, srcData, srcWidth, srcHeight, srcFormat, lock.data, this->width, this->height, april::rendersys->getNativeTextureFormat(this->format), alpha));
	}

	bool DirectX9_Texture::blit(int sx, int sy, int sw, int sh, int dx, int dy, Texture* texture, unsigned char alpha)
	{
		DirectX9_Texture* source = dynamic_cast<DirectX9_Texture*>(texture);
		if (source == NULL || !source->isLoaded())
		{
			return false;
		}
		Lock lock;
		lock.data = source->data;
		if (lock.data == NULL)
		{
			lock = source->_tryLock(sx, sy, sw, sh);
			if (lock.isFailed())
			{
				return false;
			}
		}
		bool result = this->blit(sx, sy, sw, sh, dx, dy, lock.data, source->width, source->height, april::rendersys->getNativeTextureFormat(source->format), alpha);
		if (!lock.isFailed())
		{
			source->_unlock(lock, false);
		}
		return result;
	}

	bool DirectX9_Texture::blitStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat, unsigned char alpha)
	{
		if (this->data != NULL)
		{
			return DirectX_Texture::blitStretch(sx, sy, sw, sh, dx, dy, dw, dh, srcData, srcWidth, srcHeight, srcFormat, alpha);
		}
		Lock lock = this->_tryLock(dx, dy, dw, dh);
		if (lock.isFailed())
		{
			return false;
		}
		return this->_unlock(lock, Image::blitStretch(sx, sy, sw, sh, dx, dy, dw, dh, srcData, srcWidth, srcHeight, srcFormat, lock.data, this->width, this->height, april::rendersys->getNativeTextureFormat(this->format), alpha));
	}

	bool DirectX9_Texture::blitStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, Texture* texture, unsigned char alpha)
	{
		DirectX9_Texture* source = dynamic_cast<DirectX9_Texture*>(texture);
		if (source == NULL || !source->isLoaded())
		{
			return false;
		}
		Lock lock;
		lock.data = source->data;
		if (lock.data == NULL)
		{
			lock = source->_tryLock(sx, sy, sw, sh);
			if (lock.isFailed())
			{
				return false;
			}
		}
		bool result = this->blitStretch(sx, sy, sw, sh, dx, dy, dw, dh, lock.data, source->width, source->height, april::rendersys->getNativeTextureFormat(source->format), alpha);
		if (!lock.isFailed())
		{
			source->_unlock(lock, false);
		}
		return result;
	}

	bool DirectX9_Texture::rotateHue(int x, int y, int w, int h, float degrees)
	{
		if (this->data != NULL)
		{
			return DirectX_Texture::rotateHue(x, y, w, h, degrees);
		}
		Lock lock = this->_tryLock();
		if (lock.isFailed())
		{
			return false;
		}
		return this->_unlock(lock, Image::rotateHue(x, y, w, h, degrees, lock.data, this->width, this->height, april::rendersys->getNativeTextureFormat(this->format)));
	}

	bool DirectX9_Texture::saturate(int x, int y, int w, int h, float factor)
	{
		if (this->data != NULL)
		{
			return DirectX_Texture::saturate(x, y, w, h, factor);
		}
		Lock lock = this->_tryLock();
		if (lock.isFailed())
		{
			return false;
		}
		return this->_unlock(lock, Image::saturate(x, y, w, h, factor, lock.data, this->width, this->height, april::rendersys->getNativeTextureFormat(this->format)));
	}

	bool DirectX9_Texture::invert(int x, int y, int w, int h)
	{
		if (this->data != NULL)
		{
			return DirectX_Texture::invert(x, y, w, h);
		}
		Lock lock = this->_tryLock();
		if (lock.isFailed())
		{
			return false;
		}
		return this->_unlock(lock, Image::invert(x, y, w, h, lock.data, this->width, this->height, april::rendersys->getNativeTextureFormat(this->format)));
	}

	bool DirectX9_Texture::insertAlphaMap(unsigned char* srcData, Image::Format srcFormat, unsigned char median, int ambiguity)
	{
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
		Lock lock;
		lock.data = source->data;
		if (lock.data == NULL)
		{
			lock = source->_tryLock();
			if (lock.isFailed())
			{
				return false;
			}
		}
		bool result = this->insertAlphaMap(lock.data, source->format, median, ambiguity);
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
		Lock lock;
		D3DLOCKED_RECT lockRect;
		HRESULT result = this->d3dTexture->LockRect(0, &lockRect, NULL, D3DLOCK_DISCARD);
		if (result == D3D_OK)
		{
			lock.activateLock((unsigned char*)lockRect.pBits);
			return lock;
		}
		// could be a render target
		result = APRIL_D3D_DEVICE->CreateOffscreenPlainSurface(this->width, this->height, this->d3dFormat, D3DPOOL_SYSTEMMEM, &lock.buffer, NULL);
		if (result != D3D_OK)
		{
			hlog::error(april::logTag, "Failed to get pixel data, CreateOffscreenPlainSurface() call failed!");
			return lock;
		}
		result = APRIL_D3D_DEVICE->GetRenderTargetData(this->_getSurface(), lock.buffer);
		if (result != D3D_OK)
		{
			hlog::error(april::logTag, "Failed to get pixel data, GetRenderTargetData() call failed!");
			return lock;
		}
		result = lock.buffer->LockRect(&lockRect, NULL, D3DLOCK_DISCARD);
		if (result != D3D_OK)
		{
			hlog::error(april::logTag, "Failed to get pixel data, surface lock failed!");
			return lock;
		}
		lock.activateRenderTarget((unsigned char*)lockRect.pBits);
		return lock;
	}

	DirectX9_Texture::Lock DirectX9_Texture::_tryLock(int x, int y, int w, int h)
	{
		Lock lock;
		D3DLOCKED_RECT lockRect;
		RECT rect;
		rect.left = x;
		rect.top = y;
		rect.right = x + w - 1;
		rect.bottom = y + h - 1;
		HRESULT result = this->d3dTexture->LockRect(0, &lockRect, &rect, D3DLOCK_DISCARD);
		if (result == D3D_OK)
		{
			// all functions and methods expect data from the beginning, but non-locked data will never be accessed
			lock.activateLock((unsigned char*)lockRect.pBits - (x + y * this->width) * Image::getFormatBpp(april::rendersys->getNativeTextureFormat(this->format)));
			return lock;
		}
		// could be a render target
		result = APRIL_D3D_DEVICE->CreateOffscreenPlainSurface(this->width, this->height, this->d3dFormat, D3DPOOL_SYSTEMMEM, &lock.buffer, NULL);
		if (result != D3D_OK)
		{
			hlog::error(april::logTag, "Failed to get pixel data, CreateOffscreenPlainSurface() call failed!");
			return lock;
		}
		result = APRIL_D3D_DEVICE->GetRenderTargetData(this->_getSurface(), lock.buffer);
		if (result != D3D_OK)
		{
			hlog::error(april::logTag, "Failed to get pixel data, GetRenderTargetData() call failed!");
			return lock;
		}
		result = lock.buffer->LockRect(&lockRect, &rect, D3DLOCK_DISCARD);
		if (result != D3D_OK)
		{
			hlog::error(april::logTag, "Failed to get pixel data, surface lock failed!");
			return lock;
		}
		lock.activateRenderTarget((unsigned char*)lockRect.pBits - (x + y * this->width) * Image::getFormatBpp(april::rendersys->getNativeTextureFormat(this->format)));
		return lock;
	}

	bool DirectX9_Texture::_unlock(Lock lock, bool update)
	{
		if (lock.isLocked())
		{
			this->d3dTexture->UnlockRect(0);
		}
		else if (lock.isRenderTarget())
		{
			lock.buffer->UnlockRect();
			if (update)
			{
				APRIL_D3D_DEVICE->UpdateSurface(lock.buffer, NULL, this->_getSurface(), NULL);
			}
			lock.buffer->Release();
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
		bool result = Image::write(x, y, w, h, x, y, this->data, this->width, this->height, this->format, lock.data, this->width, this->height, april::rendersys->getNativeTextureFormat(this->format));
		this->_unlock(lock, result);
		return result;
	}

}

#endif
