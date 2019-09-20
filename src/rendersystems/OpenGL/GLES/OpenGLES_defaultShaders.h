/// @file
/// @version 5.2
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

#define SHADER_AlphaHack_ALPHA_THRESHOLD "0.01" // combination of alpha using compressed RGB components can lead to very small, but non-zero alpha value
#define MAKE_DESATURATE(color) "dot(" #color ".rgb, vec3(0.2125, 0.7154, 0.0721))"
#define MAKE_SEPIA_R(color) #color ".r * 0.393 + " #color ".g * 0.769 + " #color ".b * 0.189"
#define MAKE_SEPIA_G(color) #color ".r * 0.349 + " #color ".g * 0.686 + " #color ".b * 0.168"
#define MAKE_SEPIA_B(color) #color ".r * 0.272 + " #color ".g * 0.534 + " #color ".b * 0.131"
#define MAKE_SEPIA(color) "vec3(" MAKE_SEPIA_R(color) ", " MAKE_SEPIA_G(color) ", " MAKE_SEPIA_B(color) ")"

// general header defines
#define SHADER_Plain_Include "\
"
#define SHADER_Textured_Include "\
	varying highp vec2 texFrag; \n\
"
#define SHADER_Colored_Include "\
	varying highp vec4 colorFrag; \n\
"
#define SHADER_ColoredTextured_Include "\
" SHADER_Colored_Include "\
" SHADER_Textured_Include "\
"

// vertex header defines
#define SHADER_VERTEX_Include "\
	uniform highp mat4 transformationMatrix; \n\
	attribute highp vec4 position; \n\
"
#define SHADER_VERTEX_Plain_Include "\
" SHADER_VERTEX_Include "\
	uniform highp vec4 systemColor; \n\
"
#define SHADER_VERTEX_Textured_Include "\
" SHADER_VERTEX_Include "\
" SHADER_Textured_Include "\
	uniform highp vec4 systemColor; \n\
	attribute highp vec2 tex; \n\
"
#define SHADER_VERTEX_Colored_Include "\
" SHADER_VERTEX_Include "\
" SHADER_Colored_Include "\
	attribute highp vec4 color; \n\
"
#define SHADER_VERTEX_ColoredTextured_Include "\
" SHADER_VERTEX_Include "\
" SHADER_ColoredTextured_Include "\
	attribute highp vec4 color; \n\
	attribute highp vec2 tex; \n\
"

// pixel header defines
#define SHADER_START_ExTextured "\
	#extension GL_OES_EGL_image_external : require\n\
"

#define SHADER_PIXEL_Plain_Include "\
	uniform highp vec4 systemColor; \n\
"
#define SHADER_PIXEL_Textured_Include "\
" SHADER_Textured_Include "\
	uniform highp vec4 systemColor; \n\
	uniform sampler2D sampler2d; \n\
"
#define SHADER_PIXEL_ExTextured_Include "\
" SHADER_START_ExTextured "\
" SHADER_Textured_Include "\
	uniform highp vec4 systemColor; \n\
	uniform samplerExternalOES sampler2d; \n\
"
#define SHADER_PIXEL_Colored_Include "\
" SHADER_Colored_Include "\
"
#define SHADER_PIXEL_ColoredTextured_Include "\
" SHADER_ColoredTextured_Include "\
	uniform sampler2D sampler2d; \n\
"
#define SHADER_PIXEL_ColoredExTextured_Include "\
" SHADER_START_ExTextured "\
" SHADER_ColoredTextured_Include "\
	uniform samplerExternalOES sampler2d; \n\
"

// pixel header defines (ALPHA-HACK)
#define SHADER_PIXEL_Textured_AlphaHack_Include "\
" SHADER_Textured_Include "\
	uniform highp vec4 systemColor; \n\
	uniform sampler2D sampler2d; \n\
	uniform sampler2D sampler2dAlpha; \n\
"
#define SHADER_PIXEL_ExTextured_AlphaHack_Include "\
" SHADER_START_ExTextured "\
" SHADER_Textured_Include "\
	uniform highp vec4 systemColor; \n\
	uniform samplerExternalOES sampler2d; \n\
	uniform samplerExternalOES sampler2dAlpha; \n\
