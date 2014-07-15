/// @file
/// @version 3.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Defines a generic OpenGLES texture.

#ifdef _OPENGLES
#ifndef APRIL_OPENGLES_TEXTURE_H
#define APRIL_OPENGLES_TEXTURE_H

#include "OpenGL_Texture.h"

namespace april
{
	class OpenGLES_Texture : public OpenGL_Texture
	{
	public:
		OpenGLES_Texture(bool fromResource);
		~OpenGLES_Texture();

	};

}

#endif
#endif
