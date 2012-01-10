/// @file
/// @author  Boris Mikic
/// @version 1.32
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <hltypes/hstring.h>
#include <hltypes/hfile.h>

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
		mShaderCode = hfile::hread(filename);
		compile();
	}
	
	bool VertexShader::compile()
	{
		return compile(mShaderCode);
	}
	
}
