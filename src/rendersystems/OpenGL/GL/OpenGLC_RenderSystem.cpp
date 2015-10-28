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

#define MAX_VERTEX_COUNT 65536

namespace april
{
	OpenGLC_RenderSystem::OpenGLC_RenderSystem() : OpenGL_RenderSystem(), blendSeparationSupported(false), deviceState_matrixMode(0)
	{
	}

	OpenGLC_RenderSystem::~OpenGLC_RenderSystem()
	{
	}

	void OpenGLC_RenderSystem::_setupDefaultParameters()
	{
		OpenGL_RenderSystem::_setupDefaultParameters();
		glAlphaFunc(GL_GREATER, 0.0f);
		this->_setGlColor(this->deviceState_color, true);
		this->_setGlMatrixMode(GL_MODELVIEW, true);
	}

	void OpenGLC_RenderSystem::_setDeviceModelviewMatrix(const gmat4& matrix)
	{
		this->_setGlMatrixMode(GL_MODELVIEW);
		glLoadMatrixf(matrix.data);
	}

	void OpenGLC_RenderSystem::_setDeviceProjectionMatrix(const gmat4& matrix)
	{
		this->_setGlMatrixMode(GL_PROJECTION);
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
		this->_setGlClientState(GL_TEXTURE_COORD_ARRAY, useTexture);
		this->_setGlClientState(GL_COLOR_ARRAY, useColor);
		if (!useColor)
		{
			this->_setGlColor(systemColor);
		}
	}

	void OpenGLC_RenderSystem::_deviceRender(RenderOperation renderOperation, PlainVertex* v, int nVertices)
	{
		// This kind of approach to render chunks of vertices is caused by problems on OpenGLES
		// hardware that may allow only a certain amount of vertices to be rendered at the time.
		// Apparently that number is 65536 on HTC Evo 3D so this is used for MAX_VERTEX_COUNT by default.
		int size = nVertices;
#ifdef _ANDROID
		for_iter_step (i, 0, nVertices, size)
		{
			size = this->_limitPrimitives(renderOperation, hmin(nVertices - i, MAX_VERTEX_COUNT));
#endif
			this->_setGlVertexPointer(sizeof(PlainVertex), v);
			glDrawArrays(_glRenderOperations[renderOperation], 0, size);
#ifdef _ANDROID
			v += size;
		}
#endif
	}

	void OpenGLC_RenderSystem::_deviceRender(RenderOperation renderOperation, PlainVertex* v, int nVertices, Color color)
	{
		// This kind of approach to render chunks of vertices is caused by problems on OpenGLES
		// hardware that may allow only a certain amount of vertices to be rendered at the time.
		// Apparently that number is 65536 on HTC Evo 3D so this is used for MAX_VERTEX_COUNT by default.
		int size = nVertices;
#ifdef _ANDROID
		for_iter_step (i, 0, nVertices, size)
		{
			size = this->_limitPrimitives(renderOperation, hmin(nVertices - i, MAX_VERTEX_COUNT));
#endif
			this->_setGlVertexPointer(sizeof(PlainVertex), v);
			glDrawArrays(_glRenderOperations[renderOperation], 0, size);
#ifdef _ANDROID
			v += size;
		}
#endif
	}
	
	void OpenGLC_RenderSystem::_deviceRender(RenderOperation renderOperation, TexturedVertex* v, int nVertices)
	{
		// This kind of approach to render chunks of vertices is caused by problems on OpenGLES
		// hardware that may allow only a certain amount of vertices to be rendered at the time.
		// Apparently that number is 65536 on HTC Evo 3D so this is used for MAX_VERTEX_COUNT by default.
		int size = nVertices;
#ifdef _ANDROID
		for_iter_step (i, 0, nVertices, size)
		{
			size = this->_limitPrimitives(renderOperation, hmin(nVertices - i, MAX_VERTEX_COUNT));
#endif
			this->_setGlVertexPointer(sizeof(TexturedVertex), v);
			this->_setGlTexturePointer(sizeof(TexturedVertex), &v->u);
			glDrawArrays(_glRenderOperations[renderOperation], 0, size);
#ifdef _ANDROID
			v += size;
		}
#endif
	}

