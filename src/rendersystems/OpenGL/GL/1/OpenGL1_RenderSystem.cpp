/// @file
/// @version 5.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifdef _OPENGL1
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
	OpenGL1_RenderSystem::OpenGL1_RenderSystem() : OpenGL_RenderSystem(), deviceState_matrixMode(0)
	{
		this->name = april::RenderSystemType::OpenGL1.getName();
#if defined(_WIN32) && !defined(_WINRT)
		this->hRC = 0;
#endif
	}

	OpenGL1_RenderSystem::~OpenGL1_RenderSystem()
	{
		this->destroy();
	}

	void OpenGL1_RenderSystem::_deviceInit()
	{
		OpenGL_RenderSystem::_deviceInit();
		this->deviceState_color = april::Color::White;
		this->deviceState_matrixMode = 0;
#if defined(_WIN32) && !defined(_WINRT)
		this->hRC = 0;
#endif
	}

#if defined(_WIN32) && !defined(_WINRT)
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
		this->hRC = wglCreateContext(this->hDC);
		if (this->hRC == 0)
		{
			hlog::error(logTag, "Can't create a GL rendering context!");
			this->_releaseWindow();
			return false;
		}
		if (wglMakeCurrent(this->hDC, this->hRC) == 0)
		{
			hlog::error(logTag, "Can't activate the GL rendering context!");
			this->_releaseWindow();
			return false;
		}
		return true;
	}
