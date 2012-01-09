/// @file
/// @author  Boris Mikic
/// @version 1.32
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _DIRECTX9

#include <d3d9.h>
#include <d3dx9shader.h>

#include "DirectX9_PixelShader.h"
#include "DirectX9_RenderSystem.h"

namespace april
{
	extern IDirect3DDevice9* d3dDevice;

	DirectX9_PixelShader::DirectX9_PixelShader() : PixelShader(), mShader(NULL)
	{
	}

	DirectX9_PixelShader::~DirectX9_PixelShader()
	{
		if (mShader != NULL)
		{
			mShader->Release();
			mShader = NULL;
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
		result = d3dDevice->CreatePixelShader((DWORD*)assembly->GetBufferPointer(), &mShader);
		if (result != D3D_OK)
		{
			april::log("failed to create pixel shader");
			return false;
		}
		return true;
	}

}

#endif
