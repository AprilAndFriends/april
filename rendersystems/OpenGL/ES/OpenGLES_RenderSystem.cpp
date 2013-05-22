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

#ifdef _OPENGLES

#include <hltypes/hplatform.h>
#if __APPLE__
#include <TargetConditionals.h>
#endif

#include <hltypes/hlog.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hstring.h>

#include "april.h"
#ifdef _EGL
#include "egl.h"
#endif
#include "OpenGLES_RenderSystem.h"
#include "Window.h"

namespace april
{
	OpenGLES_RenderSystem::OpenGLES_RenderSystem() : OpenGL_RenderSystem()
	{
	}

	OpenGLES_RenderSystem::~OpenGLES_RenderSystem()
	{
	}

	void OpenGLES_RenderSystem::assignWindow(Window* window)
	{
		OpenGL_RenderSystem::assignWindow(window);
	}

	int OpenGLES_RenderSystem::getMaxTextureSize()
	{
#ifdef _WIN32
		if (april::egl->display == NULL)
		{
			return 0;
		}
#endif
		return OpenGL_RenderSystem::getMaxTextureSize();
	}

	void OpenGLES_RenderSystem::_setTextureBlendMode(BlendMode textureBlendMode)
	{
		// TODO - is there a way to make this work on Win32?
#ifndef _WIN32
		// TODO - refactor
		static int blendSeparationSupported = -1;
		if (blendSeparationSupported == -1)
		{
			// determine if blend separation is possible on first call to this function
			hstr extensions = (const char*)glGetString(GL_EXTENSIONS);
			blendSeparationSupported = extensions.contains("OES_blend_equation_separate") && extensions.contains("OES_blend_func_separate");
		}
		if (blendSeparationSupported)
		{
			// blending for the new generations
			switch (textureBlendMode)
			{
			case DEFAULT:
			case ALPHA_BLEND:
				glBlendEquationSeparateOES(GL_FUNC_ADD_OES, GL_FUNC_ADD_OES);
				glBlendFuncSeparateOES(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
				break;
			case ADD:
				glBlendEquationSeparateOES(GL_FUNC_ADD_OES, GL_FUNC_ADD_OES);
				glBlendFuncSeparateOES(GL_SRC_ALPHA, GL_ONE, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
				break;
			case SUBTRACT:
				glBlendEquationSeparateOES(GL_FUNC_REVERSE_SUBTRACT_OES, GL_FUNC_ADD_OES);
				glBlendFuncSeparateOES(GL_SRC_ALPHA, GL_ONE, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
				break;
			case OVERWRITE:
				glBlendEquationSeparateOES(GL_FUNC_ADD_OES, GL_FUNC_ADD_OES);
				glBlendFuncSeparateOES(GL_ONE, GL_ZERO, GL_ONE, GL_ZERO);				
				break;
			default:
				hlog::warn(april::logTag, "Trying to set unsupported blend mode!");
				break;
			}
		}
		else
#endif
		{
			// old-school blending mode for your mom
			OpenGL_RenderSystem::_setTextureBlendMode(textureBlendMode);
		}
	}
	
}
#endif
