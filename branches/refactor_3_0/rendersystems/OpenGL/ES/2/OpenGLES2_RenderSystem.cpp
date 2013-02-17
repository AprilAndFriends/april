/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @version 3.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _OPENGLES2

#include "OpenGLES2_RenderSystem.h"

namespace april
{
	OpenGLES2_RenderSystem::OpenGLES2_RenderSystem() : OpenGLES_RenderSystem()
	{
		this->name = APRIL_RS_OPENGLES2;
	}

	OpenGLES2_RenderSystem::~OpenGLES2_RenderSystem()
	{
		this->destroy();
	}

}
#endif
