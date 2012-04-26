/// @file
/// @author  Boris Mikic
/// @version 1.32
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
	VertexShader::VertexShader()
	{
	}

	VertexShader::~VertexShader()
	{
	}

	void VertexShader::load(chstr filename)
	{
		mShaderCode = hresource::hread(filename);
		compile();
	}
	
	bool VertexShader::compile()
	{
		return compile(mShaderCode);
	}
	
}
