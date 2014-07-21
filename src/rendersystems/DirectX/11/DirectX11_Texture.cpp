/// @file
/// @version 3.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifdef _DIRECTX11
#include <hltypes/hlog.h>
#include <hltypes/hstring.h>

#include "april.h"
#include "DirectX11_RenderSystem.h"
#include "DirectX11_Texture.h"
#include "Image.h"

#define APRIL_D3D_DEVICE (((DirectX11_RenderSystem*)april::rendersys)->d3dDevice)
#define APRIL_D3D_DEVICE_CONTEXT (((DirectX11_RenderSystem*)april::rendersys)->d3dDeviceContext)

namespace april
{
	DirectX11_Texture::DirectX11_Texture(bool fromResource) : DirectX_Texture(fromResource), dxgiFormat(DXGI_FORMAT_UNKNOWN)
	{
		this->d3dTexture = nullptr;
		this->d3dView = nullptr;
		this->d3dRenderTargetView = nullptr;
	}

	DirectX11_Texture::~DirectX11_Texture()
	{
		this->unload();
	}

	bool DirectX11_Texture::_createInternalTexture(unsigned char* data, int size, Type type)
	{
		this->internalType = type;
		int bpp = Image::getFormatBpp(this->format);
		D3D11_SUBRESOURCE_DATA textureSubresourceData = {0};
		textureSubresourceData.pSysMem = data;
		this->firstUpload = true;
		if (data != NULL)
		{
			Image::Format nativeFormat = april::rendersys->getNativeTextureFormat(this->format);
			if (Image::needsConversion(this->format, nativeFormat))
			{
				textureSubresourceData.pSysMem = NULL;
				Image::convertToFormat(this->width, this->height, data, this->format, (unsigned char**)&textureSubresourceData.pSysMem, nativeFormat);
				bpp = Image::getFormatBpp(nativeFormat);
			}
			this->firstUpload = false;
		}
		else
		{
			int dummySize = this->width * this->height * bpp;
			textureSubresourceData.pSysMem = new unsigned char[dummySize];
			memset((unsigned char*)textureSubresourceData.pSysMem, 0, dummySize);
		}
		// texture
		textureSubresourceData.SysMemPitch = this->width * bpp;
		textureSubresourceData.SysMemSlicePitch = 0;
		D3D11_TEXTURE2D_DESC textureDesc = {0};
		textureDesc.Width = this->width;
		textureDesc.Height = this->height;
		if (type == TYPE_IMMUTABLE)
		{
			textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
			textureDesc.CPUAccessFlags = 0;
		}
		else
		{
			textureDesc.Usage = D3D11_USAGE_DYNAMIC;
			textureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		}
		if (type == TYPE_RENDER_TARGET)
		{
			textureDesc.BindFlags |= D3D11_BIND_RENDER_TARGET;
		}
		textureDesc.MiscFlags = 0;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.SampleDesc.Quality = 0;
		textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		textureDesc.Format = this->dxgiFormat;
		HRESULT hr = APRIL_D3D_DEVICE->CreateTexture2D(&textureDesc, &textureSubresourceData, &this->d3dTexture);
		if (textureSubresourceData.pSysMem != data)
		{
			delete [] (unsigned char*)textureSubresourceData.pSysMem;
		}
		if (FAILED(hr))
		{
			hlog::error(april::logTag, "Failed to create DX11 texture!");
			return false;
		}
		if (this->type == TYPE_RENDER_TARGET)
		{
			D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
			renderTargetViewDesc.Format = textureDesc.Format;
			renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
			renderTargetViewDesc.Texture2D.MipSlice = 0;
			hr = APRIL_D3D_DEVICE->CreateRenderTargetView(this->d3dTexture.Get(),
				&renderTargetViewDesc, &this->d3dRenderTargetView);
			if (FAILED(hr))
			{
				hlog::error(april::logTag, "Failed to create render target view for texture with render-to-texture!");
				return false;
			}
		}
		// shader resource
		D3D11_SHADER_RESOURCE_VIEW_DESC textureViewDesc;
		memset(&textureViewDesc, 0, sizeof(textureViewDesc));
		textureViewDesc.Format = textureDesc.Format;
		textureViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		textureViewDesc.Texture2D.MipLevels = textureDesc.MipLevels;
		textureViewDesc.Texture2D.MostDetailedMip = 0;
		hr = APRIL_D3D_DEVICE->CreateShaderResourceView(this->d3dTexture.Get(), &textureViewDesc, &this->d3dView);
		if (FAILED(hr))
		{
			hlog::error(april::logTag, "Failed to create DX11 texture view!");
			return false;
		}
		return true;
	}
	
