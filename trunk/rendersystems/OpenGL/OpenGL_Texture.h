/// @file
/// @author  Kresimir Spes
/// @version 1.31
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/// 
/// @section DESCRIPTION
/// 
/// Defines an OpenGL specific texture.

#if defined(_OPENGL) || (_OPENGLES1)
#ifndef APRIL_OPENGL_TEXTURE_H
#define APRIL_OPENGL_TEXTURE_H

#ifdef __APPLE__
#include <TargetConditionals.h>
#endif
#if (TARGET_OS_MAC) && !(TARGET_OS_IPHONE)
#include <OpenGL/gl.h>
#elif (TARGET_OS_IPHONE)
#include <OpenGLES/ES1/gl.h>
#elif (_OPENGLES1)
#include <GLES/gl.h>
#else
#include <GL/gl.h>
#endif

#include "RenderSystem.h"

namespace april
{
	class OpenGL_Texture : public Texture
	{
	public:
		GLuint mTexId;
		
		OpenGL_Texture(chstr filename, bool dynamic);
		OpenGL_Texture(unsigned char* rgba, int w, int h);
		OpenGL_Texture(int w, int h);
		~OpenGL_Texture();
		
		bool load();
		void unload();
		bool isLoaded();
		void saturate(float factor);
		int getSizeInBytes();
		
	};

}

#endif
#endif
