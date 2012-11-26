cbuffer constantBuffer : register(b0)
{
	matrix cMatrix;
	float4 cColor;
};

Texture2D cTexture : register(t0);
SamplerState cSampler : register(s0);

struct PixelShaderInput
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
};

float4 main(PixelShaderInput input) : SV_TARGET
{
    return (cColor * cTexture.Sample(cSampler, input.tex));
}
