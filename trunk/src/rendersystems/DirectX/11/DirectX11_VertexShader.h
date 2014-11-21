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
/// Defines a DirectX11 vertex shader.

#ifdef _DIRECTX11
#ifndef APRIL_DIRECTX11_VERTEX_SHADER_H
#define APRIL_DIRECTX11_VERTEX_SHADER_H

#include <hltypes/hplatform.h>
#include <hltypes/hstring.h>

#include "VertexShader.h"

using namespace Microsoft::WRL;

namespace april
{
	class DirectX11_RenderSystem;
	
	class DirectX11_VertexShader : public VertexShader
	{
	public:
		friend class DirectX11_RenderSystem;

		DirectX11_VertexShader(chstr filename);
		DirectX11_VertexShader();
		~DirectX11_VertexShader();

		bool loadFile(chstr filename);
		bool loadResource(chstr filename);
		void setConstantsB(const int* quads, unsigned int quadCount);
		void setConstantsI(const int* quads, unsigned int quadCount);
		void setConstantsF(const float* quads, unsigned int quadCount);

	protected:
		ComPtr<ID3D11VertexShader> dx11Shader;
		unsigned char* shaderData;
		long shaderSize;

	};

}
#endif
#endif
