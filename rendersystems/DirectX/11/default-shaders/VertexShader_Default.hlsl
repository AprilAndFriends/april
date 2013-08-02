// version 3.0

cbuffer constantBuffer : register(b0)
{
	matrix cMatrix;
	float4 cColor;
	float4 cColorModeData;
};

struct VertexShaderInput
{
	float3 position : POSITION;
	float4 color : COLOR;
	float2 tex : TEXCOORD0;
};

struct PixelShaderInput
{
	float4 position : SV_Position;
	float4 color : COLOR;
	float2 tex : TEXCOORD0;
	float4 colorModeData : COLOR1; // r is texture usage, g is color usage mode, b is color-mode, a is color-mode-alpha (used in LERP)
};

PixelShaderInput main(VertexShaderInput input)
{
	PixelShaderInput vertexShaderOutput;
	float4 position = float4(input.position, 1.0f);
	position = mul(position, cMatrix);
	vertexShaderOutput.position = position;
	if (cColorModeData.g > 1.999 && cColorModeData.g < 2.001) // PER_VERTEX color input layout
	{
		vertexShaderOutput.color = input.color;
	}
	else
	{
		vertexShaderOutput.color = cColor;
	}
	vertexShaderOutput.tex = input.tex;
	vertexShaderOutput.colorModeData = cColorModeData;
	return vertexShaderOutput;
}
