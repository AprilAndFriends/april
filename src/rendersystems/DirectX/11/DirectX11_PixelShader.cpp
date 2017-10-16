/// @file
/// @version 4.5
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
	DirectX11_PixelShader::DirectX11_PixelShader() : PixelShader(), dx11Shader(nullptr)
	{
	}

	DirectX11_PixelShader::~DirectX11_PixelShader()
	{
		this->dx11Shader = nullptr;
	}
	
	bool DirectX11_PixelShader::isLoaded() const
	{
		return (this->dx11Shader != nullptr);
	}

	bool DirectX11_PixelShader::_createShader(chstr filename, const hstream& stream)
	{
		HRESULT hr = APRIL_D3D_DEVICE->CreatePixelShader((unsigned char*)stream, (unsigned int)stream.size(), NULL, &this->dx11Shader);
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