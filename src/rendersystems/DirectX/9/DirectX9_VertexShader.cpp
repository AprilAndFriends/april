/// @file
/// @version 4.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifdef _DIRECTX9
#include <d3dx9shader.h>

#include <hltypes/hlog.h>
#include <hltypes/hstring.h>

#include "april.h"
#include "DirectX9_RenderSystem.h"
#include "DirectX9_VertexShader.h"

#define APRIL_D3D_DEVICE (((DirectX9_RenderSystem*)april::rendersys)->d3dDevice)

namespace april
{
	DirectX9_VertexShader::DirectX9_VertexShader() : VertexShader(), dx9Shader(NULL)
	{
	}

	DirectX9_VertexShader::~DirectX9_VertexShader()
	{
		if (this->dx9Shader != NULL)
		{
			this->dx9Shader->Release();
			this->dx9Shader = NULL;
		}
	}

	bool DirectX9_VertexShader::isLoaded() const
	{
		return (this->dx9Shader != NULL);
	}

	bool DirectX9_VertexShader::_createShader(chstr filename, const hstream& stream)
	{
		HRESULT result = APRIL_D3D_DEVICE->CreateVertexShader((DWORD*)(unsigned char*)stream, &this->dx9Shader);
		if (result != D3D_OK)
		{
			hlog::error(logTag, "Failed to create vertex shader!");
			return false;
		}
		return true;
	}

	void DirectX9_VertexShader::setConstantsB(const int* quads, unsigned int quadCount)
	{
		for_itert (unsigned int, i, 0, quadCount)
		{
			APRIL_D3D_DEVICE->SetVertexShaderConstantB(i, quads + i * 4, 1);
		}
	}

	void DirectX9_VertexShader::setConstantsI(const int* quads, unsigned int quadCount)
	{
		for_itert (unsigned int, i, 0, quadCount)
		{
			APRIL_D3D_DEVICE->SetVertexShaderConstantI(i, quads + i * 4, 1);
		}
	}

	void DirectX9_VertexShader::setConstantsF(const float* quads, unsigned int quadCount)
	{
		for_itert (unsigned int, i, 0, quadCount)
		{
			APRIL_D3D_DEVICE->SetVertexShaderConstantF(i, quads + i * 4, 1);
		}
	}

}

#endif