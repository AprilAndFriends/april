cbuffer constantBuffer : register(b0)
{
	matrix cMatrix;
	float4 cColor;
};

struct VertexShaderInput
{
	float3 position : POSITION;
	float4 color : COLOR;
	float2 tex : TEXCOORD0;
};

struct PixelShaderInput
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
	float2 tex : TEXCOORD0;
};

PixelShaderInput main(VertexShaderInput input)
{
	PixelShaderInput vertexShaderOutput;
	float4 position = float4(input.position, 1.0f);
	position = mul(position, cMatrix);
	vertexShaderOutput.position = position;
	vertexShaderOutput.color = input.color;
	vertexShaderOutput.tex = input.tex;
	return vertexShaderOutput;
}
