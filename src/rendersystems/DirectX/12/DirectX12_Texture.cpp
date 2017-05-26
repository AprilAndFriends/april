/// @file
/// @version 4.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifdef _DIRECTX12
#include <hltypes/hlog.h>
#include <hltypes/hstring.h>

#include "april.h"
#include "DirectX12_RenderSystem.h"
#include "DirectX12_Texture.h"
#include "Image.h"

#define DX12_RENDERSYS ((DirectX12_RenderSystem*)april::rendersys)
#define D3D_DEVICE (((DirectX12_RenderSystem*)april::rendersys)->d3dDevice)

namespace april
{
	static inline void _TRY_UNSAFE(HRESULT hr, chstr errorMessage)
	{
		if (FAILED(hr))
		{
			throw Exception(hsprintf("%s - HRESULT: 0x%08X", errorMessage.cStr(), hr));
		}
	}

	DirectX12_Texture::DirectX12_Texture(bool fromResource) : DirectX_Texture(fromResource), dxgiFormat(DXGI_FORMAT_UNKNOWN)
	{
		this->d3dTexture = nullptr;
		/*
		this->d3dView = nullptr;
		this->d3dRenderTargetView = nullptr;
		*/
	}

	DirectX12_Texture::~DirectX12_Texture()
	{
	}

