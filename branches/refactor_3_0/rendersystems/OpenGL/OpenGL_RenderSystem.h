/// @file
/// @author  Kresimir Spes
/// @author  Ivan Vucica
/// @author  Boris Mikic
/// @version 3.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/// 
/// @section DESCRIPTION
/// 
/// Defines a generic OpenGL render system.

#ifdef _OPENGL
#ifndef APRIL_OPENGL_RENDER_SYSTEM_H
#define APRIL_OPENGL_RENDER_SYSTEM_H

#include "RenderSystem.h"

namespace april
{
	class OpenGL_RenderSystem : public RenderSystem
	{
	public:
		OpenGL_RenderSystem();
		~OpenGL_RenderSystem();

	};
	
}
#endif
#endif
