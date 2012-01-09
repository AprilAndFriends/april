/// @file
/// @author  Boris Mikic
/// @version 1.32
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/// 
/// @section DESCRIPTION
/// 
/// Defines a generic pixel shader.

#ifndef APRIL_PIXEL_SHADER_H
#define APRIL_PIXEL_SHADER_H

#include <hltypes/hstring.h>

#include "aprilExport.h"

namespace april
{
	class aprilExport PixelShader
	{
	public:
		PixelShader();
		virtual ~PixelShader();

		void load(chstr filename);
		bool compile();
		virtual bool compile(chstr shaderCode) = 0;
		virtual void setConstantsB(const int* quadVectors, unsigned int quadCount) = 0;
		virtual void setConstantsI(const int* quadVectors, unsigned int quadCount) = 0;
		virtual void setConstantsF(const float* quadVectors, unsigned int quadCount) = 0;

	protected:
		hstr mShaderCode;

	};

}

#endif
