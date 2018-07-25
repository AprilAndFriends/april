/// @version 5.1

#include "PixelShader_Textured.hlsli"

float4 main(PixelShaderInput input) : SV_Target
{
	return (cTexture.Sample(cSampler, input.tex) * input.color);
}
