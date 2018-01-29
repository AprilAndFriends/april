/// @file
/// @version 5.0
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

#ifdef BUILDING_APRIL
#include "april.h"
#include "RenderSystem.h"
#include "Window.h"
#else
#include <april/april.h>
#include <april/RenderSystem.h>
#include <april/Window.h>
#endif

#include "egl.h"

namespace april
{
	extern hstr logTag;

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
				hlog::errorf(logTag, "Can't get EGL Display! Error: %X", eglGetError());
				this->destroy();
				return false;
			}
			EGLint majorVersion;
			EGLint minorVersion;
			if (!eglInitialize(this->display, &majorVersion, &minorVersion))
			{
				hlog::errorf(logTag, "Can't initialize EGL! Error: %X", eglGetError());
				this->destroy();
				return false;
			}
			hstr renderSystemName = april::rendersys->getName();
			if (renderSystemName == april::RenderSystemType::OpenGLES2.getName())
			{
				if (!eglBindAPI(EGL_OPENGL_ES_API))
				{
					hlog::errorf(logTag, "Can't bind EGL OpenGLES API! Error: %X", eglGetError());
					return false;
				}
			}
			EGLint nConfigs = 0;
			EGLBoolean result = eglGetConfigs(this->display, NULL, 0, &nConfigs);
			if (!result || nConfigs == 0)
			{
				hlog::errorf(logTag, "There are no EGL configs! Error: %X", eglGetError());
				this->destroy();
				return false;
			}
			if (renderSystemName == april::RenderSystemType::OpenGLES1.getName())
			{
				this->pi32ConfigAttribs[4] = EGL_RENDERABLE_TYPE;
				this->pi32ConfigAttribs[5] = EGL_OPENGL_ES_BIT;
				this->pi32ConfigAttribs[6] = EGL_NONE;
			}
			else if (renderSystemName == april::RenderSystemType::OpenGLES2.getName())
			{
				this->pi32ConfigAttribs[4] = EGL_RENDERABLE_TYPE;
				this->pi32ConfigAttribs[5] = EGL_OPENGL_ES2_BIT;
				this->pi32ConfigAttribs[6] = EGL_NONE;
			}
            EGLConfig* configs = new EGLConfig[nConfigs];
			result = eglChooseConfig(this->display, this->pi32ConfigAttribs, configs, nConfigs, &nConfigs);
			if (!result || nConfigs == 0)
			{
				hlog::errorf(logTag, "Can't choose EGL config! Error: %X", eglGetError());
				this->destroy();
				return false;
			}
            // prefer RGB888, Android chooses RGB565 by default
            this->config = configs[0];
			EGLint size[3] = { 0, 0, 0 };
			for_iter (i, 0, nConfigs)
            {
				memset(&size, 0, 3 * sizeof(EGLint));
#if !defined(_WIN32) || defined(_WINRT)
                // alpha == 0
                if (eglGetConfigAttrib(this->display, configs[i], EGL_ALPHA_SIZE, &size[0]) && size[0] == 0)
#endif
                {
					this->config = configs[i];
					// RGB == 888
					if (eglGetConfigAttrib(this->display, configs[i], EGL_RED_SIZE, &size[0]) &&
						eglGetConfigAttrib(this->display, configs[i], EGL_GREEN_SIZE, &size[1]) &&
						eglGetConfigAttrib(this->display, configs[i], EGL_BLUE_SIZE, &size[2]))
					{
						if (size[0] == 8 && size[1] == 8 && size[2] == 8)
						{
							hlog::write(logTag, "Found RGB888 EGL Config!");
							break;
						}
					}
				}
            }
            delete [] configs;
        }
        if (this->surface == NULL)
		{
			// hWnd is assigned outside of this code on some non-win32 platforms, so we have to use it.
#if defined(_WIN32) && !defined(_WINRT)
			this->hWnd = (EGLNativeWindowType)april::window->getBackendId();
#endif
			EGLint surfaceAttributes[1] = { EGL_NONE };
			this->surface = eglCreateWindowSurface(this->display, this->config, this->hWnd, surfaceAttributes);
			if (this->surface == NULL)
			{
				hlog::writef(logTag, "Can't create EGL window surface with native window, attempting without. Error: %X", eglGetError());
				this->surface = eglCreateWindowSurface(this->display, this->config, NULL, surfaceAttributes);
			}
		}
		if (this->surface == NULL)
		{
			hlog::errorf(logTag, "Can't create EGL window surface! Error: %X", eglGetError());
			return false;
		}
		if (this->context == NULL)
		{
			if (april::rendersys->getName() == april::RenderSystemType::OpenGLES2.getName())
			{
				EGLint contextAttributes[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
				this->context = eglCreateContext(this->display, this->config, NULL, contextAttributes);
			}
			else
			{
				this->context = eglCreateContext(this->display, this->config, NULL, NULL);
			}
			if (this->context == NULL)
			{
				hlog::errorf(logTag, "Can't create EGL context! Error: %X", eglGetError());
				return false;
			}
			if (!eglMakeCurrent(this->display, this->surface, this->surface, this->context))
			{
				hlog::errorf(logTag, "Can't set current EGL context! Error: %X", eglGetError());
				this->destroy();
				return false;
			}
			EGLBoolean result = EGL_FALSE;
			april::RenderSystem::Options options = april::rendersys->getOptions();
			if (options.vSync)
			{
				if (options.tripleBuffering)
				{
					result = eglSwapInterval(this->display, 2);
				}
				else
				{
					result = eglSwapInterval(this->display, 1);
				}
			}
			else
			{
				result = eglSwapInterval(this->display, 0);
			}
			if (!result)
			{
				eglSwapInterval(this->display, 1);
			}
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
