/// @file
/// @version 5.2
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

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
	DirectX9_Texture::DirectX9_Texture(bool fromResource) :
		DirectX_Texture(fromResource),
		d3dTexture(NULL),
		d3dSurface(NULL),
		d3dFormat(D3DFMT_UNKNOWN)
	{
	}

	bool DirectX9_Texture::_deviceCreateTexture(unsigned char* data, int size)
	{
		this->d3dPool = D3DPOOL_DEFAULT;
		this->d3dUsage = 0;
		// some GPUs seem to have problems creating off-screen A8 surfaces when D3DPOOL_DEFAULT is used so this special hack is used
		if (!((DirectX9_RenderSystem*)april::rendersys)->_supportsA8Surface && this->d3dFormat == D3DFMT_A8)
		{
			this->d3dPool = D3DPOOL_MANAGED;
		}
		else
		{
			if (this->type == Type::RenderTarget)
			{
				this->d3dUsage = D3DUSAGE_RENDERTARGET;
			}
			else if (this->type != Type::Immutable)
			{
				this->d3dUsage = D3DUSAGE_DYNAMIC;
			}
		}
		HRESULT hr = APRIL_D3D_DEVICE->CreateTexture(this->width, this->height, 1, this->d3dUsage, this->d3dFormat, this->d3dPool, &this->d3dTexture, NULL);
		if (hr == D3DERR_OUTOFVIDEOMEMORY)
		{
			static bool _preventRecursion = false;
			if (!_preventRecursion)
			{
				_preventRecursion = true;
				april::window->handleLowMemoryWarning();
				_preventRecursion = false;
				hr = APRIL_D3D_DEVICE->CreateTexture(this->width, this->height, 1, this->d3dUsage, this->d3dFormat, this->d3dPool, &this->d3dTexture, NULL);
			}
			if (hr == D3DERR_OUTOFVIDEOMEMORY)
			{
				hlog::error(logTag, "Failed to create DX9 texture: Not enough VRAM!");
				return false;
			}
		}
		if (FAILED(hr))
		{
			RenderSystem::Caps caps = april::rendersys->getCaps();
			// maybe it failed, because NPOT textures aren't supported
			if (!caps.npotTexturesLimited && !caps.npotTextures)
			{
				int w = this->width;
				int h = this->height;
				this->_setupPot(w, h);
				hr = APRIL_D3D_DEVICE->CreateTexture(w, h, 1, this->d3dUsage, this->d3dFormat, this->d3dPool, &this->d3dTexture, NULL);
				if (!FAILED(hr) && data != NULL)
				{
					unsigned char* newData = this->_createPotData(w, h, data);
					this->_rawWrite(0, 0, w, h, 0, 0, newData, w, h, this->format);
					delete[] newData;
					this->firstUpload = false;
				}
			}
			if (FAILED(hr))
			{
				hlog::error(logTag, "Failed to create DX9 texture!");
				return false;
			}
		}
		return true;
	}
	
	bool DirectX9_Texture::_deviceDestroyTexture()
	{
		if (this->d3dTexture != NULL)
		{
			if (this->d3dSurface != NULL)
			{
				this->d3dSurface->Release();
				this->d3dSurface = NULL;
			}
			this->d3dTexture->Release();
			this->d3dTexture = NULL;
			return true;
		}
		return false;
	}

	void DirectX9_Texture::_assignFormat()
	{
		Image::Format nativeFormat = april::rendersys->getNativeTextureFormat(this->format);
		if (nativeFormat == Image::Format::BGRA)			this->d3dFormat = D3DFMT_A8R8G8B8;
		else if (nativeFormat == Image::Format::BGRX)		this->d3dFormat = D3DFMT_X8R8G8B8;
		else if (nativeFormat == Image::Format::Alpha)		this->d3dFormat = D3DFMT_A8;
		else if (nativeFormat == Image::Format::Greyscale)	this->d3dFormat = D3DFMT_L8;
		else if (nativeFormat == Image::Format::Compressed)	this->d3dFormat = D3DFMT_A8R8G8B8; // TODOaa - needs changing, ARGB shouldn't be here
		else if (nativeFormat == Image::Format::Palette)	this->d3dFormat = D3DFMT_A8R8G8B8; // TODOaa - needs changing, ARGB shouldn't be here
	}

	IDirect3DSurface9* DirectX9_Texture::_getSurface()
	{
		if (this->d3dSurface == NULL)
		{
			this->d3dTexture->GetSurfaceLevel(0, &this->d3dSurface);
		}
		return this->d3dSurface;
	}

	Texture::Lock DirectX9_Texture::_tryLockSystem(int x, int y, int w, int h)
	{
		Lock lock;
		D3DLOCKED_RECT lockRect;
		HRESULT hr;
		Image::Format nativeFormat = april::rendersys->getNativeTextureFormat(this->format);
		int nativeBpp = nativeFormat.getBpp();
		RECT rect;
		rect.left = x;
		rect.top = y;
		rect.right = x + w;
		rect.bottom = y + h;
		if (this->d3dPool == D3DPOOL_MANAGED)
		{
			hr = this->d3dTexture->LockRect(0, &lockRect, &rect, D3DLOCK_DISCARD);
			if (!FAILED(hr))
			{
				lock.systemBuffer = this->d3dTexture;
				lock.activateLock(0, 0, w, h, x, y, (unsigned char*)lockRect.pBits, lockRect.Pitch / nativeBpp, h, nativeFormat);
			}
			return lock;
		}
		IDirect3DSurface9* surface = NULL;
		if (this->type != Type::RenderTarget)
		{
			hr = APRIL_D3D_DEVICE->CreateOffscreenPlainSurface(w, h, this->d3dFormat, D3DPOOL_SYSTEMMEM, &surface, NULL);
			if (FAILED(hr))
			{
				return lock;
			}
			hr = surface->LockRect(&lockRect, NULL, D3DLOCK_DISCARD);
			if (FAILED(hr))
			{
				surface->Release();
				return lock;
			}
			// a D3DLOCKED_RECT always has a "pitch" that is a multiple of 4
			lock.activateLock(0, 0, w, h, x, y, (unsigned char*)lockRect.pBits, lockRect.Pitch / nativeBpp, h, nativeFormat);
		}
		else
		{
			hr = APRIL_D3D_DEVICE->CreateOffscreenPlainSurface(this->width, this->height, this->d3dFormat, D3DPOOL_SYSTEMMEM, &surface, NULL);
			if (FAILED(hr))
			{
				return lock;
			}
			hr = APRIL_D3D_DEVICE->GetRenderTargetData(this->_getSurface(), surface);
			if (FAILED(hr))
			{
				surface->Release();
				return lock;
			}
			hr = surface->LockRect(&lockRect, NULL, D3DLOCK_DISCARD);
			if (FAILED(hr))
			{
				surface->Release();
				return lock;
			}
			// a D3DLOCKED_RECT always has a "pitch" that is a multiple of 4
			lock.activateLock(x, y, w, h, 0, 0, (unsigned char*)lockRect.pBits, lockRect.Pitch / nativeBpp, this->height, nativeFormat);
			lock.renderTarget = true;
		}
		lock.systemBuffer = surface;
		return lock;
	}

	bool DirectX9_Texture::_unlockSystem(Lock& lock, bool update)
	{
		if (lock.systemBuffer == NULL)
		{
			return false;
		}
		if (this->d3dPool == D3DPOOL_MANAGED)
		{
			IDirect3DTexture9* texture = (IDirect3DTexture9*)lock.systemBuffer;
			if (lock.locked)
			{
				texture->UnlockRect(0);
			}
		}
		else if (update)
		{
			IDirect3DSurface9* surface = (IDirect3DSurface9*)lock.systemBuffer;
			if (lock.locked)
			{
				surface->UnlockRect();
				if (!lock.renderTarget)
				{
					RECT rect;
					rect.left = lock.x;
					rect.top = lock.y;
					rect.right = lock.x + lock.w;
					rect.bottom = lock.y + lock.h;
					POINT dest;
					dest.x = lock.dx;
					dest.y = lock.dy;
					APRIL_D3D_DEVICE->UpdateSurface(surface, &rect, this->_getSurface(), &dest);
				}
				else
				{
					APRIL_D3D_DEVICE->UpdateSurface(surface, NULL, this->_getSurface(), NULL);
				}
			}
			surface->Release();
		}
		return true;
	}

}
#endif
