/// @file
/// @author  Kresimir Spes
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
/// Defines an OpenGLES2 render system.

#ifdef _OPENGLES2
#ifndef APRIL_OPENGLES2_RENDER_SYSTEM_H
#define APRIL_OPENGLES2_RENDER_SYSTEM_H

#include "OpenGLES_RenderSystem.h"

namespace april
{
	class OpenGLES2_RenderSystem : public OpenGLES_RenderSystem
	{
	public:
		OpenGLES2_RenderSystem();
		~OpenGLES2_RenderSystem();

	};
	
}
#endif
#endif
