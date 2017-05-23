/// @version 4.1

struct PixelShaderInput
{
	min16float4 position : SV_Position;
	min16float4 color : COLOR;
	min16float4 lerpAlpha : COLOR1; // only "a" is used, float4 due to 16-byte alignment
};
