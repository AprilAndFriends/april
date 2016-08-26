/// @version 4.1

struct PixelShaderInput
{
	min16float4 position : SV_Position;
	min16float4 color : COLOR;
	float2 tex : TEXCOORD0;
	min16float4 lerpAlpha : COLOR1; // only "a" is used
};

Texture2D<min16float4> cTexture : register(t0);
SamplerState cSampler : register(s0);
