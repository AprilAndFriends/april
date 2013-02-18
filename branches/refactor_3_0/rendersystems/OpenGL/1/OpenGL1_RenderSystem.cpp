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

#ifdef _OPENGL1
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
#include "OpenGL1_RenderSystem.h"
#include "OpenGL1_Texture.h"
#include "Platform.h"
#include "Timer.h"
#include "Window.h"

#define MAX_VERTEX_COUNT 65536

namespace april
{
	OpenGL1_RenderSystem::OpenGL1_RenderSystem() : OpenGL_RenderSystem()
	{
		this->name = APRIL_RS_OPENGL1;
#ifdef _WIN32
		this->hRC = 0;
#endif
	}

	OpenGL1_RenderSystem::~OpenGL1_RenderSystem()
	{
		this->destroy();
	}

#ifdef _WIN32
	void OpenGL1_RenderSystem::_releaseWindow()
	{
		if (this->hRC != 0)
		{
			wglMakeCurrent(NULL, NULL);
			wglDeleteContext(hRC);
			this->hRC = 0;
		}
		OpenGL_RenderSystem::_releaseWindow();
	}
#endif

	void OpenGL1_RenderSystem::assignWindow(Window* window)
	{
#ifdef _WIN32
		if (!this->_initWin32(window))
		{
			return;
		}
		this->hRC = wglCreateContext(this->hDC);
		if (this->hRC == 0)
		{
			hlog::error(april::logTag, "Can't create a GL rendering context!");
			this->_releaseWindow();
			return;
		}
		if (wglMakeCurrent(this->hDC, this->hRC) == 0)
		{
			hlog::error(april::logTag, "Can't activate the GL rendering context!");
			this->_releaseWindow();
			return;
		}
#endif
		OpenGL_RenderSystem::assignWindow(window);
	}

	void OpenGL1_RenderSystem::_setupDefaultParameters()
	{
		OpenGL_RenderSystem::_setupDefaultParameters();
		// pixel data
		glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
		glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
		glPixelStorei(GL_UNPACK_SWAP_BYTES, GL_FALSE);
	}

	int OpenGL1_RenderSystem::getMaxTextureSize()
	{
#ifdef _WIN32
		if (this->hRC == 0)
		{
			return 0;
		}
#endif
		int max;
		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max);
		return max;
	}

	void OpenGL1_RenderSystem::_setTextureBlendMode(BlendMode textureBlendMode)
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
			OpenGL_RenderSystem::_setTextureBlendMode(textureBlendMode);
		}
	}
	
	Texture* OpenGL1_RenderSystem::_createTexture(chstr filename)
	{
		return new OpenGL1_Texture(filename);
	}

	Texture* OpenGL1_RenderSystem::_createTexture(int w, int h, unsigned char* rgba)
	{
		return new OpenGL1_Texture(w, h, rgba);
	}
	
	Texture* OpenGL1_RenderSystem::_createTexture(int w, int h, Texture::Format format, Texture::Type type, Color color)
	{
		return new OpenGL1_Texture(w, h, format, type, color);
	}

	void OpenGL1_RenderSystem::_setVertexPointer(int stride, const void* pointer)
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
