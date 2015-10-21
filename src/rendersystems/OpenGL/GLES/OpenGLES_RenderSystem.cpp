/// @file
/// @version 3.7
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifdef _OPENGLES

#define __HL_INCLUDE_PLATFORM_HEADERS
#include <hltypes/hplatform.h>
#if __APPLE__
#include <TargetConditionals.h>
#endif

#include <hltypes/hlog.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hstring.h>

#include "april.h"
#ifdef _EGL
#include "egl.h"
#endif
#include "OpenGLES_RenderSystem.h"
#include "OpenGLES_Texture.h"
#include "Window.h"

#define VERTEX_ARRAY 0
#define TEXCOORD_ARRAY 1
#define COLOR_ARRAY 2

namespace april
{
	OpenGLES_RenderSystem::OpenGLES_RenderSystem() : OpenGL_RenderSystem()
	{
	}

	OpenGLES_RenderSystem::~OpenGLES_RenderSystem()
	{
	}

	void OpenGLES_RenderSystem::assignWindow(Window* window)
	{
#if defined(_WIN32) && !defined(_WINRT)
		if (!this->_initWin32(window))
		{
			return;
		}
#endif
		OpenGL_RenderSystem::assignWindow(window);
	}

	void OpenGLES_RenderSystem::_setupDefaultParameters()
	{
		OpenGL_RenderSystem::_setupDefaultParameters();
		this->_setClientState(VERTEX_ARRAY, true);
		this->_setClientState(TEXCOORD_ARRAY, this->deviceState.textureCoordinatesEnabled);
		this->_setClientState(COLOR_ARRAY, this->deviceState.colorEnabled);
		// TODO
	}

	void OpenGLES_RenderSystem::_applyStateChanges()
	{
		if (this->currentState.textureCoordinatesEnabled != this->deviceState.textureCoordinatesEnabled)
		{
			this->_setClientState(TEXCOORD_ARRAY, this->currentState.textureCoordinatesEnabled);
			this->deviceState.textureCoordinatesEnabled = this->currentState.textureCoordinatesEnabled;
		}
		if (this->currentState.colorEnabled != this->deviceState.colorEnabled)
		{
			this->_setClientState(COLOR_ARRAY, this->currentState.colorEnabled);
			this->deviceState.colorEnabled = this->currentState.colorEnabled;
		}
		if (this->currentState.textureId != this->deviceState.textureId)
		{
			glBindTexture(GL_TEXTURE_2D, this->currentState.textureId);
			this->setMatrixMode(GL_TEXTURE);
			if (this->currentState.textureId != 0 && (this->activeTexture->effectiveWidth != 1.0f || this->activeTexture->effectiveHeight != 1.0f))
			{
				gmat4 matrix;
				matrix.scale(this->activeTexture->effectiveWidth, this->activeTexture->effectiveHeight, 1.0f);
				// TODO
				// glUniformMatrix4fv(0, 1, GL_FALSE, matrix.data);
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
		// TODO
		/*
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
		*/
	}

	void OpenGLES_RenderSystem::_setClientState(unsigned int type, bool enabled)
	{
		enabled ? glEnableVertexAttribArray(type) : glDisableVertexAttribArray(type);
	}

	void OpenGLES_RenderSystem::_setupCaps()
	{
#ifdef _EGL
		if (april::egl->display == NULL)
		{
			return;
		}
#endif
		if (this->caps.maxTextureSize == 0)
		{
			hstr extensions = (const char*)glGetString(GL_EXTENSIONS);
#ifndef _WINRT
			this->caps.npotTexturesLimited = (extensions.contains("IMG_texture_npot") || extensions.contains("APPLE_texture_2D_limited_npot"));
#else
			this->caps.npotTexturesLimited = true;
#endif
			this->caps.npotTextures = (extensions.contains("OES_texture_npot") || extensions.contains("ARB_texture_non_power_of_two"));
		}
#ifdef _ANDROID // Android has problems with alpha textures in some implementations
		this->caps.textureFormats /= Image::FORMAT_ALPHA;
		this->caps.textureFormats /= Image::FORMAT_GRAYSCALE;
#endif
		return OpenGL_RenderSystem::_setupCaps();
	}

	void OpenGLES_RenderSystem::_setTextureBlendMode(BlendMode textureBlendMode)
	{
		// TODO
	}

	void OpenGLES_RenderSystem::_setTextureColorMode(ColorMode textureColorMode, float factor)
	{
		// TODO
	}

	void OpenGLES_RenderSystem::_loadIdentityMatrix()
	{
		static gmat4 identityMatrix(1.0f, 0.0f, 0.0f, 0.0f,
									0.0f, 1.0f, 0.0f, 0.0f,
									0.0f, 0.0f, 1.0f, 0.0f,
									0.0f, 0.0f, 0.0f, 1.0f);
		
		//glUniformMatrix4fv(0, 1, GL_FALSE, identityMatrix.data);
		// TODO
	}
	
	void OpenGLES_RenderSystem::_setMatrixMode(unsigned int mode)
	{
		// TODO
	}

	void OpenGLES_RenderSystem::_setVertexPointer(int stride, const void* pointer)
	{
		if (this->deviceState.strideVertex != stride || this->deviceState.pointerVertex != pointer)
		{
			this->deviceState.strideVertex = stride;
			this->deviceState.pointerVertex = pointer;
			glVertexAttribPointer(VERTEX_ARRAY, 3, GL_FLOAT, GL_FALSE, stride, pointer);
		}
	}

	void OpenGLES_RenderSystem::_setTexCoordPointer(int stride, const void *pointer)
	{
		if (this->deviceState.strideVertex != stride || this->deviceState.pointerVertex != pointer)
		{
			this->deviceState.strideVertex = stride;
			this->deviceState.pointerVertex = pointer;
			glVertexAttribPointer(TEXCOORD_ARRAY, 2, GL_FLOAT, GL_FALSE, stride, pointer);
		}
	}

	void OpenGLES_RenderSystem::_setColorPointer(int stride, const void *pointer)
	{
		if (this->deviceState.strideColor != stride || this->deviceState.pointerColor != pointer)
		{
			this->deviceState.strideColor = stride;
			this->deviceState.pointerColor = pointer;
			glVertexAttribPointer(COLOR_ARRAY, 4, GL_UNSIGNED_BYTE, GL_FALSE, stride, pointer);
		}
	}

}
#endif