"
#define SHADER_PIXEL_ColoredTextured_AlphaHack_Include "\
" SHADER_ColoredTextured_Include "\
	uniform sampler2D sampler2d; \n\
	uniform sampler2D sampler2dAlpha; \n\
"
#define SHADER_PIXEL_ColoredExTextured_AlphaHack_Include "\
" SHADER_START_ExTextured "\
" SHADER_ColoredTextured_Include "\
	uniform samplerExternalOES sampler2d; \n\
	uniform samplerExternalOES sampler2dAlpha; \n\
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
	uniform highp float lerpAlpha; \n\
	void main(void) \n\
	{ \n\
		gl_FragColor = vec4(mix(vec3(1.0, 1.0, 1.0), systemColor.rgb, lerpAlpha), systemColor.a); \n\
	} \n\
"
#define SHADER_PixelDesaturate SHADER_PIXEL_Plain_Include "\
	uniform highp float lerpAlpha; \n\
	void main(void) \n\
	{ \n\
		highp float value = " MAKE_DESATURATE(systemColor) "; \n\
		gl_FragColor = vec4(value, value, value, systemColor.a); \n\
	} \n\
"
#define SHADER_PixelSepia SHADER_PIXEL_Plain_Include "\
	uniform highp float lerpAlpha; \n\
	void main(void) \n\
	{ \n\
		gl_FragColor = vec4(" MAKE_SEPIA(systemColor) ", systemColor.a); \n\
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
		gl_FragColor = vec4(systemColor.rgb, texture2D(sampler2d, texFrag).r * systemColor.a); \n\
	} \n\
"
#define SHADER_PixelTexturedLerp SHADER_PIXEL_Textured_Include "\
	uniform highp float lerpAlpha; \n\
	void main(void) \n\
	{ \n\
		highp vec4 tex = texture2D(sampler2d, texFrag); \n\
		gl_FragColor = vec4(mix(tex.rgb, systemColor.rgb, lerpAlpha), tex.a * systemColor.a); \n\
	} \n\
"
#define SHADER_PixelTexturedDesaturate SHADER_PIXEL_Textured_Include "\
	uniform highp float lerpAlpha; \n\
	void main(void) \n\
	{ \n\
		highp vec4 tex = texture2D(sampler2d, texFrag); \n\
		highp float value = " MAKE_DESATURATE(tex) "; \n\
		gl_FragColor = vec4(value * systemColor.r, value * systemColor.g, value * systemColor.b, tex.a * systemColor.a); \n\
	} \n\
"
#define SHADER_PixelTexturedSepia SHADER_PIXEL_Textured_Include "\
	uniform highp float lerpAlpha; \n\
	void main(void) \n\
	{ \n\
		highp vec4 tex = texture2D(sampler2d, texFrag); \n\
		gl_FragColor = vec4(" MAKE_SEPIA(tex) " * systemColor.rgb, tex.a * systemColor.a); \n\
	} \n\
"

#define SHADER_PixelExTexturedMultiply SHADER_PIXEL_ExTextured_Include "\
	void main(void) \n\
	{ \n\
		gl_FragColor = texture2D(sampler2d, texFrag) * systemColor; \n\
	} \n\
"
#define SHADER_PixelExTexturedAlphaMap SHADER_PIXEL_ExTextured_Include "\
	void main(void) \n\
	{ \n\
		gl_FragColor = vec4(systemColor.rgb, texture2D(sampler2d, texFrag).r * systemColor.a); \n\
	} \n\
"
#define SHADER_PixelExTexturedLerp SHADER_PIXEL_ExTextured_Include "\
	uniform highp float lerpAlpha; \n\
	void main(void) \n\
	{ \n\
		highp vec4 tex = texture2D(sampler2d, texFrag); \n\
		gl_FragColor = vec4(mix(tex.rgb, systemColor.rgb, lerpAlpha), tex.a * systemColor.a); \n\
	} \n\
