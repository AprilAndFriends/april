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
/// Defines a DirectX9 pixel shader.

#ifdef _DIRECTX9
#ifndef APRIL_DIRECTX9_PIXEL_SHADER_H
#define APRIL_DIRECTX9_PIXEL_SHADER_H

#include <hltypes/hstring.h>

#include "PixelShader.h"

struct IDirect3DPixelShader9;

namespace april
{
	class DirectX9_RenderSystem;
	
	class DirectX9_PixelShader : public PixelShader
	{
	public:
		friend class DirectX9_RenderSystem;

		DirectX9_PixelShader(chstr filename);
		DirectX9_PixelShader();
		~DirectX9_PixelShader();

		bool load(chstr filename);
		void setConstantsB(const int* quadVectors, unsigned int quadCount);
		void setConstantsI(const int* quadVectors, unsigned int quadCount);
		void setConstantsF(const float* quadVectors, unsigned int quadCount);

	protected:
		IDirect3DPixelShader9* dx9Shader;

	};

}
#endif
#endif