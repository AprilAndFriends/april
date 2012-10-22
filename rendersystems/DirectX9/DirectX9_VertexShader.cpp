/// @file
/// @author  Boris Mikic
/// @version 2.42
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _DIRECTX9
#include <hltypes/hplatform.h>
#if !_HL_WINRT
#include <d3dx9shader.h>

#include <hltypes/hlog.h>
#include <hltypes/hstring.h>

#include "april.h"
#include "DirectX9_RenderSystem.h"
#include "DirectX9_VertexShader.h"

#define APRIL_D3D_DEVICE (((DirectX9_RenderSystem*)april::rendersys)->d3dDevice)

namespace april
{
	DirectX9_VertexShader::DirectX9_VertexShader(chstr filename) : VertexShader(filename), dx9Shader(NULL)
	{
	}

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

	bool DirectX9_VertexShader::compile(chstr shaderCode)
	{
		if (shaderCode == "")
		{
			hlog::error(april::logTag, "No vertex shader code given!");
			return false;
		}
		DWORD flags = D3DXSHADER_OPTIMIZATION_LEVEL3;
#ifdef _DEBUG
		flags |= D3DXSHADER_DEBUG;
#endif
		LPD3DXBUFFER assembly;
		HRESULT result = D3DXCompileShader(shaderCode.c_str(), shaderCode.size(),
			NULL, NULL, "main", "vs_2_0", flags, &assembly, NULL, NULL);
		if (result != D3D_OK)
		{
			hlog::error(april::logTag, "Failed to compile vertex shader!");
			return false;
		}
		result = APRIL_D3D_DEVICE->CreateVertexShader((DWORD*)assembly->GetBufferPointer(), &this->dx9Shader);
		if (result != D3D_OK)
		{
			hlog::error(april::logTag, "Failed to create vertex shader!");
			return false;
		}
		return true;
	}

	void DirectX9_VertexShader::setConstantsB(const int* quadVectors, unsigned int quadCount)
	{
		for_itert (unsigned int, i, 0, quadCount)
		{
			APRIL_D3D_DEVICE->SetVertexShaderConstantB(i, quadVectors + i * 4, 1);
		}
	}

	void DirectX9_VertexShader::setConstantsI(const int* quadVectors, unsigned int quadCount)
	{
		for_itert (unsigned int, i, 0, quadCount)
		{
			APRIL_D3D_DEVICE->SetVertexShaderConstantI(i, quadVectors + i * 4, 1);
		}
	}

	void DirectX9_VertexShader::setConstantsF(const float* quadVectors, unsigned int quadCount)
	{
		for_itert (unsigned int, i, 0, quadCount)
		{
			APRIL_D3D_DEVICE->SetVertexShaderConstantF(i, quadVectors + i * 4, 1);
		}
	}

}

#endif
#endif