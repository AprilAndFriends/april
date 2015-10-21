/// @version 3.7

#include "Shaders.hlsli"

min16float4 main(PixelShaderInput input) : SV_Target
{
	return input.color;
}
