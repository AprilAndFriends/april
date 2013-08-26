// version 3.0

Texture2D<min16float4> cTexture : register(t0);
SamplerState cSampler : register(s0);

struct PixelShaderInput
{
	min16float4 position : SV_Position;
	min16float4 color : COLOR;
	float2 tex : TEXCOORD0;
	min16float4 colorModeData : COLOR1; // r is texture usage, g is color usage mode, b is color-mode, a is color-mode-alpha (used in LERP)
};

min16float4 main(PixelShaderInput input) : SV_Target
{
	min16float4 result = {(min16float)1.0, (min16float)1.0, (min16float)1.0, (min16float)1.0};
	if (input.colorModeData.r > (min16float)0.99 && input.colorModeData.r < (min16float)1.01) // using texture
	{
		result = cTexture.Sample(cSampler, input.tex);
	}
	if (input.colorModeData.b > (min16float)1.99 && input.colorModeData.b < (min16float)2.01) // LERP
	{
		result = min16float4(lerp(result.rgb, input.color.rgb, input.color.a), result.a * input.colorModeData.a);
	}
	else if (input.colorModeData.b > (min16float)2.99 && input.colorModeData.b < (min16float)3.01) // ALPHA_MAP
	{
		result = min16float4(input.color.rgb, input.color.a * result.r);
	}
	else if (input.colorModeData.g > (min16float)0.99 && input.colorModeData.g < (min16float)2.01) // PER_VERTEX or MULTIPLY color input layout
	{
		result *= input.color;
	}
	return result;
}
