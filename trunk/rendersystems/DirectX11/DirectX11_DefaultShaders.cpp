/// @file
/// @author  Boris Mikic
/// @version 2.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _DIRECTX11
#include <hltypes/hstring.h>

#include "DirectX11_DefaultShaders.h"

namespace april
{
	namespace DirectX11
	{
		/*
		hstr DefaultVertexShader = "\
		void VS(in float4 posIn : POSITION, out float4 posOut : SV_Position)\n \
		{\n \
			posOut = posIn;\n \
		}\n \
		";
	
		hstr DefaultPixelShader = "\
		void PS(out float4 colorOut : SV_Target)\n \
		{\n \
			colorOut = float4(1.0f, 1.0f, 1.0f, 1.0f);\n \
		}\n \
		";
		*/
		
		hstr _constantBuffer = "\
		cbuffer constantBuffer : register(b0) \
		{ \
			matrix mat; \
		}; \
		";

		hstr _vertexInput = "\
		struct VertexShaderInput \
		{ \
			float3 pos : POSITION; \
			/*float4 color : COLOR;*/ \
		}; \
		";

		hstr _pixelInput = "\
		struct PixelShaderInput \
		{ \
			float4 pos : SV_POSITION; \
			/*float4 color : COLOR;*/ \
		}; \
		";

		hstr DefaultVertexShader = 
			_constantBuffer +
			_vertexInput +
			_pixelInput +
			"\
		PixelShaderInput VS(VertexShaderInput input) \
		{ \
			PixelShaderInput vertexShaderOutput; \
			float4 pos = float4(input.pos, 1.0f); \
			pos = mul(pos, mat); \
			vertexShaderOutput.pos = pos; \
			/*vertexShaderOutput.color = input.color;*/ \
			return vertexShaderOutput; \
		} \
		";
	
		hstr DefaultPixelShader =
			_pixelInput +
			"\
		float4 PS(PixelShaderInput input) : SV_TARGET \
		{ \
			return float4(1.0f, 1.0f, 1.0f, 1.0f);/* input.color;*/ \
		} \
		";

	}

}

#endif
