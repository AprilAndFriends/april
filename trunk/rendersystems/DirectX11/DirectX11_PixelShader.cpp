/// @file
/// @author  Boris Mikic
/// @version 2.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _DIRECTX11
#include <d3d11.h>
#include <d3dcompiler.h>

#include <hltypes/hlog.h>
#include <hltypes/hstring.h>

#include "april.h"
#include "DirectX11_PixelShader.h"
#include "DirectX11_RenderSystem.h"

#define APRIL_D3D_DEVICE (((DirectX11_RenderSystem*)april::rendersys)->d3dDevice)

namespace april
{
	DirectX11_PixelShader::DirectX11_PixelShader(chstr filename) : PixelShader(filename), dx11Shader(NULL)
	{
	}

	DirectX11_PixelShader::DirectX11_PixelShader() : PixelShader(), dx11Shader(NULL)
	{
	}

	DirectX11_PixelShader::~DirectX11_PixelShader()
	{
		if (this->dx11Shader != NULL)
		{
			this->dx11Shader->Release();
			this->dx11Shader = NULL;
		}
	}

	bool DirectX11_PixelShader::compile(chstr shaderCode)
	{
		if (shaderCode == "")
		{
			hlog::error(april::logTag, "No pixel shader code given!");
			return false;
		}
		DWORD shaderFlags = (D3DCOMPILE_OPTIMIZATION_LEVEL3 | D3DCOMPILE_ENABLE_STRICTNESS);
#ifdef _DEBUG
		shaderFlags |= D3DCOMPILE_DEBUG;
#endif
		ID3D10Blob* bufferShader = NULL;
		ID3D10Blob* bufferError = NULL;
		HRESULT hr = D3DCompile(shaderCode.c_str(), shaderCode.size() + 1, "PS", NULL, NULL,
			"PS", "ps_2_0", shaderFlags, 0, &bufferShader, &bufferError);
		if (FAILED(hr))
		{
			hlog::error(april::logTag, "Failed to compile pixel shader!");
			if (bufferError != NULL)
			{
				hlog::error(april::logTag, (char*)bufferError->GetBufferPointer());
				bufferError->Release();
			}
			return false;
		}
		hr = APRIL_D3D_DEVICE->CreatePixelShader(bufferShader->GetBufferPointer(),
			bufferShader->GetBufferSize(), NULL, &this->dx11Shader);
		if (FAILED(hr))
		{
			hlog::error(april::logTag, "Failed to create pixel shader!");
			return false;
		}
		return true;
	}

	void DirectX11_PixelShader::setConstantsB(const int* quadVectors, unsigned int quadCount)
	{
	}

	void DirectX11_PixelShader::setConstantsI(const int* quadVectors, unsigned int quadCount)
	{
	}

	void DirectX11_PixelShader::setConstantsF(const float* quadVectors, unsigned int quadCount)
	{
	}

}

#endif