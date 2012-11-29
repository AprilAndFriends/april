cbuffer constantBuffer : register(b0)
{
	matrix cMatrix;
	float4 cColor;
	float4 cColorModeData;
};

struct VertexShaderInput
{
	float3 position : POSITION;
};

struct PixelShaderInput
{
	float4 position : SV_Position;
	float4 color : COLOR;
	float4 colorModeData : COLOR1; // r is color-mode, g is color-mode-alpha (used in LERP), b and a are not used
};

PixelShaderInput main(VertexShaderInput input)
{
	PixelShaderInput vertexShaderOutput;
	float4 position = float4(input.position, 1.0f);
	position = mul(position, cMatrix);
	vertexShaderOutput.position = position;
	vertexShaderOutput.color = cColor;
	vertexShaderOutput.colorModeData = cColorModeData;
	return vertexShaderOutput;
}
