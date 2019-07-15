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
/// Defines an OpenGL1 specific texture.

#ifdef _OPENGL1
#ifndef APRIL_OPENGL1_TEXTURE_H
#define APRIL_OPENGL1_TEXTURE_H

#include "OpenGL_Texture.h"

namespace april
{
	class OpenGL1_RenderSystem;

	class OpenGL1_Texture : public OpenGL_Texture
	{
	public:
		friend class OpenGL1_RenderSystem;

		OpenGL1_Texture(bool fromResource);

	};

}
#endif
#endif
