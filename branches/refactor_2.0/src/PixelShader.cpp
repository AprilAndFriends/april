/// @file
/// @author  Boris Mikic
/// @version 2.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <hltypes/hresource.h>
#include <hltypes/hstring.h>

#include "PixelShader.h"

namespace april
{
	PixelShader::PixelShader(chstr filename)
	{
		this->load(filename);
	}

	PixelShader::PixelShader()
	{
	}

	PixelShader::~PixelShader()
	{
	}
	
	void PixelShader::load(chstr filename)
	{
		this->shaderCode = hresource::hread(filename);
		this->compile(this->shaderCode);
	}
	
	bool PixelShader::compile()
	{
		return this->compile(shaderCode);
	}
	
}
