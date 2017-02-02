/// @file
/// @version 4.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#if (defined(_OPENGL) || defined(_OPENGLES1)) && !defined(_OPENGLES)
#include "OpenGLC_Texture.h"

namespace april
{
	OpenGLC_Texture::OpenGLC_Texture(bool fromResource) : OpenGL_Texture(fromResource)
	{
	}

	OpenGLC_Texture::~OpenGLC_Texture()
	{
	}

}
#endif
