/// @file
/// @version 4.3
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

// general header defines
#define SHADER_Plain_Include "\
"
#define SHADER_Textured_Include "\
	varying mediump vec2 texFrag; \n\
"
#define SHADER_Colored_Include "\
	varying lowp vec4 colorFrag; \n\
"
#define SHADER_ColoredTextured_Include "\
" SHADER_Colored_Include "\
" SHADER_Textured_Include "\
"

// vertex header defines
#define SHADER_VERTEX_Include "\
	uniform mediump mat4 transformationMatrix; \n\
	attribute highp vec4 position; \n\
"
#define SHADER_VERTEX_Plain_Include "\
" SHADER_VERTEX_Include "\
	uniform lowp vec4 systemColor; \n\
"
#define SHADER_VERTEX_Textured_Include "\
" SHADER_VERTEX_Include "\
" SHADER_Textured_Include "\
	uniform lowp vec4 systemColor; \n\
	attribute mediump vec2 tex; \n\
"
#define SHADER_VERTEX_Colored_Include "\
" SHADER_VERTEX_Include "\
" SHADER_Colored_Include "\
	attribute lowp vec4 color; \n\
"
#define SHADER_VERTEX_ColoredTextured_Include "\
" SHADER_VERTEX_Include "\
" SHADER_ColoredTextured_Include "\
	attribute lowp vec4 color; \n\
	attribute mediump vec2 tex; \n\
"

// pixel header defines
#define SHADER_PIXEL_Plain_Include "\
	uniform lowp vec4 systemColor; \n\
"
#define SHADER_PIXEL_Textured_Include "\
" SHADER_Textured_Include "\
	uniform lowp vec4 systemColor; \n\
	uniform sampler2D sampler2d; \n\
"
#define SHADER_PIXEL_Colored_Include "\
" SHADER_Colored_Include "\
"
#define SHADER_PIXEL_ColoredTextured_Include "\
" SHADER_ColoredTextured_Include "\
	uniform sampler2D sampler2d; \n\
"

// pixel header defines (ALPHA-HACK)
#define SHADER_PIXEL_Textured_AlphaHack_Include "\
" SHADER_Textured_Include "\
	uniform lowp vec4 systemColor; \n\
	uniform sampler2D sampler2d; \n\
	uniform sampler2D sampler2dAlpha; \n\
"
#define SHADER_PIXEL_ColoredTextured_AlphaHack_Include "\
" SHADER_ColoredTextured_Include "\
	uniform sampler2D sampler2d; \n\
	uniform sampler2D sampler2dAlpha; \n\
"

// vertex shaders
#define SHADER_VertexPlain SHADER_VERTEX_Plain_Include "\
	void main(void) \n\
	{ \n\
		gl_Position = transformationMatrix * position; \n\
	} \n\
"
#define SHADER_VertexTextured SHADER_VERTEX_Textured_Include "\
	void main(void) \n\
	{ \n\
		gl_Position = transformationMatrix * position; \n\
		texFrag = tex; \n\
	} \n\
"
#define SHADER_VertexColored SHADER_VERTEX_Colored_Include "\
	void main(void) \n\
	{ \n\
		gl_Position = transformationMatrix * position; \n\
		colorFrag = color; \n\
	} \n\
"
#define SHADER_VertexColoredTextured SHADER_VERTEX_ColoredTextured_Include "\
	void main(void) \n\
	{ \n\
		gl_Position = transformationMatrix * position; \n\
		texFrag = tex; \n\
		colorFrag = color; \n\
	} \n\
"

// pixel shaders
#define SHADER_PixelMultiply SHADER_PIXEL_Plain_Include "\
	void main(void) \n\
	{ \n\
		gl_FragColor = systemColor; \n\
	} \n\
"
#define SHADER_PixelAlphaMap SHADER_PIXEL_Plain_Include "\
	void main(void) \n\
	{ \n\
		gl_FragColor = systemColor; \n\
	} \n\
"
#define SHADER_PixelLerp SHADER_PIXEL_Plain_Include "\
	uniform lowp float lerpAlpha; \n\
	void main(void) \n\
	{ \n\
		gl_FragColor = vec4(mix(vec3(1.0, 1.0, 1.0), systemColor.rgb, lerpAlpha), systemColor.a); \n\
	} \n\
"

#define SHADER_PixelTexturedMultiply SHADER_PIXEL_Textured_Include "\
	void main(void) \n\
	{ \n\
		gl_FragColor = texture2D(sampler2d, texFrag) * systemColor; \n\
	} \n\
