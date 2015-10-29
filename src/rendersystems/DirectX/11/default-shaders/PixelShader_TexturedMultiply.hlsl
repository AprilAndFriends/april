/// @version 4.0

#include "Shader_Textured.hlsli"

min16float4 main(PixelShaderInput input) : SV_Target
{
	return (cTexture.Sample(cSampler, input.tex) * input.color);
}
