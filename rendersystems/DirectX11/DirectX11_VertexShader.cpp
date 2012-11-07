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
#include "DirectX11_RenderSystem.h"
#include "DirectX11_VertexShader.h"

namespace april
{
	DirectX11_VertexShader::DirectX11_VertexShader(chstr filename) : VertexShader(filename)
	{
	}

	DirectX11_VertexShader::DirectX11_VertexShader() : VertexShader()
	{
	}

	DirectX11_VertexShader::~DirectX11_VertexShader()
	{
	}

	bool DirectX11_VertexShader::compile(chstr shaderCode)
	{
		return false;
	}

	void DirectX11_VertexShader::setConstantsB(const int* quadVectors, unsigned int quadCount)
	{
	}

	void DirectX11_VertexShader::setConstantsI(const int* quadVectors, unsigned int quadCount)
	{
	}

	void DirectX11_VertexShader::setConstantsF(const float* quadVectors, unsigned int quadCount)
	{
	}

}

#endif