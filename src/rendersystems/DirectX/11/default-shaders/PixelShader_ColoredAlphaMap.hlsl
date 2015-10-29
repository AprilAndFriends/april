/// @version 4.0

#include "Shader_Colored.hlsli"

min16float4 main(PixelShaderInput input) : SV_Target
{
	return input.color;
}
