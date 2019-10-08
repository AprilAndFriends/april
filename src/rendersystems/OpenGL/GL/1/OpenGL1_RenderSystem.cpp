/// @file
/// @version 5.2
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
	OpenGL1_RenderSystem::OpenGL1_RenderSystem() :
		OpenGL_RenderSystem(),
		deviceState_matrixMode(0)
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
		hstr extensions;
		GL_SAFE_CALL(const GLubyte* extensionsString = glGetString, (GL_EXTENSIONS));
		if (extensionsString != NULL)
		{
			extensions = (const char*)extensionsString;
		}
		hlog::write(logTag, "Extensions supported:\n- " + extensions.trimmedRight().replaced(" ", "\n- "));
		if (extensions.contains("ARB_texture_non_power_of_two"))
		{
			this->caps.npotTexturesLimited = true;
			// this isn't 100% sure, but it's a pretty good indicator that NPOT textures should be fully supported on this hardware
			if (extensions.contains("ARB_fragment_program"))
			{
				int value = 0;
				GL_SAFE_CALL(glGetIntegerv, (GL_MAX_TEXTURE_SIZE, &value));
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
		GL_SAFE_CALL(glEnableClientState, (GL_VERTEX_ARRAY));
		GL_SAFE_CALL(glAlphaFunc, (GL_GREATER, 0.0f));
		GL_SAFE_CALL(glPixelStorei, (GL_UNPACK_SKIP_ROWS, 0));
		GL_SAFE_CALL(glPixelStorei, (GL_UNPACK_SKIP_PIXELS, 0));
		GL_SAFE_CALL(glPixelStorei, (GL_UNPACK_ROW_LENGTH, 0));
		GL_SAFE_CALL(glPixelStorei, (GL_UNPACK_SWAP_BYTES, GL_FALSE));
		GL_SAFE_CALL(glEnable, (GL_TEXTURE_2D));
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
		GL_SAFE_CALL(glLoadMatrixf, (matrix.data));
	}

	void OpenGL1_RenderSystem::_setDeviceProjectionMatrix(const gmat4& matrix)
	{
		this->_setDeviceMatrixMode(GL_PROJECTION);
		GL_SAFE_CALL(glLoadMatrixf, (matrix.data));
	}

	void OpenGL1_RenderSystem::_setDeviceDepthBuffer(bool enabled, bool writeEnabled)
	{
		OpenGL_RenderSystem::_setDeviceDepthBuffer(enabled, writeEnabled);
		if (enabled)
		{
			GL_SAFE_CALL(glEnable, (GL_ALPHA_TEST));
		}
		else
		{
			GL_SAFE_CALL(glDisable, (GL_ALPHA_TEST));
		}
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
					GL_SAFE_CALL(glLoadMatrixf, (matrix.data));
				}
				else
				{
					static gmat4 matrix;
					GL_SAFE_CALL(glLoadMatrixf, (matrix.data));
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
				GL_SAFE_CALL(glBlendEquationSeparate, (GL_FUNC_ADD, GL_FUNC_ADD));
				GL_SAFE_CALL(glBlendFuncSeparate, (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA));
			}
			else if (blendMode == BlendMode::Add)
			{
				GL_SAFE_CALL(glBlendEquationSeparate, (GL_FUNC_ADD, GL_FUNC_ADD));
				GL_SAFE_CALL(glBlendFuncSeparate, (GL_SRC_ALPHA, GL_ONE, GL_ONE, GL_ONE_MINUS_SRC_ALPHA));
			}
			else if (blendMode == BlendMode::Subtract)
			{
				GL_SAFE_CALL(glBlendEquationSeparate, (GL_FUNC_REVERSE_SUBTRACT, GL_FUNC_ADD));
				GL_SAFE_CALL(glBlendFuncSeparate, (GL_SRC_ALPHA, GL_ONE, GL_ONE, GL_ONE_MINUS_SRC_ALPHA));
			}
			else if (blendMode == BlendMode::Overwrite)
			{
				GL_SAFE_CALL(glBlendEquationSeparate, (GL_FUNC_ADD, GL_FUNC_ADD));
				GL_SAFE_CALL(glBlendFuncSeparate, (GL_ONE, GL_ZERO, GL_ONE, GL_ZERO));
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
			GL_SAFE_CALL(glTexEnvi, (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE));
			if (useTexture)
			{
				GL_SAFE_CALL(glTexEnvi, (GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE));
				GL_SAFE_CALL(glTexEnvi, (GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_TEXTURE));
				GL_SAFE_CALL(glTexEnvi, (GL_TEXTURE_ENV, GL_SRC1_ALPHA, GL_PRIMARY_COLOR));
				GL_SAFE_CALL(glTexEnvi, (GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE));
				GL_SAFE_CALL(glTexEnvi, (GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE));
				GL_SAFE_CALL(glTexEnvi, (GL_TEXTURE_ENV, GL_SRC1_RGB, GL_PRIMARY_COLOR));
			}
			else
			{
				GL_SAFE_CALL(glTexEnvi, (GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE));
				GL_SAFE_CALL(glTexEnvi, (GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_PRIMARY_COLOR));
				GL_SAFE_CALL(glTexEnvi, (GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_REPLACE));
				GL_SAFE_CALL(glTexEnvi, (GL_TEXTURE_ENV, GL_SRC0_RGB, GL_PRIMARY_COLOR));
			}
		}
		else if (colorMode == ColorMode::AlphaMap)
		{
			GL_SAFE_CALL(glTexEnvi, (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE));
			if (useTexture)
			{
				GL_SAFE_CALL(glTexEnvi, (GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE));
				GL_SAFE_CALL(glTexEnvi, (GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_TEXTURE));
				GL_SAFE_CALL(glTexEnvi, (GL_TEXTURE_ENV, GL_SRC1_ALPHA, GL_PRIMARY_COLOR));
			}
			else
			{
				GL_SAFE_CALL(glTexEnvi, (GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE));
				GL_SAFE_CALL(glTexEnvi, (GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_PRIMARY_COLOR));
			}
			GL_SAFE_CALL(glTexEnvi, (GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_REPLACE));
			GL_SAFE_CALL(glTexEnvi, (GL_TEXTURE_ENV, GL_SRC0_RGB, GL_PRIMARY_COLOR));
		}
		else if (colorMode == ColorMode::Lerp)
		{
			GL_SAFE_CALL(glTexEnvi, (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE));
			if (useTexture)
			{
				GL_SAFE_CALL(glTexEnvfv, (GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, constColor));
				GL_SAFE_CALL(glTexEnvi, (GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE));
				GL_SAFE_CALL(glTexEnvi, (GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_TEXTURE));
				GL_SAFE_CALL(glTexEnvi, (GL_TEXTURE_ENV, GL_SRC1_ALPHA, GL_PRIMARY_COLOR));
				GL_SAFE_CALL(glTexEnvi, (GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_INTERPOLATE));
				GL_SAFE_CALL(glTexEnvi, (GL_TEXTURE_ENV, GL_SRC0_RGB, GL_PRIMARY_COLOR));
				GL_SAFE_CALL(glTexEnvi, (GL_TEXTURE_ENV, GL_SRC1_RGB, GL_TEXTURE));
				GL_SAFE_CALL(glTexEnvi, (GL_TEXTURE_ENV, GL_SRC2_RGB, GL_CONSTANT));
			}
			else
			{
				GL_SAFE_CALL(glTexEnvi, (GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE));
				GL_SAFE_CALL(glTexEnvi, (GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_PRIMARY_COLOR));
				GL_SAFE_CALL(glTexEnvi, (GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_REPLACE));
				GL_SAFE_CALL(glTexEnvi, (GL_TEXTURE_ENV, GL_SRC0_RGB, GL_PRIMARY_COLOR));
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
			GL_SAFE_CALL(glTexEnvi, (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE));
			if (useTexture)
			{
				GL_SAFE_CALL(glTexEnvi, (GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE));
				GL_SAFE_CALL(glTexEnvi, (GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_TEXTURE));
				GL_SAFE_CALL(glTexEnvi, (GL_TEXTURE_ENV, GL_SRC1_ALPHA, GL_PRIMARY_COLOR));
				GL_SAFE_CALL(glTexEnvi, (GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE));
				GL_SAFE_CALL(glTexEnvi, (GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE));
				GL_SAFE_CALL(glTexEnvi, (GL_TEXTURE_ENV, GL_SRC1_RGB, GL_PRIMARY_COLOR));
			}
			else
			{
				GL_SAFE_CALL(glTexEnvi, (GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE));
				GL_SAFE_CALL(glTexEnvi, (GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_PRIMARY_COLOR));
				GL_SAFE_CALL(glTexEnvi, (GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_REPLACE));
				GL_SAFE_CALL(glTexEnvi, (GL_TEXTURE_ENV, GL_SRC0_RGB, GL_PRIMARY_COLOR));
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

	void OpenGL1_RenderSystem::_setDeviceRenderTarget(Texture* texture)
	{
		hlog::warnf(logTag, "Render targets are not implemented in render system '%s'!", this->name.cStr());
	}

	void OpenGL1_RenderSystem::_deviceTakeScreenshot(Image::Format format, bool backBufferOnly)
	{
		GL_SAFE_CALL(glReadBuffer, (GL_FRONT));
		int w = april::window->getWidth();
		int h = april::window->getHeight();
		unsigned char* temp = new unsigned char[w * (h + 1) * 4]; // 4 BPP and one extra row just in case some OpenGL implementations don't blit properly and cause a memory leak
		GL_SAFE_CALL(glReadPixels, (0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, temp));
		// GL returns all pixels flipped vertically so this needs to be corrected first
		int stride = w * 4;
		unsigned char* data = new unsigned char[stride * h];
		for_iter (i, 0, h)
		{
			memcpy(&data[i * stride], &temp[(h - i - 1) * stride], stride);
		}
		delete[] temp;
		temp = data;
		data = NULL;
		if (Image::convertToFormat(w, h, temp, Image::Format::RGBX, &data, format, false))
		{
			april::window->queueScreenshot(Image::create(w, h, data, format));
			delete[] data;
		}
		delete[] temp;
	}

	void OpenGL1_RenderSystem::_setDeviceColor(const Color& color, bool forceUpdate)
	{
		if (forceUpdate || this->deviceState_color != color)
		{
			GL_SAFE_CALL(glColor4f, (color.r_f(), color.g_f(), color.b_f(), color.a_f()));
			this->deviceState_color = color;
		}
	}

	void OpenGL1_RenderSystem::_setDeviceMatrixMode(unsigned int mode, bool forceUpdate)
	{
		if (forceUpdate || this->deviceState_matrixMode != mode)
		{
			GL_SAFE_CALL(glMatrixMode, (mode));
			this->deviceState_matrixMode = mode;
		}
	}

	void OpenGL1_RenderSystem::_setGlTextureEnabled(bool enabled)
	{
		if (enabled)
		{
			GL_SAFE_CALL(glEnableClientState, (GL_TEXTURE_COORD_ARRAY));
		}
		else
		{
			GL_SAFE_CALL(glDisableClientState, (GL_TEXTURE_COORD_ARRAY));
		}
	}

	void OpenGL1_RenderSystem::_setGlColorEnabled(bool enabled)
	{
		if (enabled)
		{
			GL_SAFE_CALL(glEnableClientState, (GL_COLOR_ARRAY));
		}
		else
		{
			GL_SAFE_CALL(glDisableClientState, (GL_COLOR_ARRAY));
		}
	}

	void OpenGL1_RenderSystem::_setGlVertexPointer(int stride, const void* pointer)
	{
		GL_SAFE_CALL(glVertexPointer, (3, GL_FLOAT, stride, pointer));
	}

	void OpenGL1_RenderSystem::_setGlTexturePointer(int stride, const void* pointer)
	{
		GL_SAFE_CALL(glTexCoordPointer, (2, GL_FLOAT, stride, pointer));
	}

	void OpenGL1_RenderSystem::_setGlColorPointer(int stride, const void* pointer)
	{
		GL_SAFE_CALL(glColorPointer, (4, GL_UNSIGNED_BYTE, stride, pointer));
	}

}
#endif
