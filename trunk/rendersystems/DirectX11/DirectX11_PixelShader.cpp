/// @file
/// @author  Boris Mikic
/// @version 2.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _DIRECTX11
#include <hltypes/hplatform.h>

#include <hltypes/hlog.h>
#include <hltypes/hstring.h>

#include "april.h"
#include "DirectX11_PixelShader.h"
#include "DirectX11_RenderSystem.h"

namespace april
{
	DirectX11_PixelShader::DirectX11_PixelShader(chstr filename) : PixelShader(filename)
	{
	}

	DirectX11_PixelShader::DirectX11_PixelShader() : PixelShader()
	{
	}

	DirectX11_PixelShader::~DirectX11_PixelShader()
	{
	}

	bool DirectX11_PixelShader::compile(chstr shaderCode)
	{
		return false;
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