// version 3.0

cbuffer constantBuffer : register(b0)
{
	min16float4x4 cMatrix;
	min16float4 cColor;
	min16float4 cColorModeData;
};

struct VertexShaderInput
{
	min16float3 position : POSITION;
	min16float4 color : COLOR;
	float2 tex : TEXCOORD0;
};

struct PixelShaderInput
{
	min16float4 position : SV_Position;
	min16float4 color : COLOR;
	float2 tex : TEXCOORD0;
	min16float4 colorModeData : COLOR1; // r is texture usage, g is color usage mode, b is color-mode, a is color-mode-alpha (used in LERP)
};

PixelShaderInput main(VertexShaderInput input)
{
	PixelShaderInput vertexShaderOutput;
	min16float4 position = min16float4(input.position, 1.0f);
	vertexShaderOutput.position = mul(position, cMatrix);
	if (cColorModeData.g > (min16float)1.99 && cColorModeData.g < (min16float)2.01) // PER_VERTEX color input layout
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
