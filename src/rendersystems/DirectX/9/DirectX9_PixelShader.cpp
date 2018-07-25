/// @file
/// @version 5.2
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifdef _DIRECTX9
#include <d3d9.h>
#include <d3dx9shader.h>

#include <hltypes/hlog.h>
#include <hltypes/hstring.h>

#include "april.h"
#include "DirectX9_PixelShader.h"
#include "DirectX9_RenderSystem.h"

#define APRIL_D3D_DEVICE (((DirectX9_RenderSystem*)april::rendersys)->d3dDevice)

namespace april
{
	DirectX9_PixelShader::DirectX9_PixelShader() :
		PixelShader(),
		dx9Shader(NULL)
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

	bool DirectX9_PixelShader::isLoaded() const
	{
		return (this->dx9Shader != NULL);
	}

	bool DirectX9_PixelShader::_createShader(chstr filename, const hstream& stream)
	{
		HRESULT result = APRIL_D3D_DEVICE->CreatePixelShader((DWORD*)(unsigned char*)stream, &this->dx9Shader);
		if (result != D3D_OK)
		{
			hlog::error(logTag, "Failed to create pixel shader!");
			return false;
		}
		return true;
	}

	void DirectX9_PixelShader::setConstantsB(const int* quads, unsigned int quadCount)
	{
		for_itert (unsigned int, i, 0, quadCount)
		{
			APRIL_D3D_DEVICE->SetPixelShaderConstantB(i, quads + i * 4, 1);
		}
	}

	void DirectX9_PixelShader::setConstantsI(const int* quads, unsigned int quadCount)
	{
		for_itert (unsigned int, i, 0, quadCount)
		{
			APRIL_D3D_DEVICE->SetPixelShaderConstantI(i, quads + i * 4, 1);
		}
	}

	void DirectX9_PixelShader::setConstantsF(const float* quads, unsigned int quadCount)
	{
		for_itert (unsigned int, i, 0, quadCount)
		{
			APRIL_D3D_DEVICE->SetPixelShaderConstantF(i, quads + i * 4, 1);
		}
	}

}

#endif