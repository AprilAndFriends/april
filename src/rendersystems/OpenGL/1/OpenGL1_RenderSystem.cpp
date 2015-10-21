/// @file
/// @version 3.6
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifdef _OPENGL1
#define __HL_INCLUDE_PLATFORM_HEADERS
#include <hltypes/hplatform.h>
#if __APPLE__
#include <TargetConditionals.h>
#endif
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

#include <gtypes/Vector2.h>
#include <hltypes/hexception.h>
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
			wglDeleteContext(this->hRC);
			this->hRC = 0;
		}
		OpenGL_RenderSystem::_releaseWindow();
	}

	bool OpenGL1_RenderSystem::_initWin32(Window* window)
	{
		if (!OpenGL_RenderSystem::_initWin32(window))
		{
			return false;
		}
		PIXELFORMATDESCRIPTOR pfd;
		memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
		pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
		pfd.nVersion = 1;
		pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
		pfd.iPixelType = PFD_TYPE_RGBA;
		pfd.cColorBits = 24;
		pfd.cDepthBits = 16;
		pfd.dwLayerMask = PFD_MAIN_PLANE;
		GLuint pixelFormat = ChoosePixelFormat(this->hDC, &pfd);
		if (pixelFormat == 0)
		{
			hlog::error(logTag, "Can't find a suitable pixel format!");
			this->_releaseWindow();
			return false;
		}
		if (SetPixelFormat(this->hDC, pixelFormat, &pfd) == 0)
		{
			hlog::error(logTag, "Can't set the pixel format!");
			this->_releaseWindow();
			return false;
		}
		return true;
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
			hlog::error(logTag, "Can't create a GL rendering context!");
			this->_releaseWindow();
			return;
		}
		if (wglMakeCurrent(this->hDC, this->hRC) == 0)
		{
			hlog::error(logTag, "Can't activate the GL rendering context!");
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

	void OpenGL1_RenderSystem::_setupCaps()
	{
#ifdef _WIN32
		if (this->hRC == 0)
		{
			return;
		}
#endif
		if (this->caps.maxTextureSize == 0)
		{
			hstr extensions = (const char*)glGetString(GL_EXTENSIONS);
			if (extensions.contains("ARB_texture_non_power_of_two"))
			{
				this->caps.npotTexturesLimited = true;
				// this isn't 100% sure, but it's a pretty good indicator that NPOT textures should be fully supported on this hardware
				if (extensions.contains("ARB_fragment_program"))
				{
					int value = 0;
					glGetIntegerv(GL_MAX_TEXTURE_SIZE, &value);
					if (value >= 8192)
					{
						this->caps.npotTextures = true;
					}
				}
			}
		}
		OpenGL_RenderSystem::_setupCaps();
	}

	Texture* OpenGL1_RenderSystem::_createTexture(bool fromResource)
	{
		return new OpenGL1_Texture(fromResource);
	}

	void OpenGL1_RenderSystem::_setVertexPointer(int stride, const void* pointer)
	{
		if (this->deviceState.strideVertex != stride || this->deviceState.pointerVertex != pointer)
		{
			this->deviceState.strideVertex = stride;
			this->deviceState.pointerVertex = pointer;
			glVertexPointer(3, GL_FLOAT, stride, pointer);
		}
	}

	void OpenGL1_RenderSystem::_setTextureBlendMode(BlendMode textureBlendMode)
	{
		// TODO - is there a way to make this work on Win32?
#ifndef _WIN32
		// TODO - refactor
		static int blendSeparationSupported = -1;
		if (blendSeparationSupported == -1)
		{
			// determine if blend separation is possible on first call to this function
			hstr extensions = (const char*)glGetString(GL_EXTENSIONS);
			blendSeparationSupported = extensions.contains("EXT_blend_equation_separate") && extensions.contains("EXT_blend_func_separate");
		}
		if (blendSeparationSupported)
		{
			// blending for the new generations
			switch (textureBlendMode)
			{
			case BM_DEFAULT:
			case BM_ALPHA:
				glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
				glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
				break;
			case BM_ADD:
				glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
				glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
				break;
			case BM_SUBTRACT:
				glBlendEquationSeparate(GL_FUNC_REVERSE_SUBTRACT, GL_FUNC_ADD);
				glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
				break;
			case BM_OVERWRITE:
				glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
				glBlendFuncSeparate(GL_ONE, GL_ZERO, GL_ONE, GL_ZERO);
				break;
			default:
				hlog::warn(logTag, "Trying to set unsupported blend mode!");
				break;
			}
		}
		else
#endif
		{
			// old-school blending mode for your dad
			OpenGL_RenderSystem::_setTextureBlendMode(textureBlendMode);
		}
	}
	
	void OpenGL1_RenderSystem::_setDepthBuffer(bool enabled, bool writeEnabled)
	{
		OpenGL_RenderSystem::_setDepthBuffer(enabled, writeEnabled);
		enabled ? glEnable(GL_ALPHA_TEST) : glDisable(GL_ALPHA_TEST);
		glAlphaFunc(GL_GREATER, 0.0f);
	}

}
#endif
