/// @version 4.0

#include "Shader_ColoredTextured.hlsli"

min16float4 main(PixelShaderInput input) : SV_Target
{
	return (cTexture.Sample(cSampler, input.tex) * input.color);
}
