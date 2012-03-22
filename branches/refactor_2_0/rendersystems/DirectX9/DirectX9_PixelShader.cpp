/// @file
/// @author  Boris Mikic
/// @version 2.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _DIRECTX9

#include <d3d9.h>
#include <d3dx9shader.h>

#include "april.h"
#include "DirectX9_PixelShader.h"
#include "DirectX9_RenderSystem.h"

#define DX9_RENDERSYS ((DirectX9_RenderSystem*)april::rendersys)

namespace april
{
	DirectX9_PixelShader::DirectX9_PixelShader() : PixelShader(), dx9Shader(NULL)
	{
	}

	DirectX9_PixelShader::~DirectX9_PixelShader()
	{
		if (this->dx9Shader != NULL)
		{
			this->dx9Shader->Release();
			this->dx9Shader = NULL;
		}
	}

	bool DirectX9_PixelShader::compile(chstr shaderCode)
	{
		if (shaderCode == "")
		{
			april::log("no pixel shader code given");
			return false;
		}
		DWORD flags = D3DXSHADER_OPTIMIZATION_LEVEL3;
#ifdef _DEBUG
		flags |= D3DXSHADER_DEBUG;
#endif
		LPD3DXBUFFER assembly;
		HRESULT result = D3DXCompileShader(shaderCode.c_str(), shaderCode.size(),
			NULL, NULL, "main", "ps_2_0", flags, &assembly, NULL, NULL);
		if (result != D3D_OK)
		{
			april::log("failed to compile pixel shader");
			return false;
		}
		result = DX9_RENDERSYS->d3dDevice->CreatePixelShader((DWORD*)assembly->GetBufferPointer(), &this->dx9Shader);
		if (result != D3D_OK)
		{
			april::log("failed to create pixel shader");
			return false;
		}
		return true;
	}

	void DirectX9_PixelShader::setConstantsB(const int* quadVectors, unsigned int quadCount)
	{
		for_itert (unsigned int, i, 0, quadCount)
		{
			DX9_RENDERSYS->d3dDevice->SetPixelShaderConstantB(i, quadVectors + i * 4, 1);
		}
	}

	void DirectX9_PixelShader::setConstantsI(const int* quadVectors, unsigned int quadCount)
	{
		for_itert (unsigned int, i, 0, quadCount)
		{
			DX9_RENDERSYS->d3dDevice->SetPixelShaderConstantI(i, quadVectors + i * 4, 1);
		}
	}

	void DirectX9_PixelShader::setConstantsF(const float* quadVectors, unsigned int quadCount)
	{
		for_itert (unsigned int, i, 0, quadCount)
		{
			DX9_RENDERSYS->d3dDevice->SetPixelShaderConstantF(i, quadVectors + i * 4, 1);
		}
	}

}

#endif
