/// @version 4.0

#include "PixelShader_Textured.hlsli"

min16float4 main(PixelShaderInput input) : SV_Target
{
	min16float4 tex = cTexture.Sample(cSampler, input.tex);
	return min16float4(input.color.rgb, input.color.a * tex.r);
}
