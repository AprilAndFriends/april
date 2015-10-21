/// @file
/// @version 3.6
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Defines a generic OpenGL "Classic" render system.

#if defined(_OPENGL) || defined(_OPENGLES1)
#ifndef APRIL_OPENGLD_RENDER_SYSTEM_H
#define APRIL_OPENGLD_RENDER_SYSTEM_H

#include "OpenGL_RenderSystem.h"

namespace april
{
	class OpenGLC_Texture;

	class OpenGLC_RenderSystem : public OpenGL_RenderSystem
	{
	public:
		friend class OpenGLC_Texture;

		OpenGLC_RenderSystem();
		~OpenGLC_RenderSystem();

	};
	
}
#endif
#endif