	void DirectX11_Texture::_assignFormat()
	{
		Image::Format nativeFormat = april::rendersys->getNativeTextureFormat(this->format);
		switch (nativeFormat)
		{
		case Image::FORMAT_BGRA:
			this->dxgiFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
			break;
		case Image::FORMAT_BGRX:
			this->dxgiFormat = DXGI_FORMAT_B8G8R8X8_UNORM;
			break;
		case Image::FORMAT_ALPHA:
			this->dxgiFormat = DXGI_FORMAT_R8_UNORM;
			break;
		case Image::FORMAT_GRAYSCALE:
			this->dxgiFormat = DXGI_FORMAT_R8_UNORM;
			break;
		case Image::FORMAT_PALETTE: // TODOaa - needs changing
			this->dxgiFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
			break;
		default:
			this->dxgiFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
			break;
		}
	}

	void DirectX11_Texture::unload()
	{
		if (this->d3dTexture != nullptr)
		{
			hlog::write(april::logTag, "Unloading texture: " + this->_getInternalName());
			this->d3dTexture = nullptr;
			this->d3dView = nullptr;
			this->d3dRenderTargetView = nullptr;
		}
	}
	
	bool DirectX11_Texture::isLoaded()
	{
		return (this->d3dTexture != nullptr);
	}

	Texture::Lock DirectX11_Texture::_tryLockSystem(int x, int y, int w, int h)
	{
		Lock lock;
		Image::Format nativeFormat = april::rendersys->getNativeTextureFormat(this->format);
		int gpuBpp = Image::getFormatBpp(nativeFormat);
		D3D11_MAPPED_SUBRESOURCE* mappedSubResource = new D3D11_MAPPED_SUBRESOURCE();
		memset(mappedSubResource, 0, sizeof(D3D11_MAPPED_SUBRESOURCE));
		// Map() is being used for all, because UpdateSubResource() / UpdateSubResource1() seems to use Map() internally somewhere and the memory pointer can change
		HRESULT hr = APRIL_D3D_DEVICE_CONTEXT->Map(this->d3dTexture.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, mappedSubResource);
		if (!FAILED(hr))
		{
			lock.systemBuffer = mappedSubResource;
			lock.activateLock(x, y, w, h, x, y, (unsigned char*)mappedSubResource->pData, mappedSubResource->RowPitch / gpuBpp, this->height, nativeFormat);
		}
		return lock;
	}

	bool DirectX11_Texture::_unlockSystem(Lock& lock, bool update)
	{
		if (lock.systemBuffer == NULL)
		{
			return false;
		}
		if (update)
		{
			if (lock.locked)
			{
				// this special hack is required because Map() with D3D11_MAP_WRITE_DISCARD can allocate any piece of memory
				if (this->data != NULL && this->data != lock.data)
				{
					Image::write(0, 0, this->width, this->height, 0, 0, this->data, this->width, this->height, this->format, lock.data, lock.dataWidth, lock.dataHeight, lock.format);
				}
				APRIL_D3D_DEVICE_CONTEXT->Unmap(this->d3dTexture.Get(), 0);
			}
			else if (lock.renderTarget)
			{
				// TODOaa - implement
			}
			this->firstUpload = false;
		}
		delete (D3D11_MAPPED_SUBRESOURCE*)lock.systemBuffer;
		return update;
	}

}
#endif
