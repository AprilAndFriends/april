/// @file
/// @version 4.1
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifdef _OPENGLES1
#include <hltypes/hlog.h>
#include <hltypes/hltypesUtil.h>

#include "april.h"
#include "egl.h"
#include "OpenGLES1_RenderSystem.h"
#include "OpenGLES1_Texture.h"
#include "Platform.h"

namespace april
{
	OpenGLES1_RenderSystem::OpenGLES1_RenderSystem() : OpenGLC_RenderSystem()
	{
		this->name = april::RenderSystemType::OpenGLES1.getName();
	}

	OpenGLES1_RenderSystem::~OpenGLES1_RenderSystem()
	{
		this->destroy();
	}

	void OpenGLES1_RenderSystem::_deviceSuspend()
	{
		OpenGL_RenderSystem::_deviceSuspend();
		this->unloadTextures();
	}

	void OpenGLES1_RenderSystem::_deviceSetupCaps()
	{
#ifdef _EGL
		if (april::egl->display == NULL)
		{
			return;
		}
#endif
		hstr extensions = (const char*)glGetString(GL_EXTENSIONS);
#ifndef _WINRT
		this->caps.npotTexturesLimited = (extensions.contains("IMG_texture_npot") || extensions.contains("APPLE_texture_2D_limited_npot"));
#else
		this->caps.npotTexturesLimited = true;
#endif
		this->caps.npotTextures = (extensions.contains("OES_texture_npot") || extensions.contains("ARB_texture_non_power_of_two"));
		// TODO - is there a way to make this work on Win32?
#ifndef _WIN32
		this->blendSeparationSupported = (extensions.contains("OES_blend_equation_separate") && extensions.contains("OES_blend_func_separate"));
#endif
#ifdef _ANDROID // Android has problems with alpha textures in some implementations
		this->caps.textureFormats /= Image::FORMAT_ALPHA;
		this->caps.textureFormats /= Image::FORMAT_GRAYSCALE;
#endif
		return OpenGLC_RenderSystem::_deviceSetupCaps();
	}

	Texture* OpenGLES1_RenderSystem::_deviceCreateTexture(bool fromResource)
	{
		return new OpenGLES1_Texture(fromResource);
	}

	void OpenGLES1_RenderSystem::_setDeviceBlendMode(BlendMode blendMode)
	{
#ifndef _WIN32
		if (this->blendSeparationSupported)
		{
			// blending for the new generations
			if (blendMode == BM_ALPHA || blendMode == BM_DEFAULT)
			{
				glBlendEquationSeparateOES(GL_FUNC_ADD_OES, GL_FUNC_ADD_OES);
				glBlendFuncSeparateOES(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
			}
			else if (blendMode == BM_ADD)
			{
				glBlendEquationSeparateOES(GL_FUNC_ADD_OES, GL_FUNC_ADD_OES);
				glBlendFuncSeparateOES(GL_SRC_ALPHA, GL_ONE, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
			}
			else if (blendMode == BM_SUBTRACT)
			{
				glBlendEquationSeparateOES(GL_FUNC_REVERSE_SUBTRACT_OES, GL_FUNC_ADD_OES);
				glBlendFuncSeparateOES(GL_SRC_ALPHA, GL_ONE, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
			}
			else if (blendMode == BM_OVERWRITE)
			{
				glBlendEquationSeparateOES(GL_FUNC_ADD_OES, GL_FUNC_ADD_OES);
				glBlendFuncSeparateOES(GL_ONE, GL_ZERO, GL_ONE, GL_ZERO);
			}
			else
			{
				hlog::warn(logTag, "Trying to set unsupported blend mode!");
			}
		}
		else
#endif
		{
			OpenGLC_RenderSystem::_setDeviceBlendMode(blendMode);
		}
	}

}
#endif
