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
	OpenGLC_RenderSystem::OpenGLC_RenderSystem() : OpenGL_RenderSystem()
	{
	}

	OpenGLC_RenderSystem::~OpenGLC_RenderSystem()
	{
	}

	void OpenGLC_RenderSystem::_setupDefaultParameters()
	{
		OpenGL_RenderSystem::_setupDefaultParameters();
		this->_setClientState(GL_VERTEX_ARRAY, true);
		this->_setClientState(GL_TEXTURE_COORD_ARRAY, this->deviceState.textureCoordinatesEnabled);
		this->_setClientState(GL_COLOR_ARRAY, this->deviceState.colorEnabled);
		glColor4f(this->deviceState.systemColor.r_f(), this->deviceState.systemColor.g_f(), this->deviceState.systemColor.b_f(), this->deviceState.systemColor.a_f());
	}

	void OpenGLC_RenderSystem::_applyStateChanges()
	{
		if (this->currentState.textureCoordinatesEnabled != this->deviceState.textureCoordinatesEnabled)
		{
			this->_setClientState(GL_TEXTURE_COORD_ARRAY, this->currentState.textureCoordinatesEnabled);
			this->deviceState.textureCoordinatesEnabled = this->currentState.textureCoordinatesEnabled;
		}
		if (this->currentState.colorEnabled != this->deviceState.colorEnabled)
		{
			this->_setClientState(GL_COLOR_ARRAY, this->currentState.colorEnabled);
			this->deviceState.colorEnabled = this->currentState.colorEnabled;
		}
		if (this->currentState.systemColor != this->deviceState.systemColor)
		{
			glColor4f(this->currentState.systemColor.r_f(), this->currentState.systemColor.g_f(), this->currentState.systemColor.b_f(), this->currentState.systemColor.a_f());
			this->deviceState.systemColor = this->currentState.systemColor;
		}
		if (this->currentState.textureId != this->deviceState.textureId)
		{
			glBindTexture(GL_TEXTURE_2D, this->currentState.textureId);
			this->setMatrixMode(GL_TEXTURE);
			if (this->currentState.textureId != 0 && (this->activeTexture->effectiveWidth != 1.0f || this->activeTexture->effectiveHeight != 1.0f))
			{
				gmat4 matrix;
				matrix.scale(this->activeTexture->effectiveWidth, this->activeTexture->effectiveHeight, 1.0f);
				glLoadMatrixf(matrix.data);
			}
			else
			{
				this->_loadIdentityMatrix();
			}
			this->deviceState.textureId = this->currentState.textureId;
			// TODO - should memorize address and filter modes per texture in opengl to avoid unnecessary calls
			this->deviceState.textureAddressMode = Texture::ADDRESS_UNDEFINED;
			this->deviceState.textureFilter = Texture::FILTER_UNDEFINED;
		}
		OpenGL_RenderSystem::_applyStateChanges();
		if (this->currentState.modelviewMatrixChanged && this->modelviewMatrix != this->deviceState.modelviewMatrix)
		{
			this->setMatrixMode(GL_MODELVIEW);
			glLoadMatrixf(this->modelviewMatrix.data);
			this->deviceState.modelviewMatrix = this->modelviewMatrix;
			this->currentState.modelviewMatrixChanged = false;
		}
		if (this->currentState.projectionMatrixChanged && this->projectionMatrix != this->deviceState.projectionMatrix)
		{
			this->setMatrixMode(GL_PROJECTION);
			glLoadMatrixf(this->projectionMatrix.data);
			this->deviceState.projectionMatrix = this->projectionMatrix;
			this->currentState.projectionMatrixChanged = false;
		}
	}

	void OpenGLC_RenderSystem::_setClientState(unsigned int type, bool enabled)
	{
		enabled ? glEnableClientState(type) : glDisableClientState(type);
	}

	void OpenGLC_RenderSystem::_setTextureColorMode(ColorMode textureColorMode, float factor)
	{
		static float constColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		constColor[3] = factor;
		switch (textureColorMode)
		{
		case CM_DEFAULT:
		case CM_MULTIPLY:
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_TEXTURE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_ALPHA, GL_PRIMARY_COLOR);
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_PRIMARY_COLOR);
			break;
		case CM_LERP:
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
			glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, constColor);
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_TEXTURE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_ALPHA, GL_PRIMARY_COLOR);
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_INTERPOLATE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_PRIMARY_COLOR);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_TEXTURE);
			break;
		case CM_ALPHA_MAP:
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_TEXTURE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_ALPHA, GL_PRIMARY_COLOR);
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_REPLACE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_PRIMARY_COLOR);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_PRIMARY_COLOR);
			break;
		default:
			hlog::warn(logTag, "Trying to set unsupported color mode!");
			break;
		}
	}

	void OpenGLC_RenderSystem::_setDepthBuffer(bool enabled, bool writeEnabled)
	{
		OpenGL_RenderSystem::_setDepthBuffer(enabled, writeEnabled);
		enabled ? glEnable(GL_ALPHA_TEST) : glDisable(GL_ALPHA_TEST);
		glAlphaFunc(GL_GREATER, 0.0f);
	}

	void OpenGLC_RenderSystem::_loadIdentityMatrix()
	{
		static gmat4 identityMatrix(1.0f, 0.0f, 0.0f, 0.0f,
									0.0f, 1.0f, 0.0f, 0.0f,
									0.0f, 0.0f, 1.0f, 0.0f,
									0.0f, 0.0f, 0.0f, 1.0f);
		glLoadMatrixf(identityMatrix.data);
	}

	void OpenGLC_RenderSystem::_setMatrixMode(unsigned int mode)
	{
		glMatrixMode(mode);
	}

	void OpenGLC_RenderSystem::_setVertexPointer(int stride, const void* pointer)
	{
		if (this->deviceState.strideVertex != stride || this->deviceState.pointerVertex != pointer)
		{
			this->deviceState.strideVertex = stride;
			this->deviceState.pointerVertex = pointer;
			glVertexPointer(3, GL_FLOAT, stride, pointer);
		}
	}

	void OpenGLC_RenderSystem::_setTexCoordPointer(int stride, const void *pointer)
	{
		if (this->deviceState.strideTexCoord != stride || this->deviceState.pointerTexCoord != pointer)
		{
			this->deviceState.strideTexCoord = stride;
			this->deviceState.pointerTexCoord = pointer;
			glTexCoordPointer(2, GL_FLOAT, stride, pointer);
		}
	}

	void OpenGLC_RenderSystem::_setColorPointer(int stride, const void *pointer)
	{
		if (this->deviceState.strideColor != stride || this->deviceState.pointerColor != pointer)
		{
			this->deviceState.strideColor = stride;
			this->deviceState.pointerColor = pointer;
			glColorPointer(4, GL_UNSIGNED_BYTE, stride, pointer);
		}
	}

	void OpenGLC_RenderSystem::render(RenderOperation renderOperation, PlainVertex* v, int nVertices)
	{
		this->currentState.textureId = 0;
		this->currentState.textureCoordinatesEnabled = false;
		this->currentState.colorEnabled = false;
		this->currentState.systemColor.set(255, 255, 255, 255);
		this->_applyStateChanges();
		this->_setColorPointer(0, NULL);
		this->_setTexCoordPointer(0, NULL);
		// This kind of approach to render chunks of vertices is caused by problems on OpenGLES
		// hardware that may allow only a certain amount of vertices to be rendered at the time.
		// Apparently that number is 65536 on HTC Evo 3D so this is used for MAX_VERTEX_COUNT by default.
		int size = nVertices;
#ifdef _ANDROID
		for_iter_step (i, 0, nVertices, size)
		{
			size = this->_limitPrimitives(renderOperation, hmin(nVertices - i, MAX_VERTEX_COUNT));
#endif
			this->_setVertexPointer(sizeof(PlainVertex), v);
			glDrawArrays(glRenderOperations[renderOperation], 0, size);
#ifdef _ANDROID
			v += size;
		}
#endif
	}

	void OpenGLC_RenderSystem::render(RenderOperation renderOperation, PlainVertex* v, int nVertices, Color color)
	{
		this->currentState.textureId = 0;
		this->currentState.textureCoordinatesEnabled = false;
		this->currentState.colorEnabled = false;
		this->currentState.systemColor = color;
		this->_applyStateChanges();
		this->_setColorPointer(0, NULL);
		this->_setTexCoordPointer(0, NULL);
		// This kind of approach to render chunks of vertices is caused by problems on OpenGLES
		// hardware that may allow only a certain amount of vertices to be rendered at the time.
		// Apparently that number is 65536 on HTC Evo 3D so this is used for MAX_VERTEX_COUNT by default.
		int size = nVertices;
#ifdef _ANDROID
		for_iter_step (i, 0, nVertices, size)
		{
			size = this->_limitPrimitives(renderOperation, hmin(nVertices - i, MAX_VERTEX_COUNT));
#endif
			this->_setVertexPointer(sizeof(PlainVertex), v);
			glDrawArrays(glRenderOperations[renderOperation], 0, size);
#ifdef _ANDROID
			v += size;
		}
#endif
	}
	
	void OpenGLC_RenderSystem::render(RenderOperation renderOperation, TexturedVertex* v, int nVertices)
	{
		this->currentState.textureCoordinatesEnabled = true;
		this->currentState.colorEnabled = false;
		this->currentState.systemColor.set(255, 255, 255, 255);
		this->_applyStateChanges();
		this->_setColorPointer(0, NULL);
		// This kind of approach to render chunks of vertices is caused by problems on OpenGLES
		// hardware that may allow only a certain amount of vertices to be rendered at the time.
		// Apparently that number is 65536 on HTC Evo 3D so this is used for MAX_VERTEX_COUNT by default.
		int size = nVertices;
#ifdef _ANDROID
		for_iter_step (i, 0, nVertices, size)
		{
			size = this->_limitPrimitives(renderOperation, hmin(nVertices - i, MAX_VERTEX_COUNT));
#endif
			this->_setVertexPointer(sizeof(TexturedVertex), v);
			this->_setTexCoordPointer(sizeof(TexturedVertex), &v->u);
			glDrawArrays(glRenderOperations[renderOperation], 0, size);
#ifdef _ANDROID
			v += size;
		}
#endif
	}

	void OpenGLC_RenderSystem::render(RenderOperation renderOperation, TexturedVertex* v, int nVertices, Color color)
	{
		this->currentState.textureCoordinatesEnabled = true;
		this->currentState.colorEnabled = false;
		this->currentState.systemColor = color;
		this->_applyStateChanges();
		this->_setColorPointer(0, NULL);
		// This kind of approach to render chunks of vertices is caused by problems on OpenGLES
		// hardware that may allow only a certain amount of vertices to be rendered at the time.
		// Apparently that number is 65536 on HTC Evo 3D so this is used for MAX_VERTEX_COUNT by default.
		int size = nVertices;
#ifdef _ANDROID
		for_iter_step (i, 0, nVertices, size)
		{
			size = this->_limitPrimitives(renderOperation, hmin(nVertices - i, MAX_VERTEX_COUNT));
#endif
			this->_setVertexPointer(sizeof(TexturedVertex), v);
			this->_setTexCoordPointer(sizeof(TexturedVertex), &v->u);
			glDrawArrays(glRenderOperations[renderOperation], 0, size);
#ifdef _ANDROID
			v += size;
		}
#endif
	}

	void OpenGLC_RenderSystem::render(RenderOperation renderOperation, ColoredVertex* v, int nVertices)
	{
		this->currentState.textureId = 0;
		this->currentState.textureCoordinatesEnabled = false;
		this->currentState.colorEnabled = true;
		this->currentState.systemColor.set(255, 255, 255, 255);
		this->_applyStateChanges();
		this->_setTexCoordPointer(0, NULL);
		// This kind of approach to render chunks of vertices is caused by problems on OpenGLES
		// hardware that may allow only a certain amount of vertices to be rendered at the time.
		// Apparently that number is 65536 on HTC Evo 3D so this is used for MAX_VERTEX_COUNT by default.
		int size = nVertices;
#ifdef _ANDROID
		for_iter_step (i, 0, nVertices, size)
		{
			size = this->_limitPrimitives(renderOperation, hmin(nVertices - i, MAX_VERTEX_COUNT));
#endif
			this->_setVertexPointer(sizeof(ColoredVertex), v);
			this->_setColorPointer(sizeof(ColoredVertex), &v->color);
			glDrawArrays(glRenderOperations[renderOperation], 0, size);
#ifdef _ANDROID
			v += size;
		}
#endif
	}
	
	void OpenGLC_RenderSystem::render(RenderOperation renderOperation, ColoredTexturedVertex* v, int nVertices)
	{
		this->currentState.textureCoordinatesEnabled = true;
		this->currentState.colorEnabled = true;
		this->currentState.systemColor.set(255, 255, 255, 255);
		this->_applyStateChanges();
		// This kind of approach to render chunks of vertices is caused by problems on OpenGLES
		// hardware that may allow only a certain amount of vertices to be rendered at the time.
		// Apparently that number is 65536 on HTC Evo 3D so this is used for MAX_VERTEX_COUNT by default.
		int size = nVertices;
#ifdef _ANDROID
		for_iter_step (i, 0, nVertices, size)
		{
			size = this->_limitPrimitives(renderOperation, hmin(nVertices - i, MAX_VERTEX_COUNT));
#endif
			this->_setVertexPointer(sizeof(ColoredTexturedVertex), v);
			this->_setColorPointer(sizeof(ColoredTexturedVertex), &v->color);
			this->_setTexCoordPointer(sizeof(ColoredTexturedVertex), &v->u);
			glDrawArrays(glRenderOperations[renderOperation], 0, size);
#ifdef _ANDROID
			v += size;
		}
#endif
	}
	
}
#endif
