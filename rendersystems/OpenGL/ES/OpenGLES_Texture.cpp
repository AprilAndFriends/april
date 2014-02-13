/// @file
/// @author  Kresimir Spes
/// @author  Ivan Vucica
/// @author  Boris Mikic
/// @version 3.3
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
	OpenGLES_Texture::OpenGLES_Texture() : OpenGL_Texture()
	{
	}

	OpenGLES_Texture::~OpenGLES_Texture()
	{
	}

	bool OpenGLES_Texture::copyPixelData(unsigned char** output, Image::Format format)
	{
		// TODOa - there has to be a way how this could be done
		return false;
	}
	
}
#endif
