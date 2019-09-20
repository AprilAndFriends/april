/// @file
/// @version 5.2
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifdef _DIRECTX12
#include <comdef.h>

#include <hltypes/hlog.h>
#include <hltypes/hstring.h>

#include "april.h"
#include "DirectX12_RenderSystem.h"
#include "DirectX12_Texture.h"
#include "Image.h"

#define DX_RENDERSYS ((DirectX12_RenderSystem*)april::rendersys)
#define D3D_DEVICE (((DirectX12_RenderSystem*)april::rendersys)->d3dDevice)

namespace april
{
	static inline void _TRY_UNSAFE(HRESULT hr, chstr errorMessage)
	{
		if (FAILED(hr))
		{
			hstr systemError;
			try
			{
				char message[1024] = { 0 };
				FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, NULL, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), message, 1023, NULL);
				systemError = hstr(message).replaced("\r\n", "\n").trimmedRight('\n');
			}
			catch (...)
			{
			}
			throw Exception(hsprintf("%s - SYSTEM ERROR: '%s' - HRESULT: 0x%08X", errorMessage.cStr(), systemError.cStr(), hr));
		}
	}

	DirectX12_Texture::DirectX12_Texture(bool fromResource) : DirectX_Texture(fromResource), dxgiFormat(DXGI_FORMAT_UNKNOWN)
	{
		this->d3dTexture = nullptr;
		this->uploadHeap = nullptr;
	}
	
	void* DirectX12_Texture::getBackendId() const
	{
		return (void*)this->d3dTexture.Get();
	}

	bool DirectX12_Texture::_deviceCreateTexture(unsigned char* data, int size)
	{
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
		textureData.RowPitch = this->width * bpp;
		textureData.SlicePitch = textureData.RowPitch * this->height;
		CD3DX12_RESOURCE_DESC textureDesc = CD3DX12_RESOURCE_DESC::Tex2D(this->dxgiFormat, this->width, this->height);
		CD3DX12_HEAP_PROPERTIES defaultHeapProperties(D3D12_HEAP_TYPE_DEFAULT);
		HRESULT hr = D3D_DEVICE->CreateCommittedResource(&defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &textureDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&this->d3dTexture));
		if (hr == E_OUTOFMEMORY)
		{
			static bool _preventRecursion = false;
			if (!_preventRecursion)
			{
				_preventRecursion = true;
				april::window->handleLowMemoryWarning();
				_preventRecursion = false;
				hr = D3D_DEVICE->CreateCommittedResource(&defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &textureDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&this->d3dTexture));
			}
			if (hr == E_OUTOFMEMORY)
			{
				hlog::error(logTag, "Failed to create DX12 texture: Not enough VRAM!");
				return false;
			}
		}
		if (FAILED(hr))
		{
			this->d3dTexture = nullptr;
			hlog::error(logTag, "Failed to create DX12 texture, unable to create committed resource!");
			return false;
		}
		const UINT64 uploadBufferSize = GetRequiredIntermediateSize(this->d3dTexture.Get(), 0, 1);
		CD3DX12_HEAP_PROPERTIES uploadHeapProperties(D3D12_HEAP_TYPE_UPLOAD);
		CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);
		hr = D3D_DEVICE->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&this->uploadHeap));
		if (FAILED(hr))
		{
			this->d3dTexture = nullptr;
			hlog::error(logTag, "Failed to create DX12 texture, unable to create upload heap!");
			return false;
		}
		// upload
		UpdateSubresources(DX_RENDERSYS->commandList[DX_RENDERSYS->commandListIndex].Get(), this->d3dTexture.Get(), this->uploadHeap.Get(), 0, 0, 1, &textureData);
		DX_RENDERSYS->commandList[DX_RENDERSYS->commandListIndex]->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(this->d3dTexture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
		return true;
	}
	
	void DirectX12_Texture::_assignFormat()
	{
		Image::Format nativeFormat = april::rendersys->getNativeTextureFormat(this->format);
		if (nativeFormat == Image::Format::RGBA)
		{
			this->dxgiFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		}
		else if (nativeFormat == Image::Format::RGBX)
		{
			this->dxgiFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		}
		else if (nativeFormat == Image::Format::Alpha)
		{
			this->dxgiFormat = DXGI_FORMAT_R8_UNORM; // A8 doesn't work for some reason
		}
		else if (nativeFormat == Image::Format::Greyscale)
		{
			this->dxgiFormat = DXGI_FORMAT_R8_UNORM;
		}
		else if (nativeFormat == Image::Format::Palette)
		{
			this->dxgiFormat = DXGI_FORMAT_R8G8B8A8_UNORM; // TODOuwp - probably implement
		}
		else
		{
			this->dxgiFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		}
	}

	bool DirectX12_Texture::_deviceDestroyTexture()
	{
		if (this->d3dTexture != nullptr)
		{
			this->d3dTexture = nullptr;
			this->uploadHeap = nullptr;
			return true;
		}
		return false;
	}
	
	Texture::Lock DirectX12_Texture::_tryLockSystem(int x, int y, int w, int h)
	{
		Lock lock;
		lock.systemBuffer = this;
		lock.activateLock(x, y, w, h, y, x, this->data, this->width, this->height, april::rendersys->getNativeTextureFormat(this->format));
		return lock;
	}

	bool DirectX12_Texture::_unlockSystem(Lock& lock, bool update)
	{
		if (lock.systemBuffer == NULL)
		{
			return false;
		}
		if (update)
		{
			if (lock.locked)
			{
				if (!lock.renderTarget)
				{
					D3D12_SUBRESOURCE_DATA textureData = {};
					textureData.pData = this->data;
					textureData.RowPitch = this->width * this->getBpp();
					textureData.SlicePitch = textureData.RowPitch * this->height;
					DX_RENDERSYS->commandList[DX_RENDERSYS->commandListIndex]->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(this->d3dTexture.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST));
					UpdateSubresources(DX_RENDERSYS->commandList[DX_RENDERSYS->commandListIndex].Get(), this->d3dTexture.Get(), this->uploadHeap.Get(), 0, 0, 1, &textureData);
					DX_RENDERSYS->commandList[DX_RENDERSYS->commandListIndex]->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(this->d3dTexture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
				}
				else
				{
					// TODOuwp - probably implement
				}
			}
			this->firstUpload = false;
		}
		return update;
	}

}
#endif
