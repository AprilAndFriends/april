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

#ifdef _OPENGLES1
#include <hltypes/hplatform.h>
// TODO - should be cleaned up a bit
#if __APPLE__
#include <TargetConditionals.h>
#endif
#if TARGET_OS_IPHONE
#ifdef _OPENGLES1
	#include <OpenGLES/ES1/gl.h>
	#include <OpenGLES/ES1/glext.h>
#elif defined(_OPENGLES2)
	#include <OpenGLES/ES2/gl.h>
	#include <OpenGLES/ES2/glext.h>
	extern GLint _positionSlot;
#endif
#elif defined(_OPENGLES)
#include <GLES/gl.h>
#ifdef _ANDROID
#define GL_GLEXT_PROTOTYPES
#include <GLES/glext.h>
#else
#include <EGL/egl.h>
#endif
#else
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifndef __APPLE__
#include <gl/GL.h>
#define GL_GLEXT_PROTOTYPES
#include <gl/glext.h>
#else
#include <OpenGL/gl.h>
#endif
#endif

#include <gtypes/Vector2.h>
#include <hltypes/exception.h>
#include <hltypes/hlog.h>
#include <hltypes/hltypesUtil.h>

#include "april.h"
#include "Image.h"
#include "Keys.h"
#include "OpenGLES1_RenderSystem.h"
#include "OpenGLES1_Texture.h"
#include "Platform.h"
#include "Timer.h"
#include "Window.h"

#define MAX_VERTEX_COUNT 65536

namespace april
{
#ifdef _WIN32 // if _WIN32
	static HWND hWnd = 0;
	HDC hDC = 0;
#ifndef _OPENGLES // if _WIN32 && !GLES
	static HGLRC hRC = 0;
#else
	static EGLDisplay eglDisplay = 0;
	static EGLConfig eglConfig	= 0;
	static EGLSurface eglSurface = 0;
	static EGLContext eglContext = 0;
	static EGLint pi32ConfigAttribs[128] = {EGL_RED_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_BLUE_SIZE, 8, EGL_ALPHA_SIZE, 0, EGL_SURFACE_TYPE, EGL_WINDOW_BIT, EGL_NONE};
#endif
#endif

	// TODO - refactor
	int OpenGLES1_RenderSystem::_getMaxTextureSize()
	{
#ifdef _WIN32
#ifndef _OPENGLES
		if (hRC == 0)
#else
		if (eglDisplay == 0)
#endif
		{
			return 0;
		}
#endif
		int max;
		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max);
		return max;
	}

	OpenGLES1_RenderSystem::OpenGLES1_RenderSystem() : OpenGLES_RenderSystem()
	{
		this->name = APRIL_RS_OPENGLES1;
	}

	OpenGLES1_RenderSystem::~OpenGLES1_RenderSystem()
	{
		this->destroy();
	}

	bool OpenGLES1_RenderSystem::destroy()
	{
		if (!OpenGLES_RenderSystem::destroy())
		{
			return false;
		}
		// TODO - should maybe be refactored
#ifdef _WIN32
		this->_releaseWindow();
#endif
		return true;
	}

#ifdef _WIN32
	void OpenGLES1_RenderSystem::_releaseWindow()
	{
#ifndef _OPENGLES
		if (hRC != 0)
		{
			wglMakeCurrent(NULL, NULL);
			wglDeleteContext(hRC);
			hRC = 0;
		}
#else
		if (eglDisplay != 0)
		{
			eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
			eglTerminate(eglDisplay);
			eglDisplay = 0;
		}
#endif
		if (hDC != 0)
		{
			ReleaseDC(hWnd, hDC);
			hDC = 0;
		}
	}
