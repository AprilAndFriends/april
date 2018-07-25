/// @version 5.1

#include "PixelShader_Plain.hlsli"

float4 main(PixelShaderInput input) : SV_Target
{
	float value = MAKE_DESATURATE(input.color);
	return float4(value, value, value, input.color.a);
}
