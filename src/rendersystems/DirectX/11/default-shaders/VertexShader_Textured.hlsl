/// @version 4.0

#include "VertexShader.hlsli"
#include "Shader_Textured.hlsli"

struct VertexShaderInput
{
	min16float3 position : POSITION;
	float2 tex : TEXCOORD0;
};

PixelShaderInput main(VertexShaderInput input)
{
	PixelShaderInput vertexShaderOutput;
	vertexShaderOutput.position = mul(min16float4(input.position, (min16float)1.0), cMatrix);
	vertexShaderOutput.color = cSystemColor;
	vertexShaderOutput.tex = input.tex;
	vertexShaderOutput.lerpAlpha = cLerpAlpha;
	return vertexShaderOutput;
}
