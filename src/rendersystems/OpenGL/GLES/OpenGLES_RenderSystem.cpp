/// @file
/// @version 4.0
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

#define VERTEX_ARRAY 0
#define COLOR_ARRAY 1
#define TEXTURE_ARRAY 2

#define DELETE_SHADER(name, type) \
	if (name != NULL) \
	{ \
		this->destroy ## type ## Shader(name); \
		name = NULL; \
	}

#define LOAD_SHADER(name, type, mode, data) \
	if (name == NULL) \
	{ \
		data.clear(); \
		data.write(SHADER_ ## type ## mode); \
		name = (OpenGLES_ ## type ## Shader*)this->_deviceCreate ## type ## Shader(); \
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
		glBindAttribLocation(this->glShaderProgram, TEXTURE_ARRAY, "tex");
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

	OpenGLES_RenderSystem::OpenGLES_RenderSystem() : OpenGL_RenderSystem(), deviceState_matrixChanged(true), 
		deviceState_systemColorChanged(true), deviceState_colorModeFactorChanged(true)
	{
	}

	OpenGLES_RenderSystem::~OpenGLES_RenderSystem()
	{
	}

	void OpenGLES_RenderSystem::_deviceInit()
	{
		OpenGL_RenderSystem::_deviceInit();
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
		this->deviceState_matrixChanged = true;
		this->deviceState_systemColorChanged = true;
		this->deviceState_colorModeFactorChanged = true;
		this->deviceState_shader = NULL;
	}

	bool OpenGLES_RenderSystem::_deviceCreate(RenderSystem::Options options)
	{
		if (!OpenGL_RenderSystem::_deviceCreate(options))
		{
			return false;
		}
		return true;
	}

	bool OpenGLES_RenderSystem::_deviceDestroy()
	{
		if (!OpenGL_RenderSystem::_deviceDestroy())
		{
			return false;
		}
		this->_destroyShaders();
		return true;
	}

	void OpenGLES_RenderSystem::_deviceAssignWindow(Window* window)
	{
		OpenGL_RenderSystem::_deviceAssignWindow(window);
		this->_createShaders();
	}

	void OpenGLES_RenderSystem::_deviceReset()
	{
		this->_createShaders();
		OpenGL_RenderSystem::_deviceReset();
	}

	void OpenGLES_RenderSystem::_deviceSuspend()
	{
		OpenGL_RenderSystem::_deviceSuspend();
		this->unloadTextures();
		this->_destroyShaders();
	}

	void OpenGLES_RenderSystem::_deviceSetupCaps()
	{
#ifdef _EGL
		if (april::egl->display == NULL)
		{
			return;
		}
#endif
		hstr extensions = (const char*)glGetString(GL_EXTENSIONS);
#ifndef _WINRT
		this->caps.npotTexturesLimited = (extensions.contains("IMG_texture_npot") || extensions.contains("APPLE_texture_2D_limited_npot"));
#else
		this->caps.npotTexturesLimited = true;
#endif
		this->caps.npotTextures = (extensions.contains("OES_texture_npot") || extensions.contains("ARB_texture_non_power_of_two"));
		// TODO - is there a way to make this work on Win32?
#ifndef _WIN32
		this->blendSeparationSupported = (extensions.contains("OES_blend_equation_separate") && extensions.contains("OES_blend_func_separate"));
#endif
#ifndef _IOS // platforms other than iOS have problems with alpha textures, some drivers don't support them
		this->caps.textureFormats /= Image::FORMAT_ALPHA;
		this->caps.textureFormats /= Image::FORMAT_GRAYSCALE;
#endif
		return OpenGL_RenderSystem::_deviceSetupCaps();
	}

	void OpenGLES_RenderSystem::_deviceSetup()
	{
		OpenGL_RenderSystem::_deviceSetup();
		glEnableVertexAttribArray(VERTEX_ARRAY);
		this->deviceState_matrixChanged = true;
		this->deviceState_systemColorChanged = true;
		this->deviceState_colorModeFactorChanged = true;
		this->deviceState_shader = NULL;
	}

	void OpenGLES_RenderSystem::_createShaders()
	{
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

	void OpenGLES_RenderSystem::_destroyShaders()
	{
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
	}

	void OpenGLES_RenderSystem::_updateDeviceState(bool forceUpdate)
	{
		OpenGL_RenderSystem::_updateDeviceState(forceUpdate);
		this->_updateShader(forceUpdate);
	}

	void OpenGLES_RenderSystem::_setDeviceModelviewMatrix(const gmat4& matrix)
	{
		this->deviceState_matrixChanged = true;
	}

	void OpenGLES_RenderSystem::_setDeviceProjectionMatrix(const gmat4& matrix)
	{
		this->deviceState_matrixChanged = true;
	}

	void OpenGLES_RenderSystem::_setDeviceBlendMode(BlendMode blendMode)
	{
#ifndef _WIN32
		if (this->blendSeparationSupported)
		{
			// blending for the new generations
			switch (blendMode)
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
			OpenGL_RenderSystem::_setDeviceBlendMode(blendMode);
		}
	}

	void OpenGLES_RenderSystem::_setDeviceColorMode(ColorMode colorMode, float colorModeFactor, bool useTexture, bool useColor, const Color& systemColor)
	{
		if (this->deviceState->systemColor != systemColor)
		{
			this->deviceState_systemColorChanged = true;
		}
		if (this->deviceState->colorModeFactor != colorModeFactor)
		{
			this->deviceState_colorModeFactorChanged = true;
		}
	}

	void OpenGLES_RenderSystem::_updateShader(bool forceUpdate)
	{
		ShaderProgram* shader = NULL;
		switch (this->deviceState->colorMode)
		{
		case CM_DEFAULT:
		case CM_MULTIPLY:
			shader = _SELECT_SHADER(this->deviceState->useTexture, this->deviceState->useColor, Multiply);
			break;
		case CM_ALPHA_MAP:
			shader = _SELECT_SHADER(this->deviceState->useTexture, this->deviceState->useColor, AlphaMap);
			break;
		case CM_LERP:
			shader = _SELECT_SHADER(this->deviceState->useTexture, this->deviceState->useColor, Lerp);
			break;
		default:
			hlog::warn(logTag, "Trying to set unsupported color mode!");
			return;
		}
		if (this->deviceState_shader != shader)
		{
			forceUpdate = true;
		}
		if (forceUpdate)
		{
			glUseProgram(shader->glShaderProgram);
			this->deviceState_shader = shader;
			if (this->deviceState->useTexture)
			{
				glActiveTexture(GL_TEXTURE0);
			}
			int samplerLocation = glGetUniformLocation(this->deviceState_shader->glShaderProgram, "sampler2d");
			if (samplerLocation >= 0)
			{
				glUniform1i(samplerLocation, 0);
			}
		}
		if (forceUpdate || this->deviceState_matrixChanged)
		{
			int matrixLocation = glGetUniformLocation(this->deviceState_shader->glShaderProgram, "transformationMatrix");
			glUniformMatrix4fv(matrixLocation, 1, GL_FALSE, (this->deviceState->projectionMatrix * this->deviceState->modelviewMatrix).data);
			this->deviceState_matrixChanged = false;
		}
		if (forceUpdate || this->deviceState_systemColorChanged)
		{
			int systemColorLocation = glGetUniformLocation(this->deviceState_shader->glShaderProgram, "systemColor");
			if (systemColorLocation >= 0)
			{
				static float shaderSystemColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
				shaderSystemColor[0] = this->deviceState->systemColor.r_f();
				shaderSystemColor[1] = this->deviceState->systemColor.g_f();
				shaderSystemColor[2] = this->deviceState->systemColor.b_f();
				shaderSystemColor[3] = this->deviceState->systemColor.a_f();
				glUniform4fv(systemColorLocation, 1, shaderSystemColor);
			}
			this->deviceState_systemColorChanged = false;
		}
		if (forceUpdate || this->deviceState_colorModeFactorChanged)
		{
			int lerpLocation = glGetUniformLocation(this->deviceState_shader->glShaderProgram, "lerpAlpha");
			if (lerpLocation >= 0)
			{
				static float shaderColorModeFactor = 1.0f;
				shaderColorModeFactor = this->deviceState->colorModeFactor;
				glUniform1fv(lerpLocation, 1, &shaderColorModeFactor);
			}
			this->deviceState_colorModeFactorChanged = false;
		}
	}

	void OpenGLES_RenderSystem::_setGlTextureEnabled(bool enabled)
	{
		enabled ? glEnableVertexAttribArray(TEXTURE_ARRAY) : glDisableVertexAttribArray(TEXTURE_ARRAY);
	}

	void OpenGLES_RenderSystem::_setGlColorEnabled(bool enabled)
	{
		enabled ? glEnableVertexAttribArray(COLOR_ARRAY) : glDisableVertexAttribArray(COLOR_ARRAY);
	}

	void OpenGLES_RenderSystem::_setGlVertexPointer(int stride, const void* pointer)
	{
		glVertexAttribPointer(VERTEX_ARRAY, 3, GL_FLOAT, GL_FALSE, stride, pointer);
	}

	void OpenGLES_RenderSystem::_setGlTexturePointer(int stride, const void* pointer)
	{
		glVertexAttribPointer(TEXTURE_ARRAY, 2, GL_FLOAT, GL_TRUE, stride, pointer);
	}

	void OpenGLES_RenderSystem::_setGlColorPointer(int stride, const void* pointer)
	{
		glVertexAttribPointer(COLOR_ARRAY, 4, GL_UNSIGNED_BYTE, GL_TRUE, stride, pointer);
	}

}
#endif
