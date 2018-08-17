/// @version 5.2

#include "PixelShader.hlsli"

struct PixelShaderInput
{
	float4 position : SV_Position;
	float4 color : COLOR;
	float4 lerpAlpha : COLOR1; // only "a" is used, float4 due to 16-byte alignment
};
