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

namespace april
{
	static ColoredTexturedVertex static_ctv[VERTICES_BUFFER_COUNT];

	static unsigned int systemColor = 0;

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
		glUniform1i(glGetUniformLocation(this->glShaderProgram, "sampler2d"), 0);
		return true;
	}

	OpenGLES_RenderSystem::OpenGLES_RenderSystem() : OpenGL_RenderSystem(), activeTextureColorMode(CM_DEFAULT),
		activeTextureColorModeAlpha(255), _matrixDirty(true)
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
		this->activeSystemColor = april::Color::White;
		this->activeShader = NULL;
		this->_matrixDirty = true;
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
		this->activeSystemColor = april::Color::White;
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
		this->activeSystemColor = this->deviceState.systemColor;
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
				glBlendEquationSeparateOES(GL_FUNC_ADD_OES, GL_FUNC_ADD_OES);
				glBlendFuncSeparateOES(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
				break;
			case BM_ADD:
				glBlendEquationSeparateOES(GL_FUNC_ADD_OES, GL_FUNC_ADD_OES);
				glBlendFuncSeparateOES(GL_SRC_ALPHA, GL_ONE, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
				break;
			case BM_SUBTRACT:
				glBlendEquationSeparateOES(GL_FUNC_REVERSE_SUBTRACT_OES, GL_FUNC_ADD_OES);
				glBlendFuncSeparateOES(GL_SRC_ALPHA, GL_ONE, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
				break;
			case BM_OVERWRITE:
				glBlendEquationSeparateOES(GL_FUNC_ADD_OES, GL_FUNC_ADD_OES);
				glBlendFuncSeparateOES(GL_ONE, GL_ZERO, GL_ONE, GL_ZERO);
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
		bool dirty = false;
		if (this->_currentShader != shader)
		{
			this->_currentShader = shader;
			glUseProgram(this->_currentShader->glShaderProgram);
			if (useTexture)
			{
				glActiveTexture(GL_TEXTURE0);
			}
			dirty = true;
		}
		static gmat4 transformationMatrix = this->projectionMatrix * this->modelviewMatrix;
		static float lerpAlpha = 1.0f;
		static float systemColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		lerpAlpha = this->activeTextureColorModeAlpha * 0.003921569f;
		systemColor[0] = this->activeSystemColor.r * 0.003921569f;
		systemColor[1] = this->activeSystemColor.g * 0.003921569f;
		systemColor[2] = this->activeSystemColor.b * 0.003921569f;
		systemColor[3] = this->activeSystemColor.a * 0.003921569f;
		dirty |= this->_matrixDirty;
		dirty |= (this->_currentLerpAlpha != lerpAlpha);
		for_iter (i, 0, 4)
		{
			dirty |= (this->_currentSystemColor[i] != systemColor[i]);
		}
		this->_matrixDirty = false;
		if (dirty)
		{
			transformationMatrix = this->projectionMatrix * this->modelviewMatrix;
		}
		if (dirty)
		{
			int matrixLocation = glGetUniformLocation(this->_currentShader->glShaderProgram, "transformationMatrix");
			glUniformMatrix4fv(matrixLocation, 1, GL_FALSE, transformationMatrix.data);
			int lerpLocation = glGetUniformLocation(this->_currentShader->glShaderProgram, "lerpAlpha");
			glUniform1fv(lerpLocation, 1, &this->_currentLerpAlpha);
			int systemColorLocation = glGetUniformLocation(this->_currentShader->glShaderProgram, "systemColor");
			glUniform1fv(systemColorLocation, 4, this->_currentSystemColor);
		}
	}

	void OpenGLES_RenderSystem::render(RenderOperation renderOperation, PlainVertex* v, int nVertices)
	{
		this->render(renderOperation, v, nVertices, april::Color::White);
	}

	void OpenGLES_RenderSystem::render(RenderOperation renderOperation, PlainVertex* v, int nVertices, Color color)
	{
		ColoredTexturedVertex* ctv = (nVertices <= VERTICES_BUFFER_COUNT) ? static_ctv : new ColoredTexturedVertex[nVertices];
		unsigned int c = this->getNativeColorUInt(color);
		for_iter (i, 0, nVertices)
		{
			ctv[i].x = v[i].x;
			ctv[i].y = v[i].y;
			ctv[i].z = v[i].z;
			ctv[i].color = c;
		}
		this->currentState.textureId = 0;
		this->currentState.textureCoordinatesEnabled = false;
		this->currentState.colorEnabled = false;
		/* TODO - remove */ this->currentState.colorEnabled = true;
		this->currentState.systemColor = color;
		this->_applyStateChanges();
		// This kind of approach to render chunks of vertices is caused by problems on OpenGLES
		// hardware that may allow only a certain amount of vertices to be rendered at the time.
		// Apparently that number is 65536 on HTC Evo 3D so this is used for MAX_VERTEX_COUNT by default.
		ColoredTexturedVertex* dv = ctv;
		int size = nVertices;
#ifdef _ANDROID
		for_iter_step (i, 0, nVertices, size)
		{
			size = this->_limitPrimitives(renderOperation, hmin(nVertices - i, MAX_VERTEX_COUNT));
#endif
			this->_setVertexPointer(sizeof(ColoredTexturedVertex), dv);
			this->_setColorPointer(sizeof(ColoredTexturedVertex), &dv->color);
			this->_setTexCoordPointer(sizeof(ColoredTexturedVertex), &dv->u);
			glDrawArrays(glRenderOperations[renderOperation], 0, size);
#ifdef _ANDROID
			dv += size;
		}
#endif
		if (nVertices > VERTICES_BUFFER_COUNT)
		{
			delete[] ctv;
		}
	}

	void OpenGLES_RenderSystem::render(RenderOperation renderOperation, TexturedVertex* v, int nVertices)
	{
		this->render(renderOperation, v, nVertices, april::Color::White);
	}

	void OpenGLES_RenderSystem::render(RenderOperation renderOperation, TexturedVertex* v, int nVertices, Color color)
	{
		ColoredTexturedVertex* ctv = (nVertices <= VERTICES_BUFFER_COUNT) ? static_ctv : new ColoredTexturedVertex[nVertices];
		unsigned int c = this->getNativeColorUInt(color);
		for_iter (i, 0, nVertices)
		{
			ctv[i].x = v[i].x;
			ctv[i].y = v[i].y;
			ctv[i].z = v[i].z;
			ctv[i].u = v[i].u;
			ctv[i].v = v[i].v;
			ctv[i].color = c;
		}
		this->currentState.textureCoordinatesEnabled = true;
		this->currentState.colorEnabled = false;
		/* TODO - remove */ this->currentState.colorEnabled = true;
		this->currentState.systemColor = color;
		this->_applyStateChanges();
		// This kind of approach to render chunks of vertices is caused by problems on OpenGLES
		// hardware that may allow only a certain amount of vertices to be rendered at the time.
		// Apparently that number is 65536 on HTC Evo 3D so this is used for MAX_VERTEX_COUNT by default.
		ColoredTexturedVertex* dv = ctv;
		int size = nVertices;
#ifdef _ANDROID
		for_iter_step (i, 0, nVertices, size)
		{
			size = this->_limitPrimitives(renderOperation, hmin(nVertices - i, MAX_VERTEX_COUNT));
#endif
			this->_setVertexPointer(sizeof(ColoredTexturedVertex), dv);
			this->_setColorPointer(sizeof(ColoredTexturedVertex), &dv->color);
			this->_setTexCoordPointer(sizeof(ColoredTexturedVertex), &dv->u);
			glDrawArrays(glRenderOperations[renderOperation], 0, size);
#ifdef _ANDROID
			dv += size;
		}
#endif
		if (nVertices > VERTICES_BUFFER_COUNT)
		{
			delete[] ctv;
		}
	}

	void OpenGLES_RenderSystem::render(RenderOperation renderOperation, ColoredVertex* v, int nVertices)
	{
		ColoredTexturedVertex* ctv = (nVertices <= VERTICES_BUFFER_COUNT) ? static_ctv : new ColoredTexturedVertex[nVertices];
		for_iter (i, 0, nVertices)
		{
			ctv[i].x = v[i].x;
			ctv[i].y = v[i].y;
			ctv[i].z = v[i].z;
			ctv[i].color = v[i].color;
		}
		this->currentState.textureId = 0;
		this->currentState.textureCoordinatesEnabled = false;
		this->currentState.colorEnabled = true;
		/* TODO - remove */ this->currentState.colorEnabled = true;
		this->currentState.systemColor.set(255, 255, 255, 255);
		this->_applyStateChanges();
		// This kind of approach to render chunks of vertices is caused by problems on OpenGLES
		// hardware that may allow only a certain amount of vertices to be rendered at the time.
		// Apparently that number is 65536 on HTC Evo 3D so this is used for MAX_VERTEX_COUNT by default.
		ColoredTexturedVertex* dv = ctv;
		int size = nVertices;
#ifdef _ANDROID
		for_iter_step (i, 0, nVertices, size)
		{
			size = this->_limitPrimitives(renderOperation, hmin(nVertices - i, MAX_VERTEX_COUNT));
#endif
			this->_setVertexPointer(sizeof(ColoredTexturedVertex), dv);
			this->_setColorPointer(sizeof(ColoredTexturedVertex), &dv->color);
			this->_setTexCoordPointer(sizeof(ColoredTexturedVertex), &dv->u);
			glDrawArrays(glRenderOperations[renderOperation], 0, size);
#ifdef _ANDROID
			dv += size;
		}
#endif
		if (nVertices > VERTICES_BUFFER_COUNT)
		{
			delete[] ctv;
		}
	}

	void OpenGLES_RenderSystem::render(RenderOperation renderOperation, ColoredTexturedVertex* v, int nVertices)
	{
		ColoredTexturedVertex* ctv = (nVertices <= VERTICES_BUFFER_COUNT) ? static_ctv : new ColoredTexturedVertex[nVertices];
		for_iter (i, 0, nVertices)
		{
			ctv[i].x = v[i].x;
			ctv[i].y = v[i].y;
			ctv[i].z = v[i].z;
			ctv[i].u = v[i].u;
			ctv[i].v = v[i].v;
			ctv[i].color = v[i].color;
		}
		this->currentState.textureCoordinatesEnabled = true;
		this->currentState.colorEnabled = true;
		this->currentState.systemColor.set(255, 255, 255, 255);
		this->_applyStateChanges();
		// This kind of approach to render chunks of vertices is caused by problems on OpenGLES
		// hardware that may allow only a certain amount of vertices to be rendered at the time.
		// Apparently that number is 65536 on HTC Evo 3D so this is used for MAX_VERTEX_COUNT by default.
		ColoredTexturedVertex* dv = ctv;
		int size = nVertices;
#ifdef _ANDROID
		for_iter_step (i, 0, nVertices, size)
		{
			size = this->_limitPrimitives(renderOperation, hmin(nVertices - i, MAX_VERTEX_COUNT));
#endif
			this->_setVertexPointer(sizeof(ColoredTexturedVertex), dv);
			this->_setColorPointer(sizeof(ColoredTexturedVertex), &dv->color);
			this->_setTexCoordPointer(sizeof(ColoredTexturedVertex), &dv->u);
			glDrawArrays(glRenderOperations[renderOperation], 0, size);
#ifdef _ANDROID
			dv += size;
		}
#endif
		if (nVertices > VERTICES_BUFFER_COUNT)
		{
			delete[] ctv;
		}
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
