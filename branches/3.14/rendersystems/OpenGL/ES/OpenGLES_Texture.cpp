/// @file
/// @author  Kresimir Spes
/// @author  Ivan Vucica
/// @author  Boris Mikic
/// @version 3.1
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _OPENGLES
#include <hltypes/hstring.h>

#include "Color.h"
#include "OpenGLES_Texture.h"

namespace april
{
	OpenGLES_Texture::OpenGLES_Texture(chstr filename) : OpenGL_Texture(filename)
	{
	}

	OpenGLES_Texture::OpenGLES_Texture(int w, int h, unsigned char* rgba) : OpenGL_Texture(w, h, rgba)
	{
	}

	OpenGLES_Texture::OpenGLES_Texture(int w, int h, Format format, Type type, Color color) :
		OpenGL_Texture(w, h, format, type, color)
	{
	}

	OpenGLES_Texture::~OpenGLES_Texture()
	{
	}

	bool OpenGLES_Texture::copyPixelData(unsigned char** output)
	{
		return false;
	}
	
}
#endif
