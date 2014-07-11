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

#include "VertexShader.h"

namespace april
{
	VertexShader::VertexShader(chstr filename)
	{
		this->load(filename);
	}

	VertexShader::VertexShader()
	{
	}

	VertexShader::~VertexShader()
	{
	}

	void VertexShader::load(chstr filename)
	{
		this->shaderCode = hresource::hread(filename);
		this->compile(this->shaderCode);
	}
	
	bool VertexShader::compile()
	{
		return this->compile(this->shaderCode);
	}
	
}
