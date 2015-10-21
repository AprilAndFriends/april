/// @file
/// @version 3.6
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifdef _OPENGLES

#define __HL_INCLUDE_PLATFORM_HEADERS
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
#if defined(_WIN32) && !defined(_WINRT)
		if (!this->_initWin32(window))
		{
			return;
		}
#endif
		OpenGL_RenderSystem::assignWindow(window);
	}

	void OpenGLES_RenderSystem::_setupDefaultParameters()
	{
		OpenGL_RenderSystem::_setupDefaultParameters();
		// TODO
	}

	void OpenGLES_RenderSystem::_applyStateChanges()
	{
		OpenGL_RenderSystem::_applyStateChanges();
		// TODO
	}

	void OpenGLES_RenderSystem::_setupCaps()
	{
#ifdef _EGL
		if (april::egl->display == NULL)
		{
			return;
		}
#endif
		if (this->caps.maxTextureSize == 0)
		{
			hstr extensions = (const char*)glGetString(GL_EXTENSIONS);
#ifndef _WINRT
			this->caps.npotTexturesLimited = (extensions.contains("IMG_texture_npot") || extensions.contains("APPLE_texture_2D_limited_npot"));
#else
			this->caps.npotTexturesLimited = true;
#endif
			this->caps.npotTextures = (extensions.contains("OES_texture_npot") || extensions.contains("ARB_texture_non_power_of_two"));
		}
#ifdef _ANDROID // Android has problems with alpha textures in some implementations
		this->caps.textureFormats /= Image::FORMAT_ALPHA;
		this->caps.textureFormats /= Image::FORMAT_GRAYSCALE;
#endif
		return OpenGL_RenderSystem::_setupCaps();
	}

	void OpenGLES_RenderSystem::_setTextureBlendMode(BlendMode textureBlendMode)
	{
		// TODO
	}

	void OpenGLES_RenderSystem::_setTextureColorMode(ColorMode textureColorMode, float factor)
	{
		// TODO
	}

	void OpenGLES_RenderSystem::_loadIdentityMatrix()
	{
		// TODO
	}
	
	void OpenGLES_RenderSystem::_setMatrixMode(unsigned int mode)
	{
		// TODO
	}

	void OpenGLES_RenderSystem::_setVertexPointer(int stride, const void* pointer)
	{
		// TODO
		if (this->deviceState.strideVertex != stride || this->deviceState.pointerVertex != pointer)
		{
			this->deviceState.strideVertex = stride;
			this->deviceState.pointerVertex = pointer;
			//glVertexAttribPointer(_positionSlot, 3, GL_FLOAT, GL_FALSE, stride, pointer);
		}
	}

	void OpenGLES_RenderSystem::_setTexCoordPointer(int stride, const void *pointer)
	{
		// TODO
	}

	void OpenGLES_RenderSystem::_setColorPointer(int stride, const void *pointer)
	{
		// TODO
	}

}
#endif
