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
#include "OpenGLES_RenderSystem.h"
#include "Window.h"

namespace april
{
	OpenGLES_RenderSystem::OpenGLES_RenderSystem() : OpenGL_RenderSystem()
	{
#ifdef _WIN32
		this->eglDisplay = 0;
		this->eglConfig	= 0;
		this->eglSurface = 0;
		this->eglContext = 0;
		memset(this->pi32ConfigAttribs, 0, sizeof(EGLint) * 128);
		this->pi32ConfigAttribs[0] = EGL_RED_SIZE;
		this->pi32ConfigAttribs[1] = 8;
		this->pi32ConfigAttribs[2] = EGL_GREEN_SIZE;
		this->pi32ConfigAttribs[3] = 8;
		this->pi32ConfigAttribs[4] = EGL_BLUE_SIZE;
		this->pi32ConfigAttribs[5] = 8;
		this->pi32ConfigAttribs[6] = EGL_ALPHA_SIZE;
		this->pi32ConfigAttribs[7] = 0;
		this->pi32ConfigAttribs[8] = EGL_SURFACE_TYPE;
		this->pi32ConfigAttribs[9] = EGL_WINDOW_BIT;
		this->pi32ConfigAttribs[10] = EGL_NONE;
#endif
	}

	OpenGLES_RenderSystem::~OpenGLES_RenderSystem()
	{
	}

#ifdef _WIN32
	void OpenGLES_RenderSystem::_releaseWindow()
	{
		if (this->eglDisplay != 0)
		{
			eglMakeCurrent(this->eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
			eglTerminate(this->eglDisplay);
			this->eglDisplay = 0;
		}
		OpenGL_RenderSystem::_releaseWindow();
	}
#endif

	void OpenGLES_RenderSystem::assignWindow(Window* window)
	{
#ifdef _WIN32
		if (!this->_initWin32(window))
		{
			return;
		}
		this->eglDisplay = eglGetDisplay((NativeDisplayType)this->hDC);
		if (this->eglDisplay == EGL_NO_DISPLAY)
		{
			 this->eglDisplay = eglGetDisplay((NativeDisplayType)EGL_DEFAULT_DISPLAY);
		}
		if (this->eglDisplay == EGL_NO_DISPLAY)
		{
			hlog::error(april::logTag, "Can't get EGL display!");
			this->_releaseWindow();
			return;
		}
		EGLint majorVersion;
		EGLint minorVersion;
		if (!eglInitialize(this->eglDisplay, &majorVersion, &minorVersion))
		{
			hlog::error(april::logTag, "Can't initialize EGL!");
			this->_releaseWindow();
			return;
		}
		EGLint configs;
		EGLBoolean result = eglChooseConfig(this->eglDisplay, this->pi32ConfigAttribs, &this->eglConfig, 1, &configs);
		if (!result || configs == 0)
		{
			hlog::error(april::logTag, "Can't choose EGL config!");
			this->_releaseWindow();
			return;
		}
		this->eglSurface = eglCreateWindowSurface(this->eglDisplay, this->eglConfig, (NativeWindowType)this->hWnd, NULL);
		if (eglGetError() != EGL_SUCCESS)
		{
			hlog::error(april::logTag, "Can't create window surface!");
			this->_releaseWindow();
			return;
		}
		this->eglContext = eglCreateContext(this->eglDisplay, this->eglConfig, EGL_NO_CONTEXT, NULL);
		if (eglGetError() != EGL_SUCCESS)
		{
			hlog::error(april::logTag, "Can't create EGL context!");
			this->_releaseWindow();
			return;
		}
		eglMakeCurrent(this->eglDisplay, this->eglSurface, this->eglSurface, this->eglContext);
		if (eglGetError() != EGL_SUCCESS)
		{
			hlog::error(april::logTag, "Can't make context current!");
			this->_releaseWindow();
			return;
		}
#endif
		OpenGL_RenderSystem::assignWindow(window);
	}

	int OpenGLES_RenderSystem::getMaxTextureSize()
	{
#ifdef _WIN32
		if (this->eglDisplay == 0)
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
