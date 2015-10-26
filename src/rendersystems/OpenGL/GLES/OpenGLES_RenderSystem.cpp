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

#define MAX_VERTEX_COUNT 65536
#define VERTICES_BUFFER_COUNT 65536

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

#define _SELECT_SHADER(useTexture, useColor, type) \
	(useTexture ? (useColor ? this->shaderColoredTextured ## type : this->shaderTextured ## type) : (useColor ? this->shaderColored ## type : this->shader ## type));


namespace april
{
	static ColoredTexturedVertex static_ctv[VERTICES_BUFFER_COUNT];

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
			glGetProgramiv(this->glShaderProgram, GL_INFO_LOG_LENGTH, &messageSize);
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
		activeTextureColorModeAlpha(255), _matrixDirty(true)
	{
		this->activeShader = NULL;
		this->vertexShaderPlain = NULL;
		this->vertexShaderTextured = NULL;
		this->vertexShaderColored = NULL;
		this->vertexShaderColoredTextured = NULL;
		this->pixelShaderMultiply = NULL;
		this->pixelShaderAlphaMap = NULL;
		this->pixelShaderLerp = NULL;
		this->pixelShaderTexturedMultiply = NULL;
		this->pixelShaderTexturedAlphaMap = NULL;
		this->pixelShaderTexturedLerp = NULL;
		this->pixelShaderColoredMultiply = NULL;
		this->pixelShaderColoredAlphaMap = NULL;
		this->pixelShaderColoredLerp = NULL;
		this->pixelShaderColoredTexturedMultiply = NULL;
		this->pixelShaderColoredTexturedAlphaMap = NULL;
		this->pixelShaderColoredTexturedLerp = NULL;
		this->shaderMultiply = NULL;
		this->shaderAlphaMap = NULL;
		this->shaderLerp = NULL;
		this->shaderTexturedMultiply = NULL;
		this->shaderTexturedAlphaMap = NULL;
		this->shaderTexturedLerp = NULL;
		this->shaderColoredMultiply = NULL;
		this->shaderColoredAlphaMap = NULL;
		this->shaderColoredLerp = NULL;
		this->shaderColoredTexturedMultiply = NULL;
		this->shaderColoredTexturedAlphaMap = NULL;
		this->shaderColoredTexturedLerp = NULL;
		this->_currentShader = NULL;
		this->_currentTexture = NULL;
		this->_currentLerpAlpha = 0.0f;
		for_iter (i, 0, 4)
		{
			this->_currentSystemColor[i] = 1.0f;
		}
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
		this->_matrixDirty = true;
		this->vertexShaderPlain = NULL;
		this->vertexShaderTextured = NULL;
		this->vertexShaderColored = NULL;
		this->vertexShaderColoredTextured = NULL;
		this->pixelShaderMultiply = NULL;
		this->pixelShaderAlphaMap = NULL;
		this->pixelShaderLerp = NULL;
		this->pixelShaderTexturedMultiply = NULL;
		this->pixelShaderTexturedAlphaMap = NULL;
		this->pixelShaderTexturedLerp = NULL;
		this->pixelShaderColoredMultiply = NULL;
		this->pixelShaderColoredAlphaMap = NULL;
		this->pixelShaderColoredLerp = NULL;
		this->pixelShaderColoredTexturedMultiply = NULL;
		this->pixelShaderColoredTexturedAlphaMap = NULL;
		this->pixelShaderColoredTexturedLerp = NULL;
		this->shaderMultiply = NULL;
		this->shaderAlphaMap = NULL;
		this->shaderLerp = NULL;
		this->shaderTexturedMultiply = NULL;
		this->shaderTexturedAlphaMap = NULL;
		this->shaderTexturedLerp = NULL;
		this->shaderColoredMultiply = NULL;
		this->shaderColoredAlphaMap = NULL;
		this->shaderColoredLerp = NULL;
		this->shaderColoredTexturedMultiply = NULL;
		this->shaderColoredTexturedAlphaMap = NULL;
		this->shaderColoredTexturedLerp = NULL;
		this->_currentShader = NULL;
		this->_currentTexture = NULL;
		this->_currentLerpAlpha = 0.0f;
		for_iter (i, 0, 4)
		{
			this->_currentSystemColor[i] = 1.0f;
		}
		return true;
	}

	bool OpenGLES_RenderSystem::destroy()
	{
		if (!OpenGL_RenderSystem::destroy())
		{
			return false;
		}
		this->activeTextureColorMode = CM_DEFAULT;
		this->activeTextureColorModeAlpha = 255;
		this->activeShader = NULL;
		DELETE_SHADER(this->vertexShaderPlain, Vertex);
		DELETE_SHADER(this->vertexShaderTextured, Vertex);
		DELETE_SHADER(this->vertexShaderColored, Vertex);
		DELETE_SHADER(this->vertexShaderColoredTextured, Vertex);
		DELETE_SHADER(this->pixelShaderMultiply, Pixel);
		DELETE_SHADER(this->pixelShaderAlphaMap, Pixel);
		DELETE_SHADER(this->pixelShaderLerp, Pixel);
		DELETE_SHADER(this->pixelShaderColoredMultiply, Pixel);
		DELETE_SHADER(this->pixelShaderColoredAlphaMap, Pixel);
		DELETE_SHADER(this->pixelShaderColoredLerp, Pixel);
		DELETE_SHADER(this->pixelShaderTexturedMultiply, Pixel);
		DELETE_SHADER(this->pixelShaderTexturedAlphaMap, Pixel);
		DELETE_SHADER(this->pixelShaderTexturedLerp, Pixel);
		DELETE_SHADER(this->pixelShaderColoredTexturedMultiply, Pixel);
		DELETE_SHADER(this->pixelShaderColoredTexturedAlphaMap, Pixel);
		DELETE_SHADER(this->pixelShaderColoredTexturedLerp, Pixel);
		_HL_TRY_DELETE(this->shaderMultiply);
		_HL_TRY_DELETE(this->shaderAlphaMap);
		_HL_TRY_DELETE(this->shaderLerp);
		_HL_TRY_DELETE(this->shaderColoredMultiply);
		_HL_TRY_DELETE(this->shaderColoredAlphaMap);
		_HL_TRY_DELETE(this->shaderColoredLerp);
		_HL_TRY_DELETE(this->shaderTexturedMultiply);
		_HL_TRY_DELETE(this->shaderTexturedAlphaMap);
		_HL_TRY_DELETE(this->shaderTexturedLerp);
		_HL_TRY_DELETE(this->shaderColoredTexturedMultiply);
		_HL_TRY_DELETE(this->shaderColoredTexturedAlphaMap);
		_HL_TRY_DELETE(this->shaderColoredTexturedLerp);
		this->_currentShader = NULL;
		this->_currentTexture = NULL;
		this->_currentLerpAlpha = 0.0f;
		for_iter (i, 0, 4)
		{
			this->_currentSystemColor[i] = 1.0f;
		}
		return true;
	}

	void OpenGLES_RenderSystem::assignWindow(Window* window)
	{
		OpenGL_RenderSystem::assignWindow(window);
		hstream data;
		LOAD_SHADER(this->vertexShaderPlain, Vertex, Plain, data);
		LOAD_SHADER(this->vertexShaderTextured, Vertex, Textured, data);
		LOAD_SHADER(this->vertexShaderColored, Vertex, Colored, data);
		LOAD_SHADER(this->vertexShaderColoredTextured, Vertex, ColoredTextured, data);
		LOAD_SHADER(this->pixelShaderMultiply, Pixel, Multiply, data);
		LOAD_SHADER(this->pixelShaderAlphaMap, Pixel, AlphaMap, data);
		LOAD_SHADER(this->pixelShaderLerp, Pixel, Lerp, data);
		LOAD_SHADER(this->pixelShaderColoredMultiply, Pixel, ColoredMultiply, data);
		LOAD_SHADER(this->pixelShaderColoredAlphaMap, Pixel, ColoredAlphaMap, data);
		LOAD_SHADER(this->pixelShaderColoredLerp, Pixel, ColoredLerp, data);
		LOAD_SHADER(this->pixelShaderTexturedMultiply, Pixel, TexturedMultiply, data);
		LOAD_SHADER(this->pixelShaderTexturedAlphaMap, Pixel, TexturedAlphaMap, data);
		LOAD_SHADER(this->pixelShaderTexturedLerp, Pixel, TexturedLerp, data);
		LOAD_SHADER(this->pixelShaderColoredTexturedMultiply, Pixel, ColoredTexturedMultiply, data);
		LOAD_SHADER(this->pixelShaderColoredTexturedAlphaMap, Pixel, ColoredTexturedAlphaMap, data);
		LOAD_SHADER(this->pixelShaderColoredTexturedLerp, Pixel, ColoredTexturedLerp, data);
		LOAD_PROGRAM(this->shaderMultiply, this->pixelShaderMultiply, this->vertexShaderPlain);
		LOAD_PROGRAM(this->shaderAlphaMap, this->pixelShaderAlphaMap, this->vertexShaderPlain);
		LOAD_PROGRAM(this->shaderLerp, this->pixelShaderLerp, this->vertexShaderPlain);
		LOAD_PROGRAM(this->shaderTexturedMultiply, this->pixelShaderTexturedMultiply, this->vertexShaderTextured);
		LOAD_PROGRAM(this->shaderTexturedAlphaMap, this->pixelShaderTexturedAlphaMap, this->vertexShaderTextured);
		LOAD_PROGRAM(this->shaderTexturedLerp, this->pixelShaderTexturedLerp, this->vertexShaderTextured);
		LOAD_PROGRAM(this->shaderColoredMultiply, this->pixelShaderColoredMultiply, this->vertexShaderColored);
		LOAD_PROGRAM(this->shaderColoredAlphaMap, this->pixelShaderColoredAlphaMap, this->vertexShaderColored);
		LOAD_PROGRAM(this->shaderColoredLerp, this->pixelShaderColoredLerp, this->vertexShaderColored);
		LOAD_PROGRAM(this->shaderColoredTexturedMultiply, this->pixelShaderColoredTexturedMultiply, this->vertexShaderColoredTextured);
		LOAD_PROGRAM(this->shaderColoredTexturedAlphaMap, this->pixelShaderColoredTexturedAlphaMap, this->vertexShaderColoredTextured);
		LOAD_PROGRAM(this->shaderColoredTexturedLerp, this->pixelShaderColoredTexturedLerp, this->vertexShaderColoredTextured);
	}

	void OpenGLES_RenderSystem::_setupDefaultParameters()
	{
		OpenGL_RenderSystem::_setupDefaultParameters();
		this->_setClientState(VERTEX_ARRAY, true);
		this->_setClientState(COLOR_ARRAY, this->deviceState.colorEnabled);
		this->_setClientState(TEXCOORD_ARRAY, this->deviceState.textureCoordinatesEnabled);
	}

	void OpenGLES_RenderSystem::_applyStateChanges()
	{
		this->deviceState.systemColor = this->currentState.systemColor;
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
			this->deviceState.textureId = this->currentState.textureId;
			// TODO - should memorize address and filter modes per texture in opengl to avoid unnecessary calls
			this->deviceState.textureAddressMode = Texture::ADDRESS_UNDEFINED;
			this->deviceState.textureFilter = Texture::FILTER_UNDEFINED;
		}
		OpenGL_RenderSystem::_applyStateChanges();
		if (this->currentState.modelviewMatrixChanged && this->modelviewMatrix != this->deviceState.modelviewMatrix)
		{
			this->deviceState.modelviewMatrix = this->modelviewMatrix;
			this->currentState.modelviewMatrixChanged = false;
		}
		if (this->currentState.projectionMatrixChanged && this->projectionMatrix != this->deviceState.projectionMatrix)
		{
			this->deviceState.projectionMatrix = this->projectionMatrix;
			this->currentState.projectionMatrixChanged = false;
		}
		this->_updateShader();
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
#ifndef _IOS // platforms other than iOS have problems with alpha textures, some drivers don't support them
		this->caps.textureFormats /= Image::FORMAT_ALPHA;
		this->caps.textureFormats /= Image::FORMAT_GRAYSCALE;
#endif
		return OpenGL_RenderSystem::_setupCaps();
	}

	void OpenGLES_RenderSystem::_setTextureBlendMode(BlendMode textureBlendMode)
	{
		// TODO - is there a way to make this work on Win32?
#ifndef _WIN32
		// TODO - refactor
		static int blendSeparationSupported = -1;
		if (blendSeparationSupported == -1)
		{
			// determine if blend separation is possible on first call to this function
			hstr extensions = (const char*)glGetString(GL_EXTENSIONS);
			blendSeparationSupported = (extensions.contains("OES_blend_equation_separate") && extensions.contains("OES_blend_func_separate"));
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
	}

	void OpenGLES_RenderSystem::_loadIdentityMatrix()
	{
		// not used
	}
	
	void OpenGLES_RenderSystem::_setMatrixMode(unsigned int mode)
	{
		// not used
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
			glVertexAttribPointer(TEXCOORD_ARRAY, 2, GL_FLOAT, GL_TRUE, stride, pointer);
		}
	}

	void OpenGLES_RenderSystem::_setColorPointer(int stride, const void *pointer)
	{
		if (this->deviceState.strideColor != stride || this->deviceState.pointerColor != pointer)
		{
			this->deviceState.strideColor = stride;
			this->deviceState.pointerColor = pointer;
			glVertexAttribPointer(COLOR_ARRAY, 4, GL_UNSIGNED_BYTE, GL_TRUE, stride, pointer);
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

	void OpenGLES_RenderSystem::_updateShader()
	{
		bool useTexture = this->deviceState.textureCoordinatesEnabled;
		bool useColor = this->deviceState.colorEnabled;
		ShaderProgram* shader = this->activeShader;
		if (shader == NULL)
		{
			switch (this->activeTextureColorMode)
			{
			case CM_DEFAULT:
			case CM_MULTIPLY:
				shader = _SELECT_SHADER(useTexture, useColor, Multiply);
				break;
			case CM_ALPHA_MAP:
				shader = _SELECT_SHADER(useTexture, useColor, AlphaMap);
				break;
			case CM_LERP:
				shader = _SELECT_SHADER(useTexture, useColor, Lerp);
				break;
			}
		}
		bool dirty = false;
		if (this->_currentShader != shader)
		{
			this->_currentShader = shader;
			glUseProgram(this->_currentShader->glShaderProgram);
			dirty = true;
		}
		static gmat4 transformationMatrix = this->deviceState.projectionMatrix * this->deviceState.modelviewMatrix;
		static float lerpAlpha = 1.0f;
		static float systemColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		lerpAlpha = this->activeTextureColorModeAlpha * 0.003921569f;
		systemColor[0] = this->deviceState.systemColor.r_f();
		systemColor[1] = this->deviceState.systemColor.g_f();
		systemColor[2] = this->deviceState.systemColor.b_f();
		systemColor[3] = this->deviceState.systemColor.a_f();
		if (!dirty)
		{
			dirty = this->_matrixDirty;
		}
		if (!dirty)
		{
			dirty = (this->_currentLerpAlpha != lerpAlpha);
		}
		if (!dirty)
		{
			for_iter (i, 0, 4)
			{
				if (this->_currentSystemColor[i] != systemColor[i])
				{
					dirty = true;
					break;
				}
			}
		}
		this->_matrixDirty = false;
		if (dirty)
		{
			// set current values
			transformationMatrix = this->deviceState.projectionMatrix * this->deviceState.modelviewMatrix;;
			this->_currentLerpAlpha = lerpAlpha;
			memcpy(this->_currentSystemColor, systemColor, 4 * sizeof(float));
			// set shader values
			int matrixLocation = glGetUniformLocation(this->_currentShader->glShaderProgram, "transformationMatrix");
			glUniformMatrix4fv(matrixLocation, 1, GL_FALSE, transformationMatrix.data);
			if (useTexture)
			{
				glActiveTexture(GL_TEXTURE0);
			}
			int samplerLocation = glGetUniformLocation(this->_currentShader->glShaderProgram, "sampler2d");
			if (samplerLocation >= 0)
			{
				glUniform1i(samplerLocation, 0);
			}
			int systemColorLocation = glGetUniformLocation(this->_currentShader->glShaderProgram, "systemColor");
			if (systemColorLocation >= 0)
			{
				glUniform4fv(systemColorLocation, 1, this->_currentSystemColor);
			}
			int lerpLocation = glGetUniformLocation(this->_currentShader->glShaderProgram, "lerpAlpha");
			if (lerpLocation >= 0)
			{
				glUniform1fv(lerpLocation, 1, &this->_currentLerpAlpha);
			}
		}
	}

	void OpenGLES_RenderSystem::render(RenderOperation renderOperation, PlainVertex* v, int nVertices)
	{
		this->render(renderOperation, v, nVertices, april::Color::White);
	}

	void OpenGLES_RenderSystem::render(RenderOperation renderOperation, PlainVertex* v, int nVertices, Color color)
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

	void OpenGLES_RenderSystem::render(RenderOperation renderOperation, TexturedVertex* v, int nVertices)
	{
		this->render(renderOperation, v, nVertices, april::Color::White);
	}

	void OpenGLES_RenderSystem::render(RenderOperation renderOperation, TexturedVertex* v, int nVertices, Color color)
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

	void OpenGLES_RenderSystem::render(RenderOperation renderOperation, ColoredVertex* v, int nVertices)
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

	void OpenGLES_RenderSystem::render(RenderOperation renderOperation, ColoredTexturedVertex* v, int nVertices)
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

	void OpenGLES_RenderSystem::_setModelviewMatrix(const gmat4& matrix)
	{
		OpenGL_RenderSystem::_setModelviewMatrix(matrix);
		this->_matrixDirty = true;
	}

	void OpenGLES_RenderSystem::_setProjectionMatrix(const gmat4& matrix)
	{
		OpenGL_RenderSystem::_setProjectionMatrix(matrix);
		this->_matrixDirty = true;
	}

}
#endif
