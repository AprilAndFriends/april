/// @file
/// @version 3.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Defines a generic vertex shader.

#ifndef APRIL_VERTEX_SHADER_H
#define APRIL_VERTEX_SHADER_H

#include <hltypes/hstring.h>

#include "aprilExport.h"

namespace april
{
	class aprilExport VertexShader
	{
	public:
		VertexShader();
		virtual ~VertexShader();

		virtual bool load(chstr filename) = 0;
		virtual void setConstantsB(const int* quadVectors, unsigned int quadCount) = 0;
		virtual void setConstantsI(const int* quadVectors, unsigned int quadCount) = 0;
		virtual void setConstantsF(const float* quadVectors, unsigned int quadCount) = 0;

	protected:
		bool _loadData(chstr filename, unsigned char** data, long* size);

	};

}

#endif
