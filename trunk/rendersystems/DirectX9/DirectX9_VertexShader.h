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
/// Defines a DirectX vertex shader.

#ifdef _DIRECTX9
#ifndef APRIL_DIRECTX9_VERTEX_SHADER_H
#define APRIL_DIRECTX9_VERTEX_SHADER_H

#include "VertexShader.h"

struct IDirect3DVertexShader9;

namespace april
{
	class DirectX9_VertexShader : public VertexShader
	{
	public:
		IDirect3DVertexShader9* mShader;

		DirectX9_VertexShader();
		~DirectX9_VertexShader();

		bool compile(chstr shaderCode);
		void setConstantsB(const int* quadVectors, unsigned int quadCount);
		void setConstantsI(const int* quadVectors, unsigned int quadCount);
		void setConstantsF(const float* quadVectors, unsigned int quadCount);

	};

}
#endif
#endif
