/// @version 5.1

#include "PixelShader_Plain.hlsli"

float4 main(PixelShaderInput input) : SV_Target
{
	static const float3 white = float3(1.0, 1.0, 1.0);
	return float4(lerp(white, input.color.rgb, input.lerpAlpha.a), input.color.a);
}
