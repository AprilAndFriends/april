/// @file
/// @version 4.4
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Defines a DirectX12 vertex shader.

#ifdef _DIRECTX12
#ifndef APRIL_DirectX12_VERTEX_SHADER_H
#define APRIL_DirectX12_VERTEX_SHADER_H

#include <hltypes/hplatform.h>
#include <hltypes/hstring.h>

#include "VertexShader.h"

using namespace Microsoft::WRL;

namespace april
{
	class DirectX12_RenderSystem;
	
	class DirectX12_VertexShader : public VertexShader
	{
	public:
		friend class DirectX12_RenderSystem;

		DirectX12_VertexShader();
		~DirectX12_VertexShader();
		
		bool isLoaded() const;

		void setConstantsB(const int* quads, unsigned int quadCount);
		void setConstantsI(const int* quads, unsigned int quadCount);
		void setConstantsF(const float* quads, unsigned int quadCount);

	protected:
		hstream shaderData;

		bool _createShader(chstr filename, const hstream& stream);
		
	};

}
#endif
#endif
