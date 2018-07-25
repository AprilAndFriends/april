/// @version 5.1

#include "PixelShader_Plain.hlsli"

float4 main(PixelShaderInput input) : SV_Target
{
	return float4(MAKE_SEPIA(input.color), input.color.a);
}
