/// @version 4.1

#include "PixelShader_Plain.hlsli"

min16float4 main(PixelShaderInput input) : SV_Target
{
	return input.color;
}
