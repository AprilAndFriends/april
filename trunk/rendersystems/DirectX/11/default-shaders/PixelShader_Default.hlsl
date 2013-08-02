// version 3.0

Texture2D cTexture : register(t0);
SamplerState cSampler : register(s0);

struct PixelShaderInput
{
	float4 position : SV_Position;
	float4 color : COLOR;
	float2 tex : TEXCOORD0;
	float4 colorModeData : COLOR1; // r is texture usage, g is color usage mode, b is color-mode, a is color-mode-alpha (used in LERP)
};

float4 main(PixelShaderInput input) : SV_Target
{
	float4 result = {1.0f, 1.0f, 1.0f, 1.0f};
	if (input.colorModeData.r > 0.999 && input.colorModeData.r < 1.001) // using texture
	{
		result = cTexture.Sample(cSampler, input.tex);
	}
	if (input.colorModeData.b > 1.999 && input.colorModeData.b < 2.001) // LERP
	{
		result = float4(lerp(result.rgb, input.color.rgb, input.color.a), result.a * input.colorModeData.a);
	}
	else if (input.colorModeData.b > 2.999 && input.colorModeData.b < 3.001) // ALPHA_MAP
	{
		result = float4(input.color.rgb, input.color.a * result.r);
	}
	else if (input.colorModeData.g > 0.999 && input.colorModeData.g < 2.001) // PER_VERTEX or MULTIPLY color input layout
	{
		result *= input.color;
	}
	return result;
}
