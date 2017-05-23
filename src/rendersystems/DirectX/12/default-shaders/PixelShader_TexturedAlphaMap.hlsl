/// @version 4.1

#include "PixelShader_Textured.hlsli"

min16float4 main(PixelShaderInput input) : SV_Target
{
	return min16float4(input.color.rgb, input.color.a * cTexture.Sample(cSampler, input.tex).r);
}
