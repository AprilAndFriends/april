/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @version 3.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/// 
/// @section DESCRIPTION
/// 
/// Defines an OpenGL1 specific texture.

#ifdef _OPENGL1
#ifndef APRIL_OPENGL1_TEXTURE_H
#define APRIL_OPENGL1_TEXTURE_H

#ifdef __APPLE__
#include <TargetConditionals.h>
#endif
#if (TARGET_OS_MAC) && !(TARGET_OS_IPHONE)
#include <OpenGL/gl.h>
#elif (TARGET_OS_IPHONE)
#include <OpenGLES/ES1/gl.h>
#elif (_OPENGLES)
#include <GLES/gl.h>
#else
#include <GL/gl.h>
#endif

#include "OpenGL_Texture.h"

namespace april
{
	class OpenGL1_RenderSystem;

	class OpenGL1_Texture : public OpenGL_Texture
	{
	public:
		OpenGL1_Texture(chstr filename);
		OpenGL1_Texture(int w, int h, unsigned char* rgba);
		OpenGL1_Texture(int w, int h, Format format, Type type, Color color = Color::Clear);
		~OpenGL1_Texture();

		bool copyPixelData(unsigned char** output);

	};

}

#endif
#endif
