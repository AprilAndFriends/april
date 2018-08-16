/// @version 5.2

#include "VertexShader.hlsli"
#include "PixelShader_Plain.hlsli"

struct VertexShaderInput
{
	float3 position : POSITION;
};

PixelShaderInput main(VertexShaderInput input)
{
	PixelShaderInput vertexShaderOutput;
	vertexShaderOutput.position = mul(float4(input.position, 1.0), cMatrix);
	vertexShaderOutput.color = cSystemColor;
	vertexShaderOutput.lerpAlpha = cLerpAlpha;
	return vertexShaderOutput;
}
