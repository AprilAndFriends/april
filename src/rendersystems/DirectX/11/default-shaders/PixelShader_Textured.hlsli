/// @version 5.1

#include "PixelShader.hlsli"

struct PixelShaderInput
{
	float4 position : SV_Position;
	float4 color : COLOR;
	float2 tex : TEXCOORD0;
	float4 lerpAlpha : COLOR1; // only "a" is used
};

Texture2D<float4> cTexture : register(t0);
SamplerState cSampler : register(s0);
