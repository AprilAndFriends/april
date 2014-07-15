/// @file
/// @version 3.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifdef _OPENGLES
#include <hltypes/hstring.h>

#include "Color.h"
#include "OpenGLES_Texture.h"

namespace april
{
	OpenGLES_Texture::OpenGLES_Texture(bool fromResource) : OpenGL_Texture(fromResource)
	{
	}

	OpenGLES_Texture::~OpenGLES_Texture()
	{
	}

}
#endif
