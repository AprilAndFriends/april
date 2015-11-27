/// @version 4.0

cbuffer constantBuffer : register(b0)
{
	min16float4x4 cMatrix;
	min16float4 cLerpAlpha;
};

struct VertexShaderInput
{
	min16float3 position : POSITION;
	min16float4 color : COLOR;
	float2 tex : TEXCOORD0;
};