"
#define SHADER_PixelTexturedAlphaMap SHADER_PIXEL_Textured_Include "\
	void main(void) \n\
	{ \n\
		lowp vec4 tex = texture2D(sampler2d, texFrag); \n\
		gl_FragColor = vec4(systemColor.rgb, systemColor.a * tex.r); \n\
	} \n\
"
#define SHADER_PixelTexturedLerp SHADER_PIXEL_Textured_Include "\
	uniform lowp float lerpAlpha; \n\
	void main(void) \n\
	{ \n\
		lowp vec4 tex = texture2D(sampler2d, texFrag); \n\
		gl_FragColor = vec4(mix(tex.rgb, systemColor.rgb, lerpAlpha), tex.a * systemColor.a); \n\
	} \n\
"

#define SHADER_PixelColoredMultiply SHADER_PIXEL_Colored_Include "\
	void main(void) \n\
	{ \n\
		gl_FragColor = colorFrag; \n\
	} \n\
"
#define SHADER_PixelColoredAlphaMap SHADER_PIXEL_Colored_Include "\
	void main(void) \n\
	{ \n\
		gl_FragColor = colorFrag; \n\
	} \n\
"
#define SHADER_PixelColoredLerp SHADER_PIXEL_Colored_Include "\
	uniform lowp float lerpAlpha; \n\
	void main(void) \n\
	{ \n\
		gl_FragColor = vec4(mix(vec3(1.0, 1.0, 1.0), colorFrag.rgb, lerpAlpha), colorFrag.a); \n\
	} \n\
"

#define SHADER_PixelColoredTexturedMultiply SHADER_PIXEL_ColoredTextured_Include "\
	void main(void) \n\
	{ \n\
		gl_FragColor = texture2D(sampler2d, texFrag) * colorFrag; \n\
	} \n\
"
#define SHADER_PixelColoredTexturedAlphaMap SHADER_PIXEL_ColoredTextured_Include "\
	void main(void) \n\
	{ \n\
		lowp vec4 tex = texture2D(sampler2d, texFrag); \n\
		gl_FragColor = vec4(colorFrag.rgb, colorFrag.a * tex.r); \n\
	} \n\
"
#define SHADER_PixelColoredTexturedLerp SHADER_PIXEL_ColoredTextured_Include "\
	uniform lowp float lerpAlpha; \n\
	void main(void) \n\
	{ \n\
		lowp vec4 tex = texture2D(sampler2d, texFrag); \n\
		gl_FragColor = vec4(mix(tex.rgb, colorFrag.rgb, lerpAlpha), tex.a * colorFrag.a); \n\
	} \n\
"

// pixel shaders (ALPHA-HACK)
#define SHADER_PixelTexturedMultiply_AlphaHack SHADER_PIXEL_Textured_AlphaHack_Include "\
	void main(void) \n\
	{ \n\
		gl_FragColor = vec4(texture2D(sampler2d, texFrag).rgb, texture2D(sampler2dAlpha, texFrag).r) * systemColor; \n\
	} \n\
"
#define SHADER_PixelTexturedLerp_AlphaHack SHADER_PIXEL_Textured_AlphaHack_Include "\
	uniform lowp float lerpAlpha; \n\
	void main(void) \n\
	{ \n\
		lowp vec4 tex = texture2D(sampler2d, texFrag); \n\
		gl_FragColor = vec4(mix(tex.rgb, systemColor.rgb, lerpAlpha), tex.a * systemColor.a); \n\
	} \n\
"

#define SHADER_PixelColoredTexturedMultiply_AlphaHack SHADER_PIXEL_ColoredTextured_AlphaHack_Include "\
	void main(void) \n\
	{ \n\
		gl_FragColor = vec4(texture2D(sampler2d, texFrag).rgb, texture2D(sampler2dAlpha, texFrag).r) * colorFrag; \n\
	} \n\
"
#define SHADER_PixelColoredTexturedLerp_AlphaHack SHADER_PIXEL_ColoredTextured_AlphaHack_Include "\
	uniform lowp float lerpAlpha; \n\
	void main(void) \n\
	{ \n\
		lowp vec4 tex = texture2D(sampler2d, texFrag); \n\
		gl_FragColor = vec4(mix(tex.rgb, colorFrag.rgb, lerpAlpha), texture2D(sampler2dAlpha, texFrag).r * colorFrag.a); \n\
	} \n\
"

#endif
#endif
