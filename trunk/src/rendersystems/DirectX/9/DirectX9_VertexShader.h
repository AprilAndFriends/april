/// @file
/// @version 3.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Defines a DirectX9 vertex shader.

#ifdef _DIRECTX9
#ifndef APRIL_DIRECTX9_VERTEX_SHADER_H
#define APRIL_DIRECTX9_VERTEX_SHADER_H

#include <hltypes/hstring.h>

#include "VertexShader.h"

struct IDirect3DVertexShader9;

namespace april
{
	class DirectX9_RenderSystem;
	
	class DirectX9_VertexShader : public VertexShader
	{
	public:
		friend class DirectX9_RenderSystem;

		DirectX9_VertexShader(chstr filename);
		DirectX9_VertexShader();
		~DirectX9_VertexShader();

		bool load(chstr filename);
		void setConstantsB(const int* quadVectors, unsigned int quadCount);
		void setConstantsI(const int* quadVectors, unsigned int quadCount);
		void setConstantsF(const float* quadVectors, unsigned int quadCount);

	protected:
		IDirect3DVertexShader9* dx9Shader;

	};

}
#endif
#endif