#endif

	void OpenGL1_RenderSystem::_deviceSetupCaps()
	{
#if defined(_WIN32) && !defined(_WINRT)
		if (this->hRC == 0)
		{
			return;
		}
#endif
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
		// TODO - is there a way to make this work on Win32?
#ifndef _WIN32
		this->blendSeparationSupported = extensions.contains("EXT_blend_equation_separate") && extensions.contains("EXT_blend_func_separate");
#endif
		OpenGL_RenderSystem::_deviceSetupCaps();
	}

	void OpenGL1_RenderSystem::_deviceSetup()
	{
		glEnableClientState(GL_VERTEX_ARRAY);
		glAlphaFunc(GL_GREATER, 0.0f);
		glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
		glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
		glPixelStorei(GL_UNPACK_SWAP_BYTES, GL_FALSE);
		OpenGL_RenderSystem::_deviceSetup();
		this->_setDeviceColor(this->deviceState_color, true);
		this->_setDeviceMatrixMode(GL_MODELVIEW, true);
	}

	Texture* OpenGL1_RenderSystem::_deviceCreateTexture(bool fromResource)
	{
		return new OpenGL1_Texture(fromResource);
	}

	void OpenGL1_RenderSystem::_setDeviceModelviewMatrix(const gmat4& matrix)
	{
		this->_setDeviceMatrixMode(GL_MODELVIEW);
		glLoadMatrixf(matrix.data);
	}

	void OpenGL1_RenderSystem::_setDeviceProjectionMatrix(const gmat4& matrix)
	{
		this->_setDeviceMatrixMode(GL_PROJECTION);
		glLoadMatrixf(matrix.data);
	}

	void OpenGL1_RenderSystem::_setDeviceDepthBuffer(bool enabled, bool writeEnabled)
	{
		OpenGL_RenderSystem::_setDeviceDepthBuffer(enabled, writeEnabled);
		enabled ? glEnable(GL_ALPHA_TEST) : glDisable(GL_ALPHA_TEST);
	}

	void OpenGL1_RenderSystem::_setDeviceTexture(Texture* texture)
	{
		OpenGL_RenderSystem::_setDeviceTexture(texture);
		if (texture != NULL)
		{
			// software NPOT handling if NPOT is not supported by driver
			Caps caps = this->getCaps();
			if (!caps.npotTexturesLimited && !caps.npotTextures)
			{
				OpenGL1_Texture* currentTexture = (OpenGL1_Texture*)texture;
				this->_setDeviceMatrixMode(GL_TEXTURE);
				if (currentTexture->effectiveWidth != 1.0f || currentTexture->effectiveHeight != 1.0f)
				{
					static gmat4 matrix;
					matrix.setScale(currentTexture->effectiveWidth, currentTexture->effectiveHeight, 1.0f);
					glLoadMatrixf(matrix.data);
	}
				else
				{
					static gmat4 matrix;
					glLoadMatrixf(matrix.data);
				}
			}
		}
	}

	void OpenGL1_RenderSystem::_setDeviceBlendMode(const BlendMode& blendMode)
	{
#ifndef _WIN32
		if (this->blendSeparationSupported)
		{
			// blending for the new generations
			if (blendMode == BlendMode::Alpha)
			{
				glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
				glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
			}
			else if (blendMode == BlendMode::Add)
			{
				glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
				glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
			}
			else if (blendMode == BlendMode::Subtract)
			{
				glBlendEquationSeparate(GL_FUNC_REVERSE_SUBTRACT, GL_FUNC_ADD);
				glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
			}
			else if (blendMode == BlendMode::Overwrite)
			{
				glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
				glBlendFuncSeparate(GL_ONE, GL_ZERO, GL_ONE, GL_ZERO);
			}
			else
			{
				hlog::warn(logTag, "Trying to set unsupported blend mode!");
			}
		}
		else
#endif
		{
			OpenGL_RenderSystem::_setDeviceBlendMode(blendMode);
		}
	}
	
	void OpenGL1_RenderSystem::_setDeviceColorMode(const ColorMode& colorMode, float colorModeFactor, bool useTexture, bool useColor, const Color& systemColor)
	{
		static float constColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		constColor[0] = colorModeFactor;
		constColor[1] = colorModeFactor;
		constColor[2] = colorModeFactor;
		constColor[3] = colorModeFactor;
		if (colorMode == ColorMode::Multiply)
		{
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
			if (useTexture)
			{
				glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);
				glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_TEXTURE);
				glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_ALPHA, GL_PRIMARY_COLOR);
				glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
				glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE);
				glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_PRIMARY_COLOR);
			}
			else
			{
				glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
				glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_PRIMARY_COLOR);
				glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_REPLACE);
				glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_PRIMARY_COLOR);
			}
		}
		else if (colorMode == ColorMode::AlphaMap)
		{
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
			if (useTexture)
			{
				glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);
				glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_TEXTURE);
				glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_ALPHA, GL_PRIMARY_COLOR);
			}
			else
			{
				glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
				glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_PRIMARY_COLOR);
			}
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_REPLACE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_PRIMARY_COLOR);
		}
		else if (colorMode == ColorMode::Lerp)
		{
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
			if (useTexture)
			{
				glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, constColor);
				glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);
				glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_TEXTURE);
				glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_ALPHA, GL_PRIMARY_COLOR);
				glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_INTERPOLATE);
				glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_PRIMARY_COLOR);
				glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_TEXTURE);
				glTexEnvi(GL_TEXTURE_ENV, GL_SRC2_RGB, GL_CONSTANT);
			}
			else
			{
				glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
				glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_PRIMARY_COLOR);
				glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_REPLACE);
				glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_PRIMARY_COLOR);
			}
		}
		else if (colorMode == ColorMode::Desaturate || colorMode == ColorMode::Sepia)
		{
			static bool reported = false;
			if (!reported)
			{
				hlog::errorf(logTag, "The color mode '%s' is not properly supported right now in rendersystem '%s'. A compatibility mode will be used. This error will be printed only once.", colorMode.getName().cStr(), this->getName().cStr());
			}
			reported = true;
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
			if (useTexture)
			{
				glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);
				glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_TEXTURE);
				glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_ALPHA, GL_PRIMARY_COLOR);
				glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
				glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE);
				glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_PRIMARY_COLOR);
			}
			else
			{
				glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
				glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_PRIMARY_COLOR);
				glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_REPLACE);
				glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_PRIMARY_COLOR);
			}
		}
		else
		{
			hlog::warn(logTag, "Trying to set unsupported color mode!");
		}
		if (!useColor)
		{
			this->_setDeviceColor(systemColor);
		}
	}

	void OpenGL1_RenderSystem::_setDeviceColor(const Color& color, bool forceUpdate)
	{
		if (forceUpdate || this->deviceState_color != color)
		{
			glColor4f(color.r_f(), color.g_f(), color.b_f(), color.a_f());
			this->deviceState_color = color;
		}
	}

	void OpenGL1_RenderSystem::_setDeviceMatrixMode(unsigned int mode, bool forceUpdate)
	{
		if (forceUpdate || this->deviceState_matrixMode != mode)
		{
			glMatrixMode(mode);
			this->deviceState_matrixMode = mode;
		}
	}

	void OpenGL1_RenderSystem::_setGlTextureEnabled(bool enabled)
	{
		enabled ? glEnableClientState(GL_TEXTURE_COORD_ARRAY) : glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	}

	void OpenGL1_RenderSystem::_setGlColorEnabled(bool enabled)
	{
		enabled ? glEnableClientState(GL_COLOR_ARRAY) : glDisableClientState(GL_COLOR_ARRAY);
	}

	void OpenGL1_RenderSystem::_setGlVertexPointer(int stride, const void* pointer)
	{
		glVertexPointer(3, GL_FLOAT, stride, pointer);
	}

	void OpenGL1_RenderSystem::_setGlTexturePointer(int stride, const void* pointer)
	{
		glTexCoordPointer(2, GL_FLOAT, stride, pointer);
	}

	void OpenGL1_RenderSystem::_setGlColorPointer(int stride, const void* pointer)
	{
		glColorPointer(4, GL_UNSIGNED_BYTE, stride, pointer);
	}

}
#endif
