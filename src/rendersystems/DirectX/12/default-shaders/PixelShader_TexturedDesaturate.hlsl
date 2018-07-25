/// @version 5.2

#include "PixelShader_Textured.hlsli"

float4 main(PixelShaderInput input) : SV_Target
{
	float4 tex = cTexture.Sample(cSampler, input.tex);
	float value = MAKE_DESATURATE(tex);
	return float4(value * input.color.r, value * input.color.g, value * input.color.b, tex.a * input.color.a);
}
