/// @file
/// @version 4.2
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
#include "OpenGLES2_PixelShader.h"
#include "OpenGLES2_Texture.h"
#include "OpenGLES2_VertexShader.h"
#include "Platform.h"

namespace april
{
	OpenGLES2_RenderSystem::OpenGLES2_RenderSystem() : OpenGLES_RenderSystem()
	{
		this->name = april::RenderSystemType::OpenGLES2.getName();
	}

	OpenGLES2_RenderSystem::~OpenGLES2_RenderSystem()
	{
		this->destroy();
	}

	Texture* OpenGLES2_RenderSystem::_deviceCreateTexture(bool fromResource)
	{
		return new OpenGLES2_Texture(fromResource);
	}

	PixelShader* OpenGLES2_RenderSystem::_deviceCreatePixelShader()
	{
		return new OpenGLES2_PixelShader();
	}

	VertexShader* OpenGLES2_RenderSystem::_deviceCreateVertexShader()
	{
		return new OpenGLES2_VertexShader();
	}

}
#endif
