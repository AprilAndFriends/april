/// @file
/// @author  Kresimir Spes
/// @author  Ivan Vucica
/// @author  Boris Mikic
/// @version 3.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _OPENGLES1
#include <hltypes/hlog.h>
#include <hltypes/hltypesUtil.h>

#include "april.h"
#include "OpenGLES1_RenderSystem.h"
#include "OpenGLES1_Texture.h"
#include "Platform.h"

namespace april
{
	OpenGLES1_RenderSystem::OpenGLES1_RenderSystem() : OpenGLES_RenderSystem()
	{
		this->name = APRIL_RS_OPENGLES1;
	}

	OpenGLES1_RenderSystem::~OpenGLES1_RenderSystem()
	{
		this->destroy();
	}

	Texture* OpenGLES1_RenderSystem::_createTexture(chstr filename)
	{
		return new OpenGLES1_Texture(filename);
	}

	Texture* OpenGLES1_RenderSystem::_createTexture(int w, int h, unsigned char* rgba)
	{
		return new OpenGLES1_Texture(w, h, rgba);
	}
	
	Texture* OpenGLES1_RenderSystem::_createTexture(int w, int h, Texture::Format format, Texture::Type type, Color color)
	{
		return new OpenGLES1_Texture(w, h, format, type, color);
	}

	void OpenGLES1_RenderSystem::_setVertexPointer(int stride, const void* pointer)
	{
		if (this->deviceState.strideVertex != stride || this->deviceState.pointerVertex != pointer)
		{
			this->deviceState.strideVertex = stride;
			this->deviceState.pointerVertex = pointer;
			glVertexPointer(3, GL_FLOAT, stride, pointer);
		}
	}
	
}

#endif
