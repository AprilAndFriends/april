/// @file
/// @version 4.1
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifdef _EGL
#include <EGL/egl.h>

#include <hltypes/hltypesUtil.h>
#include <hltypes/hstring.h>
#include <hltypes/hlog.h>

#include "april.h"
#include "egl.h"

#define METERS_TO_INCHES_FACTOR (EGL_DISPLAY_SCALING * 0.0254f)

namespace april
{
	EglData* egl = NULL;

	EglData::EglData()
	{
		this->hWnd = NULL;
		this->display = NULL;
		this->config = NULL;
		this->surface = NULL;
		this->context = NULL;
		memset(this->pi32ConfigAttribs, 0, sizeof(EGLint) * 128);
		this->pi32ConfigAttribs[0] = EGL_BUFFER_SIZE;
		this->pi32ConfigAttribs[1] = 0;
		this->pi32ConfigAttribs[2] = EGL_SURFACE_TYPE;
		this->pi32ConfigAttribs[3] = EGL_WINDOW_BIT;
		this->pi32ConfigAttribs[4] = EGL_NONE;
	}

	EglData::~EglData()
	{
		this->destroy();
	}

	bool EglData::create()
	{
		if (this->display == NULL)
		{
			this->display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
			if (this->display == EGL_NO_DISPLAY)
			{
				hlog::error(logTag, "Can't get EGL Display! Error: " + hstr((int)eglGetError()));
				this->destroy();
				return false;
			}
			EGLint majorVersion;
			EGLint minorVersion;
			if (!eglInitialize(this->display, &majorVersion, &minorVersion))
			{
				hlog::error(logTag, "Can't initialize EGL! Error: " + hstr((int)eglGetError()));
				this->destroy();
				return false;
			}
			EGLint configs = 0;
			EGLBoolean result = eglGetConfigs(this->display, NULL, 0, &configs);
			if (!result || configs == 0)
			{
				hlog::error(logTag, "There are no EGL configs! Error: " + hstr((int)eglGetError()));
				this->destroy();
				return false;
			}
			if (april::rendersys->getName() == april::RenderSystemType::OpenGLES1.getName())
			{
				this->pi32ConfigAttribs[4] = EGL_RENDERABLE_TYPE;
				this->pi32ConfigAttribs[5] = EGL_OPENGL_ES_BIT;
				this->pi32ConfigAttribs[6] = EGL_NONE;
			}
			else if (april::rendersys->getName() == april::RenderSystemType::OpenGLES2.getName())
			{
				this->pi32ConfigAttribs[4] = EGL_RENDERABLE_TYPE;
				this->pi32ConfigAttribs[5] = EGL_OPENGL_ES2_BIT;
				this->pi32ConfigAttribs[6] = EGL_NONE;
			}
			result = eglChooseConfig(this->display, this->pi32ConfigAttribs, &this->config, 1, &configs);
			if (!result || configs == 0)
			{
				hlog::error(logTag, "Can't choose EGL config! Error: " + hstr((int)eglGetError()));
				this->destroy();
				return false;
			}
		}
		if (this->surface == NULL)
		{
			// hWnd is assigned outside of this code on some non-win32 platforms, so we have to use it.
#if defined(_WIN32) && !defined(_WINRT)
			this->hWnd = (EGLNativeWindowType)window->getBackendId();
#endif
			this->surface = eglCreateWindowSurface(this->display, this->config, this->hWnd, NULL);
			if (this->surface == NULL)
			{
				this->surface = eglCreateWindowSurface(this->display, this->config, NULL, NULL);
			}
		}
		if (this->surface == NULL)
		{
			hlog::error(logTag, "Can't create EGL window surface! Error: " + hstr((int)eglGetError()));
			return false;
		}
		if (this->context == NULL)
		{
			if (april::rendersys->getName() == april::RenderSystemType::OpenGLES2.getName())
			{
				if (!eglBindAPI(EGL_OPENGL_ES_API))
				{
					hlog::error(logTag, "Can't bind EGL OpenGLES API! Error: " + hstr((int)eglGetError()));
					return false;
				}
				EGLint contextAttributes[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
				this->context = eglCreateContext(this->display, this->config, NULL, contextAttributes);
			}
			else
			{
				this->context = eglCreateContext(this->display, this->config, NULL, NULL);
			}
			if (this->context == NULL)
			{
				hlog::error(logTag, "Can't create EGL context! Error: " + hstr((int)eglGetError()));
				return false;
			}
			if (!eglMakeCurrent(this->display, this->surface, this->surface, this->context))
			{
				hlog::error(logTag, "Can't set current EGL context! Error: " + hstr((int)eglGetError()));
				this->destroy();
				return false;
			}
			eglSwapInterval(this->display, 1);
		}
		return true;
	}

	bool EglData::destroy()
	{
		if (this->display != NULL)
		{
			eglMakeCurrent(this->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
		}
		if (this->context != NULL)
		{
			eglDestroyContext(this->display, this->context);
			this->context = NULL;
		}
		if (this->surface != NULL)
		{
			eglDestroySurface(this->display, this->surface);
			this->surface = NULL;
		}
		if (this->display != NULL)
		{
			eglTerminate(this->display);
			this->display = NULL;
		}
		return true;
	}

	void EglData::swapBuffers()
	{
		eglSwapBuffers(this->display, this->surface);
	}

}
#endif