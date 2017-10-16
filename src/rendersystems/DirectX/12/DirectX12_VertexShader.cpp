/// @file
/// @version 4.5
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
#include "DirectX12_RenderSystem.h"
#include "DirectX12_VertexShader.h"

#define APRIL_D3D_DEVICE (((DirectX12_RenderSystem*)april::rendersys)->d3dDevice)

using namespace Microsoft::WRL;

namespace april
{
	DirectX12_VertexShader::DirectX12_VertexShader() : VertexShader()
	{
	}

	DirectX12_VertexShader::~DirectX12_VertexShader()
	{
	}

	bool DirectX12_VertexShader::isLoaded() const
	{
		return (this->shaderData.size() > 0);
	}

	bool DirectX12_VertexShader::_createShader(chstr filename, const hstream& stream)
	{
		this->shaderData = stream; // copies data for later usage
		return true;
	}

	void DirectX12_VertexShader::setConstantsB(const int* quads, unsigned int quadCount)
	{
	}

	void DirectX12_VertexShader::setConstantsI(const int* quads, unsigned int quadCount)
	{
	}

	void DirectX12_VertexShader::setConstantsF(const float* quads, unsigned int quadCount)
	{
	}

}

#endif