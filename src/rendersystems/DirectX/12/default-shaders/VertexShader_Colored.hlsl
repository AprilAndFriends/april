/// @version 4.1

#include "VertexShader.hlsli"
#include "PixelShader_Plain.hlsli"

struct VertexShaderInput
{
	min16float3 position : POSITION;
	min16float4 color : COLOR;
};

PixelShaderInput main(VertexShaderInput input)
{
	PixelShaderInput vertexShaderOutput;
	vertexShaderOutput.position = mul(min16float4(input.position, (min16float)1.0), cMatrix);
	vertexShaderOutput.color = input.color;
	vertexShaderOutput.lerpAlpha = cLerpAlpha;
	return vertexShaderOutput;
}