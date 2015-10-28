/// @file
/// @version 4.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Defines a generic OpenGL "Classic" texture.

#if (defined(_OPENGL) || defined(_OPENGLES1)) && !defined(_OPENGLES)
#ifndef APRIL_OPENGLC_TEXTURE_H
#define APRIL_OPENGLC_TEXTURE_H

#include "OpenGL_Texture.h"

namespace april
{
	class OpenGLC_RenderSystem;

	class OpenGLC_Texture : public OpenGL_Texture
	{
	public:
		friend class OpenGLC_RenderSystem;

		OpenGLC_Texture(bool fromResource);
		~OpenGLC_Texture();

	};

}
#endif
#endif
