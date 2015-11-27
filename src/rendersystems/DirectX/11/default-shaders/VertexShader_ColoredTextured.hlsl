/// @version 4.0

#include "VertexShader.hlsli"
#include "PixelShader_Textured.hlsli"

PixelShaderInput main(VertexShaderInput input)
{
	PixelShaderInput vertexShaderOutput;
	vertexShaderOutput.position = mul(min16float4(input.position, (min16float)1.0), cMatrix);
	vertexShaderOutput.color = input.color;
	vertexShaderOutput.tex = input.tex;
	vertexShaderOutput.lerpAlpha = cLerpAlpha;
	return vertexShaderOutput;
}