"
#define SHADER_PixelExTexturedDesaturate SHADER_PIXEL_ExTextured_Include "\
	uniform highp float lerpAlpha; \n\
	void main(void) \n\
	{ \n\
		highp vec4 tex = texture2D(sampler2d, texFrag); \n\
		highp float value = " MAKE_DESATURATE(tex) "; \n\
		gl_FragColor = vec4(value * systemColor.r, value * systemColor.g, value * systemColor.b, tex.a * systemColor.a); \n\
	} \n\
"
#define SHADER_PixelExTexturedSepia SHADER_PIXEL_ExTextured_Include "\
	uniform highp float lerpAlpha; \n\
	void main(void) \n\
	{ \n\
		highp vec4 tex = texture2D(sampler2d, texFrag); \n\
		gl_FragColor = vec4(" MAKE_SEPIA(tex) " * systemColor.rgb, tex.a * systemColor.a); \n\
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
	uniform highp float lerpAlpha; \n\
	void main(void) \n\
	{ \n\
		gl_FragColor = vec4(mix(vec3(1.0, 1.0, 1.0), colorFrag.rgb, lerpAlpha), colorFrag.a); \n\
	} \n\
"
#define SHADER_PixelColoredDesaturate SHADER_PIXEL_Colored_Include "\
	uniform highp float lerpAlpha; \n\
	void main(void) \n\
	{ \n\
		gl_FragColor = colorFrag; \n\
	} \n\
"
#define SHADER_PixelColoredSepia SHADER_PIXEL_Colored_Include "\
	uniform highp float lerpAlpha; \n\
	void main(void) \n\
	{ \n\
		gl_FragColor = colorFrag; \n\
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
		gl_FragColor = vec4(colorFrag.rgb, texture2D(sampler2d, texFrag).r * colorFrag.a); \n\
	} \n\
"
#define SHADER_PixelColoredTexturedLerp SHADER_PIXEL_ColoredTextured_Include "\
	uniform highp float lerpAlpha; \n\
	void main(void) \n\
	{ \n\
		highp vec4 tex = texture2D(sampler2d, texFrag); \n\
		gl_FragColor = vec4(mix(tex.rgb, colorFrag.rgb, lerpAlpha), tex.a * colorFrag.a); \n\
	} \n\
"
#define SHADER_PixelColoredTexturedDesaturate SHADER_PIXEL_ColoredTextured_Include "\
	uniform highp float lerpAlpha; \n\
	void main(void) \n\
	{ \n\
		highp vec4 tex = texture2D(sampler2d, texFrag); \n\
		highp float value = " MAKE_DESATURATE(tex) "; \n\
		gl_FragColor = vec4(value * colorFrag.r, value * colorFrag.g, value * colorFrag.b, tex.a * colorFrag.a); \n\
	} \n\
"
#define SHADER_PixelColoredTexturedSepia SHADER_PIXEL_ColoredTextured_Include "\
	uniform highp float lerpAlpha; \n\
	void main(void) \n\
	{ \n\
		highp vec4 tex = texture2D(sampler2d, texFrag); \n\
		gl_FragColor = vec4(" MAKE_SEPIA(tex) " * colorFrag.rgb, tex.a * colorFrag.a); \n\
	} \n\
"

#define SHADER_PixelColoredExTexturedMultiply SHADER_PIXEL_ColoredExTextured_Include "\
	void main(void) \n\
	{ \n\
		gl_FragColor = texture2D(sampler2d, texFrag) * colorFrag; \n\
	} \n\
"
#define SHADER_PixelColoredExTexturedAlphaMap SHADER_PIXEL_ColoredExTextured_Include "\
	void main(void) \n\
	{ \n\
		gl_FragColor = vec4(colorFrag.rgb, texture2D(sampler2d, texFrag).r * colorFrag.a); \n\
	} \n\
"
#define SHADER_PixelColoredExTexturedLerp SHADER_PIXEL_ColoredExTextured_Include "\
	uniform highp float lerpAlpha; \n\
	void main(void) \n\
	{ \n\
		highp vec4 tex = texture2D(sampler2d, texFrag); \n\
		gl_FragColor = vec4(mix(tex.rgb, colorFrag.rgb, lerpAlpha), tex.a * colorFrag.a); \n\
	} \n\
"
#define SHADER_PixelColoredExTexturedDesaturate SHADER_PIXEL_ColoredExTextured_Include "\
	uniform highp float lerpAlpha; \n\
	void main(void) \n\
	{ \n\
		highp vec4 tex = texture2D(sampler2d, texFrag); \n\
		highp float value = " MAKE_DESATURATE(tex) "; \n\
		gl_FragColor = vec4(value * colorFrag.r, value * colorFrag.g, value * colorFrag.b, tex.a * colorFrag.a); \n\
	} \n\
"
#define SHADER_PixelColoredExTexturedSepia SHADER_PIXEL_ColoredExTextured_Include "\
	uniform highp float lerpAlpha; \n\
	void main(void) \n\
	{ \n\
		highp vec4 tex = texture2D(sampler2d, texFrag); \n\
		gl_FragColor = vec4(" MAKE_SEPIA(tex) " * colorFrag.rgb, tex.a * colorFrag.a); \n\
	} \n\
"

// pixel shaders (ALPHA-HACK)
#define SHADER_PixelTexturedMultiply_AlphaHack SHADER_PIXEL_Textured_AlphaHack_Include "\
	void main(void) \n\
	{ \n\
		highp vec4 newColor = vec4(texture2D(sampler2d, texFrag).rgb, texture2D(sampler2dAlpha, texFrag).r) * systemColor; \n\
		if (newColor.a < " SHADER_AlphaHack_ALPHA_THRESHOLD ") \n\
		{ \n\
			discard; \n\
		} \n\
		gl_FragColor = newColor; \n\
	} \n\
"
#define SHADER_PixelTexturedLerp_AlphaHack SHADER_PIXEL_Textured_AlphaHack_Include "\
	uniform highp float lerpAlpha; \n\
	void main(void) \n\
	{ \n\
		highp vec4 newColor = vec4(mix(texture2D(sampler2d, texFrag).rgb, systemColor.rgb, lerpAlpha), texture2D(sampler2dAlpha, texFrag).r * systemColor.a); \n\
		if (newColor.a < " SHADER_AlphaHack_ALPHA_THRESHOLD ") \n\
		{ \n\
			discard; \n\
		} \n\
		gl_FragColor = newColor; \n\
	} \n\
"
#define SHADER_PixelTexturedDesaturate_AlphaHack SHADER_PIXEL_Textured_AlphaHack_Include "\
	uniform highp float lerpAlpha; \n\
	void main(void) \n\
	{ \n\
		highp vec4 tex = texture2D(sampler2d, texFrag); \n\
		highp float value = " MAKE_DESATURATE(tex) "; \n\
		highp vec4 newColor = vec4(value * systemColor.r, value * systemColor.g, value * systemColor.b, texture2D(sampler2dAlpha, texFrag).r * systemColor.a); \n\
		if (newColor.a < " SHADER_AlphaHack_ALPHA_THRESHOLD ") \n\
		{ \n\
			discard; \n\
		} \n\
		gl_FragColor = newColor; \n\
	} \n\
"
#define SHADER_PixelTexturedSepia_AlphaHack SHADER_PIXEL_Textured_AlphaHack_Include "\
	uniform highp float lerpAlpha; \n\
	void main(void) \n\
	{ \n\
		highp vec4 tex = texture2D(sampler2d, texFrag); \n\
		highp vec4 newColor = vec4(" MAKE_SEPIA(tex) " * systemColor.rgb, texture2D(sampler2dAlpha, texFrag).r * systemColor.a); \n\
		if (newColor.a < " SHADER_AlphaHack_ALPHA_THRESHOLD ") \n\
		{ \n\
			discard; \n\
		} \n\
		gl_FragColor = newColor; \n\
	} \n\
"

#define SHADER_PixelExTexturedMultiply_AlphaHack SHADER_PIXEL_ExTextured_AlphaHack_Include "\
	void main(void) \n\
	{ \n\
		highp vec4 newColor = vec4(texture2D(sampler2d, texFrag).rgb, texture2D(sampler2dAlpha, texFrag).r) * systemColor; \n\
		if (newColor.a < " SHADER_AlphaHack_ALPHA_THRESHOLD ") \n\
		{ \n\
			discard; \n\
		} \n\
		gl_FragColor = newColor; \n\
	} \n\
"
#define SHADER_PixelExTexturedLerp_AlphaHack SHADER_PIXEL_ExTextured_AlphaHack_Include "\
	uniform highp float lerpAlpha; \n\
	void main(void) \n\
	{ \n\
		highp vec4 newColor = vec4(mix(texture2D(sampler2d, texFrag).rgb, systemColor.rgb, lerpAlpha), texture2D(sampler2dAlpha, texFrag).r * systemColor.a); \n\
		if (newColor.a < " SHADER_AlphaHack_ALPHA_THRESHOLD ") \n\
		{ \n\
			discard; \n\
		} \n\
		gl_FragColor = newColor; \n\
	} \n\
"
#define SHADER_PixelExTexturedDesaturate_AlphaHack SHADER_PIXEL_ExTextured_AlphaHack_Include "\
	uniform highp float lerpAlpha; \n\
	void main(void) \n\
	{ \n\
		highp vec4 tex = texture2D(sampler2d, texFrag); \n\
		highp float value = " MAKE_DESATURATE(tex) "; \n\
		highp vec4 newColor = vec4(value * systemColor.r, value * systemColor.g, value * systemColor.b, texture2D(sampler2dAlpha, texFrag).r * systemColor.a); \n\
		if (newColor.a < " SHADER_AlphaHack_ALPHA_THRESHOLD ") \n\
		{ \n\
			discard; \n\
		} \n\
		gl_FragColor = newColor; \n\
	} \n\
"
#define SHADER_PixelExTexturedSepia_AlphaHack SHADER_PIXEL_ExTextured_AlphaHack_Include "\
	uniform highp float lerpAlpha; \n\
	void main(void) \n\
	{ \n\
		highp vec4 tex = texture2D(sampler2d, texFrag); \n\
		highp vec4 newColor = vec4(" MAKE_SEPIA(tex) " * systemColor.rgb, texture2D(sampler2dAlpha, texFrag).r * systemColor.a); \n\
		if (newColor.a < " SHADER_AlphaHack_ALPHA_THRESHOLD ") \n\
		{ \n\
			discard; \n\
		} \n\
		gl_FragColor = newColor; \n\
	} \n\
"

#define SHADER_PixelColoredTexturedMultiply_AlphaHack SHADER_PIXEL_ColoredTextured_AlphaHack_Include "\
	void main(void) \n\
	{ \n\
		highp vec4 newColor = vec4(texture2D(sampler2d, texFrag).rgb, texture2D(sampler2dAlpha, texFrag).r) * colorFrag; \n\
		if (newColor.a < " SHADER_AlphaHack_ALPHA_THRESHOLD ") \n\
		{ \n\
			discard; \n\
		} \n\
		gl_FragColor = newColor; \n\
	} \n\
"
#define SHADER_PixelColoredTexturedLerp_AlphaHack SHADER_PIXEL_ColoredTextured_AlphaHack_Include "\
	uniform highp float lerpAlpha; \n\
	void main(void) \n\
	{ \n\
		highp vec4 newColor = vec4(mix(texture2D(sampler2d, texFrag).rgb, colorFrag.rgb, lerpAlpha), texture2D(sampler2dAlpha, texFrag).r * colorFrag.a); \n\
		if (newColor.a < " SHADER_AlphaHack_ALPHA_THRESHOLD ") \n\
		{ \n\
			discard; \n\
		} \n\
		gl_FragColor = newColor; \n\
	} \n\
"
#define SHADER_PixelColoredTexturedDesaturate_AlphaHack SHADER_PIXEL_ColoredTextured_AlphaHack_Include "\
	uniform highp float lerpAlpha; \n\
	void main(void) \n\
	{ \n\
		highp vec4 tex = texture2D(sampler2d, texFrag); \n\
		highp float value = " MAKE_DESATURATE(tex) "; \n\
		highp vec4 newColor = vec4(value * colorFrag.r, value * colorFrag.g, value * colorFrag.b, texture2D(sampler2dAlpha, texFrag).r * colorFrag.a); \n\
		if (newColor.a < " SHADER_AlphaHack_ALPHA_THRESHOLD ") \n\
		{ \n\
			discard; \n\
		} \n\
		gl_FragColor = newColor; \n\
	} \n\
"
#define SHADER_PixelColoredTexturedSepia_AlphaHack SHADER_PIXEL_ColoredTextured_AlphaHack_Include "\
	uniform highp float lerpAlpha; \n\
	void main(void) \n\
	{ \n\
		highp vec4 tex = texture2D(sampler2d, texFrag); \n\
		highp vec4 newColor = vec4(" MAKE_SEPIA(tex) " * colorFrag.rgb, texture2D(sampler2dAlpha, texFrag).r * colorFrag.a); \n\
		if (newColor.a < " SHADER_AlphaHack_ALPHA_THRESHOLD ") \n\
		{ \n\
			discard; \n\
		} \n\
		gl_FragColor = newColor; \n\
	} \n\
"

#define SHADER_PixelColoredExTexturedMultiply_AlphaHack SHADER_PIXEL_ColoredExTextured_AlphaHack_Include "\
	void main(void) \n\
	{ \n\
		highp vec4 newColor = vec4(texture2D(sampler2d, texFrag).rgb, texture2D(sampler2dAlpha, texFrag).r) * colorFrag; \n\
		if (newColor.a < " SHADER_AlphaHack_ALPHA_THRESHOLD ") \n\
		{ \n\
			discard; \n\
		} \n\
		gl_FragColor = newColor; \n\
	} \n\
"
#define SHADER_PixelColoredExTexturedLerp_AlphaHack SHADER_PIXEL_ColoredExTextured_AlphaHack_Include "\
	uniform highp float lerpAlpha; \n\
	void main(void) \n\
	{ \n\
		highp vec4 newColor = vec4(mix(texture2D(sampler2d, texFrag).rgb, colorFrag.rgb, lerpAlpha), texture2D(sampler2dAlpha, texFrag).r * colorFrag.a); \n\
		if (newColor.a < " SHADER_AlphaHack_ALPHA_THRESHOLD ") \n\
		{ \n\
			discard; \n\
		} \n\
		gl_FragColor = newColor; \n\
	} \n\
"
#define SHADER_PixelColoredExTexturedDesaturate_AlphaHack SHADER_PIXEL_ColoredExTextured_AlphaHack_Include "\
	uniform highp float lerpAlpha; \n\
	void main(void) \n\
	{ \n\
		highp vec4 tex = texture2D(sampler2d, texFrag); \n\
		highp float value = " MAKE_DESATURATE(tex) "; \n\
		highp vec4 newColor = vec4(value * colorFrag.r, value * colorFrag.g, value * colorFrag.b, texture2D(sampler2dAlpha, texFrag).r * colorFrag.a); \n\
		if (newColor.a < " SHADER_AlphaHack_ALPHA_THRESHOLD ") \n\
		{ \n\
			discard; \n\
		} \n\
		gl_FragColor = newColor; \n\
	} \n\
"
#define SHADER_PixelColoredExTexturedSepia_AlphaHack SHADER_PIXEL_ColoredExTextured_AlphaHack_Include "\
	uniform highp float lerpAlpha; \n\
	void main(void) \n\
	{ \n\
		highp vec4 tex = texture2D(sampler2d, texFrag); \n\
		highp vec4 newColor = vec4(" MAKE_SEPIA(tex) " * colorFrag.rgb, texture2D(sampler2dAlpha, texFrag).r * colorFrag.a); \n\
		if (newColor.a < " SHADER_AlphaHack_ALPHA_THRESHOLD ") \n\
		{ \n\
			discard; \n\
		} \n\
		gl_FragColor = newColor; \n\
	} \n\
"

#endif
#endif
