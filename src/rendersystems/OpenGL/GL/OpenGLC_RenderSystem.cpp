/// @file
/// @version 3.6
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#if (defined(_OPENGL) || defined(_OPENGLES1)) && !defined(_OPENGLES)
#include "OpenGLC_RenderSystem.h"

namespace april
{
	OpenGLC_RenderSystem::OpenGLC_RenderSystem() : OpenGL_RenderSystem()
	{
	}

	OpenGLC_RenderSystem::~OpenGLC_RenderSystem()
	{
	}

	void OpenGLC_RenderSystem::_setupDefaultParameters()
	{
		OpenGL_RenderSystem::_setupDefaultParameters();
		glEnableClientState(GL_VERTEX_ARRAY);
		this->_setClientState(GL_TEXTURE_COORD_ARRAY, this->deviceState.textureCoordinatesEnabled);
		this->_setClientState(GL_COLOR_ARRAY, this->deviceState.colorEnabled);
		glColor4f(this->deviceState.systemColor.r_f(), this->deviceState.systemColor.g_f(), this->deviceState.systemColor.b_f(), this->deviceState.systemColor.a_f());
	}

}
#endif
