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
/// Defines a DirectX pixel shader.

#ifdef _DIRECTX9
#ifndef APRIL_DIRECTX9_PIXEL_SHADER_H
#define APRIL_DIRECTX9_PIXEL_SHADER_H

#include "PixelShader.h"

struct IDirect3DPixelShader9;

namespace april
{
	class DirectX9_PixelShader : public PixelShader
	{
	public:
		IDirect3DPixelShader9* mShader;

		DirectX9_PixelShader();
		~DirectX9_PixelShader();

		bool compile(chstr shaderCode);

	};

}
#endif
#endif
