/// @file
/// @version 3.7
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Defines an OpenGLES pixel shader.

#ifdef _OPENGLES
#ifndef APRIL_OPENGLES_DEFAULT_SHADERS_H
#define APRIL_OPENGLES_DEFAULT_SHADERS_H

#define SHADER_Include "\
"

#define SHADER_VertexDefault SHADER_Include "\
	uniform mediump mat4 transformationMatrix; \n\
	//uniform lowp float lerp; \n\
	attribute highp vec4 position; \n\
	attribute lowp vec4 color; \n\
	attribute mediump vec4 tex; \n\
	varying mediump vec2 texFrag; \n\
	varying lowp vec4 colorFrag; \n\
	void main(void) \n\
	{ \n\
		gl_Position = transformationMatrix * position; \n\
		texFrag = tex.st; \n\
		colorFrag = color; \n\
	} \n\
"

#define SHADER_PixelTexturedMultiply SHADER_Include "\
	//uniform sampler2D sampler2d; \n\
	varying mediump vec2 texFrag; \n\
	varying lowp vec4 colorFrag; \n\
	void main(void) \n\
	{ \n\
		//gl_FragColor = texture2D(sampler2d, texFrag) * colorFrag; \n\
		gl_FragColor = colorFrag; \n\
	} \n\
"

#define SHADER_PixelTexturedAlphaMap SHADER_Include "\
	Texture2D<vec4> cTexture : register(t0); \n\
	SamplerState cSampler : register(s0); \n\
	vec4 main(PixelShaderInput in) : SV_Target \n\
	{ \n\
		vec4 tex = cTexture.Sample(cSampler, in.tex); \n\
		return vec4(in.color.rgb, in.color.a * tex.r); \n\
	} \n\
"

#define SHADER_PixelTexturedLerp SHADER_Include "\
	Texture2D<vec4> cTexture : register(t0); \n\
	SamplerState cSampler : register(s0); \n\
	vec4 main(PixelShaderInput in) : SV_Target \n\
	{ \n\
		vec4 tex = cTexture.Sample(cSampler, in.tex); \n\
		return vec4(lerp(tex.rgb, in.color.rgb, in.color.a), tex.a * in.lerpAlpha.a); \n\
	} \n\
"

#define SHADER_PixelMultiply SHADER_Include "\
	vec4 main(PixelShaderInput in) : SV_Target \n\
	{ \n\
		return in.color; \n\
	} \n\
"

#define SHADER_PixelAlphaMap SHADER_Include "\
	vec4 main(PixelShaderInput in) : SV_Target \n\
	{ \n\
		return in.color; \n\
	} \n\
"

#define SHADER_PixelLerp SHADER_Include "\
	vec4 main(PixelShaderInput in) : SV_Target \n\
	{ \n\
		return vec4(lerp(vec3((float)1.0, (float)1.0, (float)1.0), in.color.rgb, in.color.a), in.lerpAlpha.a); \n\
	} \n\
"

#endif
#endif