#endif

	void OpenGLES1_RenderSystem::assignWindow(Window* window)
	{
#ifdef _WIN32
		hWnd = (HWND)april::window->getBackendId();
		PIXELFORMATDESCRIPTOR pfd;
		memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
		pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
		pfd.nVersion = 1;
		pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL |	PFD_DOUBLEBUFFER;
		pfd.iPixelType = PFD_TYPE_RGBA;
		pfd.cColorBits = 24;
		pfd.cStencilBits = 16;
		pfd.dwLayerMask = PFD_MAIN_PLANE;

		hDC = GetDC(hWnd);
		if (hDC == 0)
		{
			hlog::error(april::logTag, "Can't create a GL device context!");
			return;
		}
		GLuint pixelFormat = ChoosePixelFormat(hDC, &pfd);
		if (pixelFormat == 0)
		{
			hlog::error(april::logTag, "Can't find a suitable pixel format!");
			this->_releaseWindow();
			return;
		}
		if (SetPixelFormat(hDC, pixelFormat, &pfd) == 0)
		{
			hlog::error(april::logTag, "Can't set the pixel format!");
			this->_releaseWindow();
			return;
		}
#ifndef _OPENGLES
		hRC = wglCreateContext(hDC);
		if (hRC == 0)
		{
			hlog::error(april::logTag, "Can't create a GL rendering context!");
			this->_releaseWindow();
			return;
		}
		if (wglMakeCurrent(hDC, hRC) == 0)
		{
			hlog::error(april::logTag, "Can't activate the GL rendering context!");
			this->_releaseWindow();
			return;
		}
#else
		eglDisplay = eglGetDisplay((NativeDisplayType)hDC);
		if (eglDisplay == EGL_NO_DISPLAY)
		{
			 eglDisplay = eglGetDisplay((NativeDisplayType)EGL_DEFAULT_DISPLAY);
		}
		if (eglDisplay == EGL_NO_DISPLAY)
		{
			hlog::error(april::logTag, "Can't get EGL display!");
			this->_releaseWindow();
			return;
		}
		EGLint majorVersion;
		EGLint minorVersion;
		if (!eglInitialize(eglDisplay, &majorVersion, &minorVersion))
		{
			hlog::error(april::logTag, "Can't initialize EGL!");
			this->_releaseWindow();
			return;
		}
		EGLint configs;
		EGLBoolean result = eglChooseConfig(eglDisplay, pi32ConfigAttribs, &eglConfig, 1, &configs);
		if (!result || configs == 0)
		{
			hlog::error(april::logTag, "Can't choose EGL config!");
			this->_releaseWindow();
			return;
		}

		eglSurface = eglCreateWindowSurface(eglDisplay, eglConfig, (NativeWindowType)hWnd, NULL);
		if (eglGetError() != EGL_SUCCESS)
		{
			hlog::error(april::logTag, "Can't create window surface!");
			this->_releaseWindow();
			return;
		}
		eglContext = eglCreateContext(eglDisplay, eglConfig, EGL_NO_CONTEXT, NULL);
		if (eglGetError() != EGL_SUCCESS)
		{
			hlog::error(april::logTag, "Can't create EGL context!");
			this->_releaseWindow();
			return;
		}
		eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext);
		if (eglGetError() != EGL_SUCCESS)
		{
			hlog::error(april::logTag, "Can't make context current!");
			this->_releaseWindow();
			return;
		}
