/// @file
/// @version 3.6
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifdef _OPENGLES2
#include <hltypes/hlog.h>
#include <hltypes/hltypesUtil.h>

#include "april.h"
#include "egl.h"
#include "OpenGLES2_RenderSystem.h"
#include "OpenGLES2_Texture.h"
#include "Platform.h"

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

	Texture* OpenGLES2_RenderSystem::_createTexture(bool fromResource)
	{
		return new OpenGLES2_Texture(fromResource);
	}

}
#endif
