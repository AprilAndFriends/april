/// @version 5.2

#include "VertexShader.hlsli"
#include "PixelShader_Textured.hlsli"

struct VertexShaderInput
{
	float3 position : POSITION;
	float4 color : COLOR;
	float2 tex : TEXCOORD0;
};

PixelShaderInput main(VertexShaderInput input)
{
	PixelShaderInput vertexShaderOutput;
	vertexShaderOutput.position = mul(float4(input.position, 1.0), cMatrix);
	vertexShaderOutput.color = input.color;
	vertexShaderOutput.tex = input.tex;
	vertexShaderOutput.lerpAlpha = cLerpAlpha;
	return vertexShaderOutput;
}
