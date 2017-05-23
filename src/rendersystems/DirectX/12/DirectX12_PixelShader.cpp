/// @file
/// @version 4.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifdef _DIRECTX12
#include <d3d11.h>

#include <hltypes/hlog.h>
#include <hltypes/hresource.h>
#include <hltypes/hstring.h>

#include "april.h"
#include "DirectX12_PixelShader.h"
#include "DirectX12_RenderSystem.h"

#define APRIL_D3D_DEVICE (((DirectX12_RenderSystem*)april::rendersys)->d3dDevice)

using namespace Microsoft::WRL;

namespace april
{
	DirectX12_PixelShader::DirectX12_PixelShader() : PixelShader(), dx11Shader(nullptr)
	{
	}

	DirectX12_PixelShader::~DirectX12_PixelShader()
	{
		this->dx11Shader = nullptr;
	}
	
	bool DirectX12_PixelShader::isLoaded() const
	{
		return (this->dx11Shader != nullptr);
	}

	bool DirectX12_PixelShader::_createShader(chstr filename, const hstream& stream)
	{
		this->shaderData = stream; // copies data for later usage
		return true;
	}

	void DirectX12_PixelShader::setConstantsB(const int* quads, unsigned int quadCount)
	{
	}

	void DirectX12_PixelShader::setConstantsI(const int* quads, unsigned int quadCount)
	{
	}

	void DirectX12_PixelShader::setConstantsF(const float* quads, unsigned int quadCount)
	{
	}

}

#endif