#endif
#endif
		this->_setupDefaultParameters();
		this->setMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		this->setMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		this->orthoProjection.setSize((float)april::window->getWidth(), (float)april::window->getHeight());
	}

	void OpenGLES1_RenderSystem::_setupDefaultParameters()
	{
		OpenGLES_RenderSystem::_setupDefaultParameters();
		// pixel data
#ifndef _OPENGLES
		glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
		glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
		glPixelStorei(GL_UNPACK_SWAP_BYTES, GL_FALSE);
#endif
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		// other
		if (this->options.contains("zbuffer"))
		{
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_LEQUAL);
		}
		this->_setClientState(GL_TEXTURE_COORD_ARRAY, this->deviceState.textureCoordinatesEnabled);
		this->_setClientState(GL_COLOR_ARRAY, this->deviceState.colorEnabled);
		glColor4f(this->deviceState.systemColor.r_f(), this->deviceState.systemColor.g_f(), this->deviceState.systemColor.b_f(), this->deviceState.systemColor.a_f());
		glBindTexture(GL_TEXTURE_2D, this->deviceState.textureId);
		this->state.textureFilter = april::Texture::FILTER_LINEAR;
		this->state.textureAddressMode = april::Texture::ADDRESS_WRAP;
		this->state.blendMode = april::DEFAULT;
		this->state.colorMode = april::NORMAL;
	}

	void OpenGLES1_RenderSystem::_setTextureBlendMode(BlendMode textureBlendMode)
	{
		// TODO - is there a way to make this work on Win32?
#ifndef _WIN32
		static int blendSeparationSupported = -1;
		if (blendSeparationSupported == -1)
		{
			// determine if blend separation is possible on first call to this function
			hstr extensions = (const char*)glGetString(GL_EXTENSIONS);
#ifdef _OPENGLES
			blendSeparationSupported = extensions.contains("OES_blend_equation_separate") && extensions.contains("OES_blend_func_separate");
#else
			blendSeparationSupported = extensions.contains("GL_EXT_blend_equation_separate") && extensions.contains("GL_EXT_blend_func_separate");
#endif
		}
		if (blendSeparationSupported)
		{
			// blending for the new generations
			switch (textureBlendMode)
			{
			case DEFAULT:
			case ALPHA_BLEND:
#ifdef _OPENGLES1
				glBlendEquationSeparateOES(GL_FUNC_ADD_OES, GL_FUNC_ADD_OES);
				glBlendFuncSeparateOES(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
#else
				glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
				glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
#endif
				break;
			case ADD:
#ifdef _OPENGLES1
				glBlendEquationSeparateOES(GL_FUNC_ADD_OES, GL_FUNC_ADD_OES);
				glBlendFuncSeparateOES(GL_SRC_ALPHA, GL_ONE, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
#else
				glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
				glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
#endif
				break;
			case SUBTRACT:
#ifdef _OPENGLES1
				glBlendEquationSeparateOES(GL_FUNC_REVERSE_SUBTRACT_OES, GL_FUNC_ADD_OES);
				glBlendFuncSeparateOES(GL_SRC_ALPHA, GL_ONE, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
					
#else
				glBlendEquationSeparate(GL_FUNC_REVERSE_SUBTRACT, GL_FUNC_ADD);
				glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
#endif
				break;
			case OVERWRITE:
#ifdef _OPENGLES1
				glBlendEquationSeparateOES(GL_FUNC_ADD_OES, GL_FUNC_ADD_OES);
				glBlendFuncSeparateOES(GL_ONE, GL_ZERO, GL_ONE, GL_ZERO);				
#else
				glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
				glBlendFuncSeparate(GL_ONE, GL_ZERO, GL_ONE, GL_ZERO);
#endif
				break;
			default:
				hlog::warn(april::logTag, "Trying to set unsupported blend mode!");
				break;
			}
		}
		else
#endif
		{
			OpenGLES_RenderSystem::_setTextureBlendMode(textureBlendMode);
		}
	}
	
	Texture* OpenGLES1_RenderSystem::_createTexture(chstr filename)
	{
		return new OpenGLES1_Texture(filename);
	}

	Texture* OpenGLES1_RenderSystem::createTexture(int w, int h, unsigned char* rgba)
	{
		return new OpenGLES1_Texture(w, h, rgba);
	}
	
	Texture* OpenGLES1_RenderSystem::createTexture(int w, int h, Texture::Format format, Texture::Type type, Color color)
	{
		return new OpenGLES1_Texture(w, h, format, type, color);
	}

	void OpenGLES1_RenderSystem::_setVertexPointer(int stride, const void* pointer)
	{
		if (this->deviceState.strideVertex != stride || this->deviceState.pointerVertex != pointer)
		{
			this->deviceState.strideVertex = stride;
			this->deviceState.pointerVertex = pointer;
#ifdef _OPENGLES2
			glVertexAttribPointer(_positionSlot, 3, GL_FLOAT, GL_FALSE, stride, pointer);
#else
			glVertexPointer(3, GL_FLOAT, stride, pointer);
#endif
		}
	}
	
}

#endif
