/// @file
/// @version 3.7
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifdef _DIRECTX11
#include <d3d11.h>

#include <hltypes/hlog.h>
#include <hltypes/hresource.h>
#include <hltypes/hstring.h>

#include "april.h"
#include "DirectX11_PixelShader.h"
#include "DirectX11_RenderSystem.h"

#define APRIL_D3D_DEVICE (((DirectX11_RenderSystem*)april::rendersys)->d3dDevice)

using namespace Microsoft::WRL;

namespace april
{
	DirectX11_PixelShader::DirectX11_PixelShader(chstr filename) : PixelShader(), dx11Shader(nullptr)
	{
		this->loadResource(filename);
	}

	DirectX11_PixelShader::DirectX11_PixelShader() : PixelShader(), dx11Shader(nullptr)
	{
	}

	DirectX11_PixelShader::~DirectX11_PixelShader()
	{
		this->dx11Shader = nullptr;
	}

	bool DirectX11_PixelShader::loadFile(chstr filename)
	{
		if (this->dx11Shader != nullptr)
		{
			hlog::error(logTag, "Shader already loaded.");
			return false;
		}
		unsigned char* data = NULL;
		int32_t size = 0;
		if (!this->_loadFileData(filename, &data, &size))
		{
			hlog::error(logTag, "Shader file not found: " + filename);
			return false;
		}
		HRESULT hr = APRIL_D3D_DEVICE->CreatePixelShader(data, size, NULL, &this->dx11Shader);
		delete [] data;
		if (FAILED(hr))
		{
			hlog::error(logTag, "Failed to create pixel shader!");
			return false;
		}
		return true;
	}

	bool DirectX11_PixelShader::loadResource(chstr filename)
	{
		if (this->dx11Shader != nullptr)
		{
			hlog::error(logTag, "Shader already loaded.");
			return false;
		}
		unsigned char* data = NULL;
		int32_t size = 0;
		if (!this->_loadResourceData(filename, &data, &size))
		{
			hlog::error(logTag, "Shader file not found: " + filename);
			return false;
		}
		HRESULT hr = APRIL_D3D_DEVICE->CreatePixelShader(data, size, NULL, &this->dx11Shader);
		delete [] data;
		if (FAILED(hr))
		{
			hlog::error(logTag, "Failed to create pixel shader!");
			return false;
		}
		return true;
	}

	void DirectX11_PixelShader::setConstantsB(const int* quads, unsigned int quadCount)
	{
	}

	void DirectX11_PixelShader::setConstantsI(const int* quads, unsigned int quadCount)
	{
	}

	void DirectX11_PixelShader::setConstantsF(const float* quads, unsigned int quadCount)
	{
	}

}

#endif