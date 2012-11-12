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

	}

}

#endif
