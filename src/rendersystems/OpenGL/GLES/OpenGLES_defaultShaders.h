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
	/*struct PixelShaderInput \n\
	{ \n\
		attribute highp vec4 position; \n\
		lowp vec4 color; \n\
		mediump vec2 tex; \n\
		lowp vec4 lerpAlpha; \n\
	};*/ \n\
"

#define SHADER_VertexDefault SHADER_Include "\
	/*cbuffer constantBuffer : register(b0) \n\
	{ \n\
		mat4 cMatrix; \n\
		vec4 cLerpAlpha; \n\
	}; \n\
	struct VertexShaderInput \n\
	{ \n\
		vec3 position; \n\
		vec4 color; \n\
		vec2 tex; \n\
	};*/ \n\
	attribute highp vec4 position; \n\
	uniform mediump mat4 transformationMatrix; \n\
	/*PixelShaderInput*/ void main(void/*VertexShaderInput in*/) \n\
	{ \n\
		gl_Position = transformationMatrix * position; \n\
		//PixelShaderInput vertexShaderOutput; \n\
		//vertexShaderOutput.position =  mul(vec4(in.position, (float)1.0), cMatrix); \n\
		//vertexShaderOutput.color = in.color; \n\
		//vertexShaderOutput.tex = in.tex; \n\
		//vertexShaderOutput.lerpAlpha = cLerpAlpha; \n\
		//return vertexShaderOutput; \n\
	} \n\
"

#define SHADER_PixelTexturedMultiply SHADER_Include "\
	Texture2D<vec4> cTexture : register(t0); \n\
	SamplerState cSampler : register(s0); \n\
	vec4 main(PixelShaderInput in) : SV_Target \n\
	{ \n\
		return (cTexture.Sample(cSampler, in.tex) * in.color); \n\
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