	bool DirectX12_Texture::_deviceCreateTexture(unsigned char* data, int size, Type type)
	{
		return true;
		this->internalType = type;
		int bpp = this->format.getBpp();
		D3D12_SUBRESOURCE_DATA textureData = {};
		textureData.pData = data;
		this->firstUpload = true;
		if (data != NULL)
		{
			Image::Format nativeFormat = april::rendersys->getNativeTextureFormat(this->format);
			if (Image::needsConversion(this->format, nativeFormat))
			{
				textureData.pData = NULL;
				Image::convertToFormat(this->width, this->height, data, this->format, (unsigned char**)&textureData.pData, nativeFormat);
				bpp = nativeFormat.getBpp();
			}
			this->firstUpload = false;
		}
		else
		{
			int dummySize = this->width * this->height * bpp;
			textureData.pData = new unsigned char[dummySize];
			memset((unsigned char*)textureData.pData, 0, dummySize);
		}
		// texture
		textureData.RowPitch = this->width * this->getBpp();
		textureData.SlicePitch = textureData.RowPitch * this->height;
		CD3DX12_RESOURCE_DESC textureDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM, this->width, this->height);
		CD3DX12_HEAP_PROPERTIES defaultHeapProperties(D3D12_HEAP_TYPE_DEFAULT);
		HRESULT hr = D3D_DEVICE->CreateCommittedResource(&defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &textureDesc,
			D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&this->d3dTexture));
		if (hr == E_OUTOFMEMORY)
		{
			static bool _preventRecursion = false;
			if (!_preventRecursion)
			{
				_preventRecursion = true;
				april::window->handleLowMemoryWarningEvent();
				_preventRecursion = false;
				hr = D3D_DEVICE->CreateCommittedResource(&defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &textureDesc,
					D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&this->d3dTexture));
			}
			if (hr == E_OUTOFMEMORY)
			{
				hlog::error(logTag, "Failed to create DX12 texture: Not enough VRAM!");
				return false;
			}
		}
		const UINT64 uploadBufferSize = GetRequiredIntermediateSize(this->d3dTexture.Get(), 0, 1);
		ComPtr<ID3D12Resource> textureUploadHeap;
		CD3DX12_HEAP_PROPERTIES uploadHeapProperties(D3D12_HEAP_TYPE_UPLOAD);
		CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);
		hr = D3D_DEVICE->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &bufferDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&textureUploadHeap));
		if (FAILED(hr))
		{
			hlog::error(logTag, "Failed to create DX12 texture!");
			return false;
		}
		ComPtr<ID3D12GraphicsCommandList> commandList = DX12_RENDERSYS->getCommandList();
		UpdateSubresources(commandList.Get(), this->d3dTexture.Get(), textureUploadHeap.Get(), 0, 0, 1, &textureData);
		commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(this->d3dTexture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = textureDesc.Format;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		D3D_DEVICE->CreateShaderResourceView(this->d3dTexture.Get(), &srvDesc, DX12_RENDERSYS->getSrvHeap()->GetCPUDescriptorHandleForHeapStart());
		DX12_RENDERSYS->executeCurrentCommands();
		DX12_RENDERSYS->waitForCommands();
		DX12_RENDERSYS->prepareNewCommands();







		/*
		D3D11_TEXTURE2D_DESC textureDesc = {0};
		textureDesc.Width = this->width;
		textureDesc.Height = this->height;
		if (type == Type::Immutable)
		{
			textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
			textureDesc.CPUAccessFlags = 0;
		}
		else
		{
			textureDesc.Usage = D3D11_USAGE_DYNAMIC;
			textureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		}
		if (type == Type::RenderTarget)
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
		HRESULT hr = D3D_DEVICE->CreateTexture2D(&textureDesc, &textureSubresourceData, &this->d3dTexture);
		if (hr == E_OUTOFMEMORY)
		{
			static bool _preventRecursion = false;
			if (!_preventRecursion)
			{
				_preventRecursion = true;
				april::window->handleLowMemoryWarningEvent();
				_preventRecursion = false;
				hr = D3D_DEVICE->CreateTexture2D(&textureDesc, &textureSubresourceData, &this->d3dTexture);
			}
			if (hr == E_OUTOFMEMORY)
			{
				hlog::error(logTag, "Failed to create DX11 texture: Not enough VRAM!");
				return false;
			}
		}
		if (textureSubresourceData.pSysMem != data)
		{
			delete[] (unsigned char*)textureSubresourceData.pSysMem;
		}
		if (FAILED(hr))
		{
			hlog::error(logTag, "Failed to create DX11 texture!");
			return false;
		}
		if (this->type == Type::RenderTarget)
		{
			D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
			renderTargetViewDesc.Format = textureDesc.Format;
			renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
			renderTargetViewDesc.Texture2D.MipSlice = 0;
			hr = D3D_DEVICE->CreateRenderTargetView(this->d3dTexture.Get(),
				&renderTargetViewDesc, &this->d3dRenderTargetView);
			if (FAILED(hr))
			{
				hlog::error(logTag, "Failed to create render target view for texture with render-to-texture!");
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
		hr = D3D_DEVICE->CreateShaderResourceView(this->d3dTexture.Get(), &textureViewDesc, &this->d3dView);
		if (FAILED(hr))
		{
			hlog::error(logTag, "Failed to create DX11 texture view!");
			return false;
		}
		*/
		return true;
	}
	
	void DirectX12_Texture::_assignFormat()
	{
		Image::Format nativeFormat = april::rendersys->getNativeTextureFormat(this->format);
		if (nativeFormat == Image::Format::BGRA)
		{
			this->dxgiFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
		}
		else if (nativeFormat == Image::Format::BGRX)
		{
			this->dxgiFormat = DXGI_FORMAT_B8G8R8X8_UNORM;
		}
		else if (nativeFormat == Image::Format::Alpha)
		{
			this->dxgiFormat = DXGI_FORMAT_R8_UNORM;
		}
		else if (nativeFormat == Image::Format::Greyscale)
		{
			this->dxgiFormat = DXGI_FORMAT_R8_UNORM;
		}
		else if (nativeFormat == Image::Format::Palette)
		{
			this->dxgiFormat = DXGI_FORMAT_B8G8R8A8_UNORM; // TODOaa - needs changing
		}
		else
		{
			this->dxgiFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
		}
	}

	bool DirectX12_Texture::_deviceDestroyTexture()
	{
		if (this->d3dTexture != nullptr)
		{
			this->d3dTexture = nullptr;
			/*
			this->d3dView = nullptr;
			this->d3dRenderTargetView = nullptr;
			*/
			return true;
		}
		return false;
	}
	
	Texture::Lock DirectX12_Texture::_tryLockSystem(int x, int y, int w, int h)
	{
		Lock lock;
		Image::Format nativeFormat = april::rendersys->getNativeTextureFormat(this->format);
		lock.activateLock(x, y, w, h, y, x, NULL, this->width, this->height, nativeFormat);
		/*
		D3D11_MAPPED_SUBRESOURCE* mappedSubResource = new D3D11_MAPPED_SUBRESOURCE();
		memset(mappedSubResource, 0, sizeof(D3D11_MAPPED_SUBRESOURCE));
		// Map() is being used for all, because UpdateSubResource() / UpdateSubResource1() seems to use Map() internally somewhere and the memory pointer can change
		HRESULT hr = D3D_DEVICE_CONTEXT->Map(this->d3dTexture.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, mappedSubResource);
		if (!FAILED(hr))
		{
			lock.systemBuffer = mappedSubResource;
			lock.activateLock(x, y, w, h, x, y, (unsigned char*)mappedSubResource->pData, mappedSubResource->RowPitch / nativeFormat.getBpp(), this->height, nativeFormat);
		}
		*/
		return lock;
	}

	bool DirectX12_Texture::_unlockSystem(Lock& lock, bool update)
	{
		return true;
		if (lock.systemBuffer == NULL)
		{
			return false;
		}
		/*
		if (update)
		{
			if (lock.locked)
			{
				if (!lock.renderTarget)
				{
					// this special hack is required because Map() with D3D11_MAP_WRITE_DISCARD can allocate any piece of memory
					if (this->data != NULL && this->data != lock.data)
					{
						Image::write(0, 0, this->width, this->height, 0, 0, this->data, this->width, this->height, this->format, lock.data, lock.dataWidth, lock.dataHeight, lock.format);
					}
					D3D_DEVICE_CONTEXT->Unmap(this->d3dTexture.Get(), 0);
				}
				else
				{
					// TODOaa - implement
				}
			}
			this->firstUpload = false;
		}
		delete (D3D11_MAPPED_SUBRESOURCE*)lock.systemBuffer;
		*/
		return update;
	}

}
#endif
