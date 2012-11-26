cbuffer constantBuffer : register(b0)
{
	matrix cMatrix;
	float4 cColor;
};

struct PixelShaderInput
{
	float4 position : SV_POSITION;
};

float4 main(PixelShaderInput input) : SV_TARGET
{
	return cColor;
}
