Texture2D cTexture : register(t0);
SamplerState cSampler : register(s0);

struct PixelShaderInput
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
	float2 tex : TEXCOORD0;
};

float4 main(PixelShaderInput input) : SV_TARGET
{
    return (input.color * cTexture.Sample(cSampler, input.tex));
}
