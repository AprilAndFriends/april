/// @file
/// @author  Boris Mikic
/// @version 3.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

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
	EglData egl;

	EglData::EglData()
	{
		this->display = NULL;
		this->config = NULL;
		this->surface = NULL;
		this->context = NULL;
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
	}

	EglData::~EglData()
	{
	}

	bool EglData::init()
	{
		if (this->display == NULL)
		{
			this->display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
			if (this->display == EGL_NO_DISPLAY)
			{
				hlog::error(april::logTag, "Can't get EGL Display!");
				this->destroy();
				return false;
			}
			EGLint majorVersion;
			EGLint minorVersion;
			if (!eglInitialize(this->display, &majorVersion, &minorVersion))
			{
				hlog::error(april::logTag, "Can't initialize EGL!");
				this->destroy();
				return false;
			}
			EGLint configs = 0;
			eglGetConfigs(this->display, NULL, 0, &configs);
			if (configs == 0)
			{
				hlog::error(april::logTag, "There are no EGL configs!");
				this->destroy();
				return false;
			}
			EGLBoolean result = eglChooseConfig(this->display, this->pi32ConfigAttribs, &this->config, 1, &configs);
			if (!result || configs == 0)
			{
				hlog::error(april::logTag, "Can't choose EGL config!");
				this->destroy();
				return false;
			}
			eglSwapInterval(this->display, 1);
		}
		return true;
	}

	bool EglData::create()
	{
		return this->create(NULL, NULL);
	}

	bool EglData::create(int* width, int* height)
	{
		this->init(); // just in case somebody didn't call init before
		if (this->surface == NULL)
		{
			this->surface = eglCreateWindowSurface(this->display, this->config, this->hWnd, NULL);
		}
		if (this->context == NULL)
		{
			this->context = eglCreateContext(this->display, this->config, NULL, NULL);
			if (!eglMakeCurrent(this->display, this->surface, this->surface, this->context))
			{
				hlog::error(april::logTag, "Can't set current EGL context!");
				this->destroy();
				return false;
			}
		}
		if (width != NULL && height != NULL)
		{
			this->getSystemParameters(width, height, NULL);
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

	bool EglData::getSystemParameters(int* width, int* height, int* dpi)
	{
		if (this->display == NULL || this->surface == NULL)
		{
			*width = 0;
			*height = 0;
			if (dpi != NULL)
			{
				*dpi = 0;
			}
			return false;
		}
		if (!eglQuerySurface(this->display, this->surface, EGL_WIDTH, (EGLint*)width))
		{
			*width = 0;
		}
		if (!eglQuerySurface(this->display, this->surface, EGL_HEIGHT, (EGLint*)height))
		{
			*height = 0;
		}
		if (dpi != NULL)
		{
			int dpiX = 0;
			int dpiY = 0;
			eglQuerySurface(this->display, this->surface, EGL_HORIZONTAL_RESOLUTION, (EGLint*)&dpiX);
			eglQuerySurface(this->display, this->surface, EGL_VERTICAL_RESOLUTION, (EGLint*)&dpiY);
			*dpi = (int)(hhypot(dpiX / METERS_TO_INCHES_FACTOR, dpiY / METERS_TO_INCHES_FACTOR) * 0.5f);
		}
		return true;
	}

}
#endif