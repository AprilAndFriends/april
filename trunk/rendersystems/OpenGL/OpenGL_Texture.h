/************************************************************************************\
This source file is part of the Awesome Portable Rendering Interface Library         *
For latest info, see http://libapril.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes                                                     *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#ifdef _OPENGL
#ifndef APRIL_OPENGL_TEXTURE_H
#define APRIL_OPENGL_TEXTURE_H

#ifdef __APPLE__
#include <TargetConditionals.h>
#endif
#if (TARGET_OS_MAC) && !(TARGET_OS_IPHONE)
#include <OpenGL/gl.h>
#elif (TARGET_OS_IPHONE)
#include <OpenGLES/ES1/gl.h>
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
		~OpenGL_Texture();
		
		bool load();
		void unload();
		bool isLoaded();
		int getSizeInBytes();
		
	};

}

#endif
#endif
