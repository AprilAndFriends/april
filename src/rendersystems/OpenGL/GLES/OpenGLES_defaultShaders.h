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
	uniform lowp float lerpAlpha; \n\
	varying mediump vec2 texFrag; \n\
	varying lowp vec4 colorFrag; \n\
"

#define SHADER_VertexDefault SHADER_Include "\
	uniform mediump mat4 transformationMatrix; \n\
	attribute highp vec4 position; \n\
	attribute lowp vec4 color; \n\
	attribute mediump vec2 tex; \n\
	void main(void) \n\
	{ \n\
		gl_Position = transformationMatrix * position; \n\
		texFrag = tex; \n\
		colorFrag = color; \n\
	} \n\
"

#define SHADER_PixelTexturedMultiply SHADER_Include "\
	uniform sampler2D sampler2d; \n\
	void main(void) \n\
	{ \n\
		gl_FragColor = texture2D(sampler2d, texFrag) * colorFrag; \n\
	} \n\
"

#define SHADER_PixelTexturedAlphaMap SHADER_Include "\
	uniform sampler2D sampler2d; \n\
	void main(void) \n\
	{ \n\
		lowp vec4 tex = texture2D(sampler2d, texFrag); \n\
		gl_FragColor = vec4(colorFrag.rgb, colorFrag.a * tex.r); \n\
	} \n\
"

#define SHADER_PixelTexturedLerp SHADER_Include "\
	uniform sampler2D sampler2d; \n\
	void main(void) \n\
	{ \n\
		lowp vec4 tex = texture2D(sampler2d, texFrag); \n\
		gl_FragColor = vec4(mix(tex.rgb, colorFrag.rgb, colorFrag.a), tex.a * lerpAlpha); \n\
	} \n\
"

#define SHADER_PixelMultiply SHADER_Include "\
	uniform sampler2D sampler2d; \n\
	void main(void) \n\
	{ \n\
		gl_FragColor = colorFrag; \n\
	} \n\
"

#define SHADER_PixelAlphaMap SHADER_Include "\
	uniform sampler2D sampler2d; \n\
	void main(void) \n\
	{ \n\
		gl_FragColor = colorFrag; \n\
	} \n\
"

#define SHADER_PixelLerp SHADER_Include "\
	uniform sampler2D sampler2d; \n\
	void main(void) \n\
	{ \n\
		gl_FragColor = vec4(mix(vec3(1.0, 1.0, 1.0), colorFrag.rgb, colorFrag.a), lerpAlpha); \n\
	} \n\
"

#endif
#endif
