/// @version 5.1

cbuffer constantBuffer : register(b0)
{
	float4x4 cMatrix;
	float4 cSystemColor;
	float4 cLerpAlpha;
};

