/// @file
/// @version 4.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#if (defined(_OPENGL) || defined(_OPENGLES1)) && !defined(_OPENGLES)
#include <hltypes/hlog.h>
#include <hltypes/hstring.h>

#include "april.h"
#include "OpenGLC_RenderSystem.h"
#include "OpenGLC_Texture.h"

namespace april
{
	OpenGLC_RenderSystem::OpenGLC_RenderSystem() : OpenGL_RenderSystem(), deviceState_matrixMode(0)
	{
	}

	OpenGLC_RenderSystem::~OpenGLC_RenderSystem()
	{
	}

	void OpenGLC_RenderSystem::_deviceInit()
	{
		OpenGL_RenderSystem::_deviceInit();
		this->deviceState_color = april::Color::White;
		this->deviceState_matrixMode = 0;
	}

	void OpenGLC_RenderSystem::_deviceSetup()
	{
		OpenGL_RenderSystem::_deviceSetup();
		glEnableClientState(GL_VERTEX_ARRAY);
		glAlphaFunc(GL_GREATER, 0.0f);
		this->_setDeviceColor(this->deviceState_color, true);
		this->_setDeviceMatrixMode(GL_MODELVIEW, true);
	}

	void OpenGLC_RenderSystem::_setDeviceModelviewMatrix(const gmat4& matrix)
	{
		this->_setDeviceMatrixMode(GL_MODELVIEW);
		glLoadMatrixf(matrix.data);
	}

	void OpenGLC_RenderSystem::_setDeviceProjectionMatrix(const gmat4& matrix)
	{
		this->_setDeviceMatrixMode(GL_PROJECTION);
		glLoadMatrixf(matrix.data);
	}

	void OpenGLC_RenderSystem::_setDeviceDepthBuffer(bool enabled, bool writeEnabled)
	{
		OpenGL_RenderSystem::_setDeviceDepthBuffer(enabled, writeEnabled);
		enabled ? glEnable(GL_ALPHA_TEST) : glDisable(GL_ALPHA_TEST);
	}

	void OpenGLC_RenderSystem::_setDeviceColorMode(ColorMode colorMode, float colorModeFactor, bool useTexture, bool useColor, const Color& systemColor)
	{
		static float constColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		constColor[0] = colorModeFactor;
		constColor[1] = colorModeFactor;
		constColor[2] = colorModeFactor;
		constColor[3] = colorModeFactor;
		switch (colorMode)
		{
		case CM_DEFAULT:
		case CM_MULTIPLY:
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
			break;
		case CM_ALPHA_MAP:
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
			break;
		case CM_LERP:
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
			break;
		default:
			hlog::warn(logTag, "Trying to set unsupported color mode!");
			break;
		}
		if (!useColor)
		{
			this->_setDeviceColor(systemColor);
		}
	}

	void OpenGLC_RenderSystem::_setDeviceColor(const Color& color, bool forceUpdate)
	{
		if (forceUpdate || this->deviceState_color != color)
		{
			glColor4f(color.r_f(), color.g_f(), color.b_f(), color.a_f());
			this->deviceState_color = color;
		}
	}

	void OpenGLC_RenderSystem::_setDeviceMatrixMode(unsigned int mode, bool forceUpdate)
	{
		if (forceUpdate || this->deviceState_matrixMode != mode)
		{
			glMatrixMode(mode);
			this->deviceState_matrixMode = mode;
		}
	}

	void OpenGLC_RenderSystem::_setGlTextureEnabled(bool enabled)
	{
		enabled ? glEnableClientState(GL_TEXTURE_COORD_ARRAY) : glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	}

	void OpenGLC_RenderSystem::_setGlColorEnabled(bool enabled)
	{
		enabled ? glEnableClientState(GL_COLOR_ARRAY) : glDisableClientState(GL_COLOR_ARRAY);
	}

	void OpenGLC_RenderSystem::_setGlVertexPointer(int stride, const void* pointer)
	{
		glVertexPointer(3, GL_FLOAT, stride, pointer);
	}

	void OpenGLC_RenderSystem::_setGlTexturePointer(int stride, const void* pointer)
	{
		glTexCoordPointer(2, GL_FLOAT, stride, pointer);
	}

	void OpenGLC_RenderSystem::_setGlColorPointer(int stride, const void* pointer)
	{
		glColorPointer(4, GL_UNSIGNED_BYTE, stride, pointer);
	}

}
#endif
