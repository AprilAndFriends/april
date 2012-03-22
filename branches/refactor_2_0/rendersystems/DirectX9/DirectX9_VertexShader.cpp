/// @file
/// @author  Boris Mikic
/// @version 2.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _DIRECTX9

#include <d3dx9shader.h>

#include "april.h"
#include "DirectX9_RenderSystem.h"
#include "DirectX9_VertexShader.h"

#define DX9_RENDERSYS ((DirectX9_RenderSystem*)april::rendersys)

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

	bool DirectX9_VertexShader::compile(chstr shaderCode)
	{
		if (shaderCode == "")
		{
			april::log("no vertex shader code given");
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
			april::log("failed to compile vertex shader");
			return false;
		}
		result = DX9_RENDERSYS->d3dDevice->CreateVertexShader((DWORD*)assembly->GetBufferPointer(), &this->dx9Shader);
		if (result != D3D_OK)
		{
			april::log("failed to create vertex shader");
			return false;
		}
		return true;
	}

	void DirectX9_VertexShader::setConstantsB(const int* quadVectors, unsigned int quadCount)
	{
		for_itert (unsigned int, i, 0, quadCount)
		{
			DX9_RENDERSYS->d3dDevice->SetVertexShaderConstantB(i, quadVectors + i * 4, 1);
		}
	}

	void DirectX9_VertexShader::setConstantsI(const int* quadVectors, unsigned int quadCount)
	{
		for_itert (unsigned int, i, 0, quadCount)
		{
			DX9_RENDERSYS->d3dDevice->SetVertexShaderConstantI(i, quadVectors + i * 4, 1);
		}
	}

	void DirectX9_VertexShader::setConstantsF(const float* quadVectors, unsigned int quadCount)
	{
		for_itert (unsigned int, i, 0, quadCount)
		{
			DX9_RENDERSYS->d3dDevice->SetVertexShaderConstantF(i, quadVectors + i * 4, 1);
		}
	}

}

#endif
