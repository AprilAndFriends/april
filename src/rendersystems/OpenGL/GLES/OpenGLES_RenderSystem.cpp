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
#include "OpenGLES_defaultShaders.h"
#include "OpenGLES_PixelShader.h"
#include "OpenGLES_RenderSystem.h"
#include "OpenGLES_Texture.h"
#include "OpenGLES_VertexShader.h"
#include "Window.h"

#define VERTEX_ARRAY 0
#define COLOR_ARRAY 1
#define TEXCOORD_ARRAY 2

#define DELETE_SHADER(name, type) \
	if (name != NULL) \
	{ \
		this->destroy ## type ## Shader(name); \
		name = NULL; \
	}

#define LOAD_SHADER(name, type, mode, data)\
	if (name == NULL) \
	{ \
		data.clear(); \
		data.write(SHADER_ ## type ## mode); \
		name = (OpenGLES_ ## type ## Shader*)this->_create ## type ## Shader(); \
		name->load(data); \
	}

#define LOAD_PROGRAM(name, pixelShader, vertexShader)\
	if (name == NULL) \
	{ \
		name = new ShaderProgram(); \
		if (!name->load(pixelShader->glShader, vertexShader->glShader)) \
		{ \
			_HL_TRY_DELETE(name); \
		} \
	}

namespace april
{
	OpenGLES_RenderSystem::ShaderProgram::ShaderProgram() : glShaderProgram(0)
	{
	}

	OpenGLES_RenderSystem::ShaderProgram::~ShaderProgram()
	{
		if (this->glShaderProgram != 0)
		{
			glDeleteProgram(this->glShaderProgram);
		}
	}

	bool OpenGLES_RenderSystem::ShaderProgram::load(unsigned int pixelShaderId, unsigned int vertexShaderId)
	{
		if (this->glShaderProgram != 0)
		{
			hlog::error(logTag, "Shader program alread created!");
			return false;
		}
		this->glShaderProgram = glCreateProgram();
		if (this->glShaderProgram == 0)
		{
			hlog::error(logTag, "Could not create shader program!");
			return false;
		}
		glAttachShader(this->glShaderProgram, pixelShaderId);
		glAttachShader(this->glShaderProgram, vertexShaderId);
		glBindAttribLocation(this->glShaderProgram, VERTEX_ARRAY, "position");
		glBindAttribLocation(this->glShaderProgram, COLOR_ARRAY, "color");
		glBindAttribLocation(this->glShaderProgram, TEXCOORD_ARRAY, "tex");
		glLinkProgram(this->glShaderProgram);
		GLint linked;
		glGetProgramiv(this->glShaderProgram, GL_LINK_STATUS, &linked);
		if (linked == 0)
		{
			int messageSize = 0;
			int written = 0;
			glGetShaderiv(this->glShaderProgram, GL_INFO_LOG_LENGTH, &messageSize);
			char* message = new char[messageSize];
			glGetProgramInfoLog(this->glShaderProgram, messageSize, &written, message);
			hlog::error(logTag, "Shader Program could not be linked! Error:\n" + hstr(message));
			delete[] message;
			glDeleteProgram(this->glShaderProgram);
			this->glShaderProgram = 0;
			return false;
		}
		return true;
	}

	OpenGLES_RenderSystem::OpenGLES_RenderSystem() : OpenGL_RenderSystem(), activeTextureColorMode(CM_DEFAULT),
		activeTextureColorModeAlpha(255)
	{
		this->activeShader = NULL;
		this->vertexShaderDefault = NULL;
		this->pixelShaderTexturedMultiply = NULL;
		this->pixelShaderTexturedAlphaMap = NULL;
		this->pixelShaderTexturedLerp = NULL;
		this->pixelShaderMultiply = NULL;
		this->pixelShaderAlphaMap = NULL;
		this->pixelShaderLerp = NULL;
		this->shaderTexturedMultiply = NULL;
		this->shaderTexturedAlphaMap = NULL;
		this->shaderTexturedLerp = NULL;
		this->shaderMultiply = NULL;
		this->shaderAlphaMap = NULL;
		this->shaderLerp = NULL;
		this->_currentShader = NULL;
	}

	OpenGLES_RenderSystem::~OpenGLES_RenderSystem()
	{
	}

	bool OpenGLES_RenderSystem::create(RenderSystem::Options options)
	{
		if (!OpenGL_RenderSystem::create(options))
		{
			return false;
		}
		this->activeTextureColorMode = CM_DEFAULT;
		this->activeTextureColorModeAlpha = 255;
		this->activeShader = NULL;
		this->vertexShaderDefault = NULL;
		this->pixelShaderTexturedMultiply = NULL;
		this->pixelShaderTexturedAlphaMap = NULL;
		this->pixelShaderTexturedLerp = NULL;
		this->pixelShaderMultiply = NULL;
		this->pixelShaderAlphaMap = NULL;
		this->pixelShaderLerp = NULL;
		this->shaderTexturedMultiply = NULL;
		this->shaderTexturedAlphaMap = NULL;
		this->shaderTexturedLerp = NULL;
		this->shaderMultiply = NULL;
		this->shaderAlphaMap = NULL;
		this->shaderLerp = NULL;
		this->_currentShader = NULL;
		return true;
	}

	bool OpenGLES_RenderSystem::destroy()
	{
		if (!OpenGL_RenderSystem::destroy())
		{
			return false;
		}
		this->activeShader = NULL;
		DELETE_SHADER(this->vertexShaderDefault, Vertex);
		DELETE_SHADER(this->pixelShaderTexturedMultiply, Pixel);
		DELETE_SHADER(this->pixelShaderTexturedAlphaMap, Pixel);
		DELETE_SHADER(this->pixelShaderTexturedLerp, Pixel);
		DELETE_SHADER(this->pixelShaderMultiply, Pixel);
		DELETE_SHADER(this->pixelShaderAlphaMap, Pixel);
		DELETE_SHADER(this->pixelShaderLerp, Pixel);
		_HL_TRY_DELETE(this->shaderTexturedMultiply);
		_HL_TRY_DELETE(this->shaderTexturedAlphaMap);
		_HL_TRY_DELETE(this->shaderTexturedLerp);
		_HL_TRY_DELETE(this->shaderMultiply);
		_HL_TRY_DELETE(this->shaderAlphaMap);
		_HL_TRY_DELETE(this->shaderLerp);
		this->_currentShader = NULL;
		return true;
	}

	void OpenGLES_RenderSystem::assignWindow(Window* window)
	{
		OpenGL_RenderSystem::assignWindow(window);
		hstream data;
		LOAD_SHADER(this->vertexShaderDefault, Vertex, Default, data);
		LOAD_SHADER(this->pixelShaderTexturedMultiply, Pixel, TexturedMultiply, data);
		LOAD_SHADER(this->pixelShaderTexturedAlphaMap, Pixel, TexturedAlphaMap, data);
		LOAD_SHADER(this->pixelShaderTexturedLerp, Pixel, TexturedLerp, data);
		LOAD_SHADER(this->pixelShaderMultiply, Pixel, Multiply, data);
		LOAD_SHADER(this->pixelShaderAlphaMap, Pixel, AlphaMap, data);
		LOAD_SHADER(this->pixelShaderLerp, Pixel, Lerp, data);
		LOAD_PROGRAM(this->shaderTexturedMultiply, this->pixelShaderTexturedMultiply, this->vertexShaderDefault);
		LOAD_PROGRAM(this->shaderTexturedAlphaMap, this->pixelShaderTexturedAlphaMap, this->vertexShaderDefault);
		LOAD_PROGRAM(this->shaderTexturedLerp, this->pixelShaderTexturedLerp, this->vertexShaderDefault);
		LOAD_PROGRAM(this->shaderMultiply, this->pixelShaderMultiply, this->vertexShaderDefault);
		LOAD_PROGRAM(this->shaderAlphaMap, this->pixelShaderAlphaMap, this->vertexShaderDefault);
		LOAD_PROGRAM(this->shaderLerp, this->pixelShaderLerp, this->vertexShaderDefault);
	}

	void OpenGLES_RenderSystem::_setupDefaultParameters()
	{
		OpenGL_RenderSystem::_setupDefaultParameters();
		this->_setClientState(VERTEX_ARRAY, true);
		this->_setClientState(COLOR_ARRAY, this->deviceState.colorEnabled);
		this->_setClientState(TEXCOORD_ARRAY, this->deviceState.textureCoordinatesEnabled);
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
		this->_updateShader(this->currentState.textureCoordinatesEnabled);
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
		OpenGL_RenderSystem::_setTextureBlendMode(textureBlendMode);
		// TODO
	}

	void OpenGLES_RenderSystem::_setTextureColorMode(ColorMode textureColorMode, float factor)
	{
		this->activeTextureColorModeAlpha = 255;
		switch (textureColorMode)
		{
		case CM_LERP: // LERP also needs alpha
			this->activeTextureColorModeAlpha = (unsigned char)(factor * 255.0f);
		case CM_DEFAULT:
		case CM_MULTIPLY:
		case CM_ALPHA_MAP:
			this->activeTextureColorMode = textureColorMode;
			break;
		default:
			hlog::warn(logTag, "Trying to set unsupported texture color mode!");
			break;
		}
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

	void OpenGLES_RenderSystem::setPixelShader(PixelShader* pixelShader)
	{
		// TODO
		//this->activePixelShader = (OpenGLES_PixelShader*)pixelShader;
	}

	void OpenGLES_RenderSystem::setVertexShader(VertexShader* vertexShader)
	{
		//this->activeVertexShader = (OpenGLES_VertexShader*)vertexShader;
	}

	void OpenGLES_RenderSystem::_updateShader(bool useTexture)
	{
		ShaderProgram* shader = this->activeShader;
		if (shader == NULL)
		{
			switch (this->activeTextureColorMode)
			{
			case CM_DEFAULT:
			case CM_MULTIPLY:
				shader = (useTexture ? this->shaderTexturedMultiply : this->shaderMultiply);
				break;
			case CM_ALPHA_MAP:
				shader = (useTexture ? this->shaderTexturedAlphaMap : this->shaderAlphaMap);
				break;
			case CM_LERP:
				shader = (useTexture ? this->shaderTexturedLerp : this->shaderLerp);
				break;
			}
		}
		if (this->_currentShader != shader)
		{
			this->_currentShader = shader;
			glUseProgram(this->_currentShader->glShaderProgram);
		}
	}

}
#endif
