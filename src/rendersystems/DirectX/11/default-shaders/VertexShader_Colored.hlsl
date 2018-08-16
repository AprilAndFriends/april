/// @version 5.2

#include "VertexShader.hlsli"
#include "PixelShader_Plain.hlsli"

struct VertexShaderInput
{
	float3 position : POSITION;
	float4 color : COLOR;
};

PixelShaderInput main(VertexShaderInput input)
{
	PixelShaderInput vertexShaderOutput;
	vertexShaderOutput.position = mul(float4(input.position, 1.0), cMatrix);
	vertexShaderOutput.color = input.color;
	vertexShaderOutput.lerpAlpha = cLerpAlpha;
	return vertexShaderOutput;
}