	void OpenGLC_RenderSystem::_deviceRender(RenderOperation renderOperation, TexturedVertex* v, int nVertices, Color color)
	{
		// This kind of approach to render chunks of vertices is caused by problems on OpenGLES
		// hardware that may allow only a certain amount of vertices to be rendered at the time.
		// Apparently that number is 65536 on HTC Evo 3D so this is used for MAX_VERTEX_COUNT by default.
		int size = nVertices;
#ifdef _ANDROID
		for_iter_step (i, 0, nVertices, size)
		{
			size = this->_limitPrimitives(renderOperation, hmin(nVertices - i, MAX_VERTEX_COUNT));
#endif
			this->_setGlVertexPointer(sizeof(TexturedVertex), v);
			this->_setGlTexturePointer(sizeof(TexturedVertex), &v->u);
			glDrawArrays(_glRenderOperations[renderOperation], 0, size);
#ifdef _ANDROID
			v += size;
		}
#endif
	}

	void OpenGLC_RenderSystem::_deviceRender(RenderOperation renderOperation, ColoredVertex* v, int nVertices)
	{
		// This kind of approach to render chunks of vertices is caused by problems on OpenGLES
		// hardware that may allow only a certain amount of vertices to be rendered at the time.
		// Apparently that number is 65536 on HTC Evo 3D so this is used for MAX_VERTEX_COUNT by default.
		int size = nVertices;
#ifdef _ANDROID
		for_iter_step (i, 0, nVertices, size)
		{
			size = this->_limitPrimitives(renderOperation, hmin(nVertices - i, MAX_VERTEX_COUNT));
#endif
			this->_setGlVertexPointer(sizeof(ColoredVertex), v);
			this->_setGlColorPointer(sizeof(ColoredVertex), &v->color);
			glDrawArrays(_glRenderOperations[renderOperation], 0, size);
#ifdef _ANDROID
			v += size;
		}
#endif
	}
	
	void OpenGLC_RenderSystem::_deviceRender(RenderOperation renderOperation, ColoredTexturedVertex* v, int nVertices)
	{
		// This kind of approach to render chunks of vertices is caused by problems on OpenGLES
		// hardware that may allow only a certain amount of vertices to be rendered at the time.
		// Apparently that number is 65536 on HTC Evo 3D so this is used for MAX_VERTEX_COUNT by default.
		int size = nVertices;
#ifdef _ANDROID
		for_iter_step (i, 0, nVertices, size)
		{
			size = this->_limitPrimitives(renderOperation, hmin(nVertices - i, MAX_VERTEX_COUNT));
#endif
			this->_setGlVertexPointer(sizeof(ColoredTexturedVertex), v);
			this->_setGlColorPointer(sizeof(ColoredTexturedVertex), &v->color);
			this->_setGlTexturePointer(sizeof(ColoredTexturedVertex), &v->u);
			glDrawArrays(_glRenderOperations[renderOperation], 0, size);
#ifdef _ANDROID
			v += size;
		}
#endif
	}
	
	void OpenGLC_RenderSystem::_setGlClientState(unsigned int type, bool enabled, bool forceUpdate)
	{
		enabled ? glEnableClientState(type) : glDisableClientState(type);
	}

	void OpenGLC_RenderSystem::_setGlColor(const Color& color, bool forceUpdate)
	{
		if (forceUpdate || this->deviceState_color != color)
		{
			glColor4f(color.r_f(), color.g_f(), color.b_f(), color.a_f());
			this->deviceState_color = color;
		}
	}

	void OpenGLC_RenderSystem::_setGlMatrixMode(unsigned int mode, bool forceUpdate)
	{
		if (forceUpdate || this->deviceState_matrixMode != mode)
		{
			glMatrixMode(mode);
			this->deviceState_matrixMode = mode;
		}
	}

	void OpenGLC_RenderSystem::_setGlVertexPointer(int stride, const void* pointer, bool forceUpdate)
	{
		if (forceUpdate || this->deviceState_vertexStride != stride || this->deviceState_vertexPointer != pointer)
		{
			glVertexPointer(3, GL_FLOAT, stride, pointer);
			this->deviceState_vertexStride = stride;
			this->deviceState_vertexPointer = pointer;
		}
	}

	void OpenGLC_RenderSystem::_setGlTexturePointer(int stride, const void* pointer, bool forceUpdate)
	{
		if (forceUpdate || this->deviceState_textureStride != stride || this->deviceState_texturePointer != pointer)
		{
			glTexCoordPointer(2, GL_FLOAT, stride, pointer);
			this->deviceState_textureStride = stride;
			this->deviceState_texturePointer = pointer;
		}
	}

	void OpenGLC_RenderSystem::_setGlColorPointer(int stride, const void* pointer, bool forceUpdate)
	{
		if (forceUpdate || this->deviceState_colorStride != stride || this->deviceState_colorPointer != pointer)
		{
			glColorPointer(4, GL_UNSIGNED_BYTE, stride, pointer);
			this->deviceState_colorStride = stride;
			this->deviceState_colorPointer = pointer;
		}
	}

}
#endif
