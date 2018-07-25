/// @version 5.2

#include "PixelShader_Textured.hlsli"

float4 main(PixelShaderInput input) : SV_Target
{
	float4 tex = cTexture.Sample(cSampler, input.tex);
	return float4(MAKE_SEPIA_FLOAT3(tex.rgb) * input.color.rgb, tex.a * input.color.a);
}
