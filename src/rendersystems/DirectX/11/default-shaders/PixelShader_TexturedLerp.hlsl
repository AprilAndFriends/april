/// @version 4.0

#include "Shader_Textured.hlsli"

min16float4 main(PixelShaderInput input) : SV_Target
{
	min16float4 tex = cTexture.Sample(cSampler, input.tex);
	return min16float4(lerp(tex.rgb, input.color.rgb, input.lerpAlpha.a), tex.a * input.color.a);
}
