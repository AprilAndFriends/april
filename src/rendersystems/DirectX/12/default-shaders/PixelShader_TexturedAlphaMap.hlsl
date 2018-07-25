/// @version 5.2

#include "PixelShader_Textured.hlsli"

float4 main(PixelShaderInput input) : SV_Target
{
	return float4(input.color.rgb, input.color.a * cTexture.Sample(cSampler, input.tex).r);
}
