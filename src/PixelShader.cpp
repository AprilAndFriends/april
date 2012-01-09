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

#include "PixelShader.h"

namespace april
{
	PixelShader::PixelShader()
	{
	}

	PixelShader::~PixelShader()
	{
	}
	
	void PixelShader::load(chstr filename)
	{
		mShaderCode = hfile::hread(filename);
		compile();
	}
	
	bool PixelShader::compile()
	{
		return compile(mShaderCode);
	}
	
}
