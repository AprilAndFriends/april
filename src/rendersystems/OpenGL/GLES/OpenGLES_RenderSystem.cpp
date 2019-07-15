/// @file
/// @version 5.2
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

#include "Application.h"
#include "april.h"
#ifdef _EGL
#include "egl.h"
#endif
#include "OpenGLES_defaultShaders.h"
#include "OpenGLES_PixelShader.h"
#include "OpenGLES_RenderSystem.h"
#include "OpenGLES_Texture.h"
#include "OpenGLES_VertexShader.h"
#include "RenderState.h"
#include "Window.h"

#define VERTEX_ARRAY 0
#define COLOR_ARRAY 1
#define TEXTURE_ARRAY 2

#define DELETE_SHADER(type, name) \
	if (name != NULL) \
	{ \
		this->destroy ## type ## Shader(name); \
		name = NULL; \
	}

#define DELETE_VERTEX_SHADER(name) DELETE_SHADER(Vertex, name)
#define DELETE_PIXEL_SHADER(name) DELETE_SHADER(Pixel, name)

#define LOAD_SHADER(type, name, mode, data) \
	if (name == NULL) \
	{ \
		data.clear(); \
		data.write(SHADER_ ## type ## mode); \
		name = (OpenGLES_ ## type ## Shader*)this->_deviceCreate ## type ## Shader(); \
		name->load(data); \
	}

#define LOAD_VERTEX_SHADER(name, mode, data) LOAD_SHADER(Vertex, name, mode, data)
#define LOAD_PIXEL_SHADER(name, mode, data) LOAD_SHADER(Pixel, name, mode, data)

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
		if (this->glShaderProgram != 0 && april::rendersys->canUseLowLevelCalls())
		{
			glDeleteProgram(this->glShaderProgram);
		}
	}

	bool OpenGLES_RenderSystem::ShaderProgram::load(unsigned int pixelShaderId, unsigned int vertexShaderId)
	{
		if (!april::rendersys->canUseLowLevelCalls())
		{
			return false;
		}
		if (this->glShaderProgram != 0)
		{
			hlog::error(logTag, "Shader program already created!");
			return false;
		}
		GL_SAFE_CALL(this->glShaderProgram = glCreateProgram, ());
		if (this->glShaderProgram == 0)
		{
			hlog::error(logTag, "Could not create shader program!");
			return false;
		}
		GL_SAFE_CALL(glAttachShader, (this->glShaderProgram, pixelShaderId));
		GL_SAFE_CALL(glAttachShader, (this->glShaderProgram, vertexShaderId));
		GL_SAFE_CALL(glBindAttribLocation, (this->glShaderProgram, VERTEX_ARRAY, "position"));
		GL_SAFE_CALL(glBindAttribLocation, (this->glShaderProgram, COLOR_ARRAY, "color"));
		GL_SAFE_CALL(glBindAttribLocation, (this->glShaderProgram, TEXTURE_ARRAY, "tex"));
		GL_SAFE_CALL(glLinkProgram, (this->glShaderProgram));
		GLint linked;
		GL_SAFE_CALL(glGetProgramiv, (this->glShaderProgram, GL_LINK_STATUS, &linked));
		if (linked == 0)
		{
			int messageSize = 0;
			glGetProgramiv(this->glShaderProgram, GL_INFO_LOG_LENGTH, &messageSize);
			if (messageSize > 0)
			{
				int written = 0;
				char* message = new char[messageSize];
				glGetProgramInfoLog(this->glShaderProgram, messageSize, &written, message);
				hlog::error(logTag, "Shader Program could not be linked! Error:\n" + hstr(message));
				delete[] message;
			}
			else
			{
				hlog::error(logTag, "Shader Program could not be linked! Error message could not be obtained!");
			}
			glDeleteProgram(this->glShaderProgram);
			this->glShaderProgram = 0;
			return false;
		}
		return true;
	}

	OpenGLES_RenderSystem::OpenGLES_RenderSystem() : OpenGL_RenderSystem(), deviceState_matrixChanged(true), deviceState_systemColorChanged(true),
		deviceState_colorModeFactorChanged(true), framebufferId(0), renderbufferId(0), renderTarget(NULL), deviceState_shader(NULL)
	{
#ifdef __ANDROID__
		this->etc1Supported = false;
#endif
		this->caps.renderTarget = true;
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
		this->pixelShaderDesaturate = NULL;
		this->pixelShaderSepia = NULL;
		this->pixelShaderTexturedMultiply = NULL;
		this->pixelShaderTexturedAlphaMap = NULL;
		this->pixelShaderTexturedLerp = NULL;
		this->pixelShaderTexturedDesaturate = NULL;
		this->pixelShaderTexturedSepia = NULL;
		this->pixelShaderColoredMultiply = NULL;
		this->pixelShaderColoredAlphaMap = NULL;
		this->pixelShaderColoredLerp = NULL;
		this->pixelShaderColoredDesaturate = NULL;
		this->pixelShaderColoredSepia = NULL;
		this->pixelShaderColoredTexturedMultiply = NULL;
		this->pixelShaderColoredTexturedAlphaMap = NULL;
		this->pixelShaderColoredTexturedLerp = NULL;
		this->pixelShaderColoredTexturedDesaturate = NULL;
		this->pixelShaderColoredTexturedSepia = NULL;
#ifdef __ANDROID__
		this->pixelShaderTexturedMultiply_AlphaHack = NULL;
		this->pixelShaderTexturedLerp_AlphaHack = NULL;
		this->pixelShaderTexturedDesaturate_AlphaHack = NULL;
		this->pixelShaderTexturedSepia_AlphaHack = NULL;
		this->pixelShaderColoredTexturedMultiply_AlphaHack = NULL;
		this->pixelShaderColoredTexturedLerp_AlphaHack = NULL;
		this->pixelShaderColoredTexturedDesaturate_AlphaHack = NULL;
		this->pixelShaderColoredTexturedSepia_AlphaHack = NULL;
#endif
		this->shaderMultiply = NULL;
		this->shaderAlphaMap = NULL;
		this->shaderLerp = NULL;
		this->shaderDesaturate = NULL;
		this->shaderSepia = NULL;
		this->shaderTexturedMultiply = NULL;
		this->shaderTexturedAlphaMap = NULL;
		this->shaderTexturedLerp = NULL;
		this->shaderTexturedDesaturate = NULL;
		this->shaderTexturedSepia = NULL;
		this->shaderColoredMultiply = NULL;
		this->shaderColoredAlphaMap = NULL;
		this->shaderColoredLerp = NULL;
		this->shaderColoredDesaturate = NULL;
		this->shaderColoredSepia = NULL;
		this->shaderColoredTexturedMultiply = NULL;
		this->shaderColoredTexturedAlphaMap = NULL;
		this->shaderColoredTexturedLerp = NULL;
		this->shaderColoredTexturedDesaturate = NULL;
		this->shaderColoredTexturedSepia = NULL;
#ifdef __ANDROID__
		this->shaderTexturedMultiply_AlphaHack = NULL;
		this->shaderTexturedLerp_AlphaHack = NULL;
		this->shaderTexturedDesaturate_AlphaHack = NULL;
		this->shaderTexturedSepia_AlphaHack = NULL;
		this->shaderColoredTexturedMultiply_AlphaHack = NULL;
		this->shaderColoredTexturedLerp_AlphaHack = NULL;
		this->shaderColoredTexturedDesaturate_AlphaHack = NULL;
		this->shaderColoredTexturedSepia_AlphaHack = NULL;
#endif
		this->deviceState_matrixChanged = true;
		this->deviceState_systemColorChanged = true;
		this->deviceState_colorModeFactorChanged = true;
		this->deviceState_shader = NULL;
		this->renderTarget = NULL;
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
		this->framebufferId = 0;
		this->renderbufferId = 0;
		this->_destroyShaders();
		return true;
	}

	void OpenGLES_RenderSystem::_deviceAssignWindow(Window* window)
	{
		OpenGL_RenderSystem::_deviceAssignWindow(window);
#ifdef _ANDROID
		if (window->getName() == WindowType::AndroidJNI.getName() && !this->options.intermediateRenderTexture)
		{
			hlog::errorf(logTag, "The render system option 'intermediate render texture' must be enabled when using window type '%s' on Android! Otherwise rendering issues WILL happen!", window->getName().cStr());
		}
#endif
		this->renderTarget = NULL;
	}

	void OpenGLES_RenderSystem::_deviceSuspend()
	{
		OpenGL_RenderSystem::_deviceSuspend();
		this->_deviceUnloadTextures();
		this->_tryDestroyIntermediateRenderTextures();
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
		hstr extensions;
		GL_SAFE_CALL(const GLubyte* extensionsString = glGetString, (GL_EXTENSIONS));
		if (extensionsString != NULL)
		{
			extensions = (const char*)extensionsString;
		}
		hlog::write(logTag, "Extensions supported:\n- " + extensions.trimmedRight().replaced(" ", "\n- "));
#if defined(_IOS) || defined(_UWP) // iOS devices support limited NPOT textures as per device specification since iPhone 3GS
		this->caps.npotTexturesLimited = true;
#else
		this->caps.npotTexturesLimited = (extensions.contains("IMG_texture_npot") || extensions.contains("APPLE_texture_2D_limited_npot"));
#endif
		this->caps.npotTextures = (extensions.contains("OES_texture_npot") || extensions.contains("ARB_texture_non_power_of_two"));
		// TODO - is there a way to make this work on Win32?
#if !defined(_WIN32) || defined(_UWP)
		this->blendSeparationSupported = (extensions.contains("OES_blend_equation_separate") && extensions.contains("OES_blend_func_separate"));
		hlog::write(logTag, "Blend-separate supported: " + hstr(this->blendSeparationSupported ? "yes" : "no"));
#endif
#ifdef __ANDROID__
		this->etc1Supported = extensions.contains("OES_compressed_ETC1_RGB8_texture");
		hlog::write(logTag, "ETC1 supported: " + hstr(this->etc1Supported ? "yes" : "no"));
#endif
        // OpenGLES implementations do not appear to support alpha textures by default
		this->caps.textureFormats /= Image::Format::Alpha;
		this->caps.textureFormats /= Image::Format::Greyscale;
		return OpenGL_RenderSystem::_deviceSetupCaps();
	}

	void OpenGLES_RenderSystem::_deviceSetup()
	{
		GL_SAFE_CALL(glEnableVertexAttribArray, (VERTEX_ARRAY));
		OpenGL_RenderSystem::_deviceSetup();
		this->_createShaders();
		this->deviceState->texture = NULL;
		this->deviceState_matrixChanged = true;
		this->deviceState_systemColorChanged = true;
		this->deviceState_colorModeFactorChanged = true;
		this->deviceState_shader = NULL;
#ifdef _IOS // this is only required for iOS
		GL_SAFE_CALL(glGetIntegerv, (GL_FRAMEBUFFER_BINDING, (GLint*)&this->framebufferId));
		GL_SAFE_CALL(glGetIntegerv, (GL_RENDERBUFFER_BINDING, (GLint*)&this->renderbufferId));
#endif
		this->_updateIntermediateRenderTextures();
		if (this->_currentIntermediateRenderTexture != NULL)
		{
			GL_SAFE_CALL(glBindFramebuffer, (GL_FRAMEBUFFER, ((OpenGLES_Texture*)this->_currentIntermediateRenderTexture)->framebufferId));
#ifdef _IOS // this is only required for iOS
			GL_SAFE_CALL(glBindRenderbuffer, (GL_RENDERBUFFER, 0));
#endif
		}
	}

	void OpenGLES_RenderSystem::_createShaders()
	{
		hstream data;
		LOAD_VERTEX_SHADER(this->vertexShaderPlain, Plain, data);
		LOAD_VERTEX_SHADER(this->vertexShaderTextured, Textured, data);
		LOAD_VERTEX_SHADER(this->vertexShaderColored, Colored, data);
		LOAD_VERTEX_SHADER(this->vertexShaderColoredTextured, ColoredTextured, data);
		LOAD_PIXEL_SHADER(this->pixelShaderMultiply, Multiply, data);
		LOAD_PIXEL_SHADER(this->pixelShaderAlphaMap, AlphaMap, data);
		LOAD_PIXEL_SHADER(this->pixelShaderLerp, Lerp, data);
		LOAD_PIXEL_SHADER(this->pixelShaderDesaturate, Desaturate, data);
		LOAD_PIXEL_SHADER(this->pixelShaderSepia, Sepia, data);
		LOAD_PIXEL_SHADER(this->pixelShaderColoredMultiply, ColoredMultiply, data);
		LOAD_PIXEL_SHADER(this->pixelShaderColoredAlphaMap, ColoredAlphaMap, data);
		LOAD_PIXEL_SHADER(this->pixelShaderColoredLerp, ColoredLerp, data);
		LOAD_PIXEL_SHADER(this->pixelShaderColoredDesaturate, ColoredDesaturate, data);
		LOAD_PIXEL_SHADER(this->pixelShaderColoredSepia, ColoredSepia, data);
		LOAD_PIXEL_SHADER(this->pixelShaderTexturedMultiply, TexturedMultiply, data);
		LOAD_PIXEL_SHADER(this->pixelShaderTexturedAlphaMap, TexturedAlphaMap, data);
		LOAD_PIXEL_SHADER(this->pixelShaderTexturedLerp, TexturedLerp, data);
		LOAD_PIXEL_SHADER(this->pixelShaderTexturedDesaturate, TexturedDesaturate, data);
		LOAD_PIXEL_SHADER(this->pixelShaderTexturedSepia, TexturedSepia, data);
		LOAD_PIXEL_SHADER(this->pixelShaderColoredTexturedMultiply, ColoredTexturedMultiply, data);
		LOAD_PIXEL_SHADER(this->pixelShaderColoredTexturedAlphaMap, ColoredTexturedAlphaMap, data);
		LOAD_PIXEL_SHADER(this->pixelShaderColoredTexturedLerp, ColoredTexturedLerp, data);
		LOAD_PIXEL_SHADER(this->pixelShaderColoredTexturedDesaturate, ColoredTexturedDesaturate, data);
		LOAD_PIXEL_SHADER(this->pixelShaderColoredTexturedSepia, ColoredTexturedSepia, data);
#ifdef __ANDROID__
		LOAD_PIXEL_SHADER(this->pixelShaderTexturedMultiply_AlphaHack, TexturedMultiply_AlphaHack, data);
		LOAD_PIXEL_SHADER(this->pixelShaderTexturedLerp_AlphaHack, TexturedLerp_AlphaHack, data);
		LOAD_PIXEL_SHADER(this->pixelShaderTexturedDesaturate_AlphaHack, TexturedDesaturate_AlphaHack, data);
		LOAD_PIXEL_SHADER(this->pixelShaderTexturedSepia_AlphaHack, TexturedSepia_AlphaHack, data);
		LOAD_PIXEL_SHADER(this->pixelShaderColoredTexturedMultiply_AlphaHack, ColoredTexturedMultiply_AlphaHack, data);
		LOAD_PIXEL_SHADER(this->pixelShaderColoredTexturedLerp_AlphaHack, ColoredTexturedLerp_AlphaHack, data);
		LOAD_PIXEL_SHADER(this->pixelShaderColoredTexturedDesaturate_AlphaHack, ColoredTexturedDesaturate_AlphaHack, data);
		LOAD_PIXEL_SHADER(this->pixelShaderColoredTexturedSepia_AlphaHack, ColoredTexturedSepia_AlphaHack, data);
#endif
		LOAD_PROGRAM(this->shaderMultiply, this->pixelShaderMultiply, this->vertexShaderPlain);
		LOAD_PROGRAM(this->shaderAlphaMap, this->pixelShaderAlphaMap, this->vertexShaderPlain);
		LOAD_PROGRAM(this->shaderLerp, this->pixelShaderLerp, this->vertexShaderPlain);
		LOAD_PROGRAM(this->shaderDesaturate, this->pixelShaderDesaturate, this->vertexShaderPlain);
		LOAD_PROGRAM(this->shaderSepia, this->pixelShaderSepia, this->vertexShaderPlain);
		LOAD_PROGRAM(this->shaderTexturedMultiply, this->pixelShaderTexturedMultiply, this->vertexShaderTextured);
		LOAD_PROGRAM(this->shaderTexturedAlphaMap, this->pixelShaderTexturedAlphaMap, this->vertexShaderTextured);
		LOAD_PROGRAM(this->shaderTexturedLerp, this->pixelShaderTexturedLerp, this->vertexShaderTextured);
		LOAD_PROGRAM(this->shaderTexturedDesaturate, this->pixelShaderTexturedDesaturate, this->vertexShaderTextured);
		LOAD_PROGRAM(this->shaderTexturedSepia, this->pixelShaderTexturedSepia, this->vertexShaderTextured);
		LOAD_PROGRAM(this->shaderColoredMultiply, this->pixelShaderColoredMultiply, this->vertexShaderColored);
		LOAD_PROGRAM(this->shaderColoredAlphaMap, this->pixelShaderColoredAlphaMap, this->vertexShaderColored);
		LOAD_PROGRAM(this->shaderColoredLerp, this->pixelShaderColoredLerp, this->vertexShaderColored);
		LOAD_PROGRAM(this->shaderColoredDesaturate, this->pixelShaderColoredDesaturate, this->vertexShaderColored);
		LOAD_PROGRAM(this->shaderColoredSepia, this->pixelShaderColoredSepia, this->vertexShaderColored);
		LOAD_PROGRAM(this->shaderColoredTexturedMultiply, this->pixelShaderColoredTexturedMultiply, this->vertexShaderColoredTextured);
		LOAD_PROGRAM(this->shaderColoredTexturedAlphaMap, this->pixelShaderColoredTexturedAlphaMap, this->vertexShaderColoredTextured);
		LOAD_PROGRAM(this->shaderColoredTexturedLerp, this->pixelShaderColoredTexturedLerp, this->vertexShaderColoredTextured);
		LOAD_PROGRAM(this->shaderColoredTexturedDesaturate, this->pixelShaderColoredTexturedDesaturate, this->vertexShaderColoredTextured);
		LOAD_PROGRAM(this->shaderColoredTexturedSepia, this->pixelShaderColoredTexturedSepia, this->vertexShaderColoredTextured);
#ifdef __ANDROID__
		LOAD_PROGRAM(this->shaderTexturedMultiply_AlphaHack, this->pixelShaderTexturedMultiply_AlphaHack, this->vertexShaderTextured);
		LOAD_PROGRAM(this->shaderTexturedLerp_AlphaHack, this->pixelShaderTexturedLerp_AlphaHack, this->vertexShaderTextured);
		LOAD_PROGRAM(this->shaderTexturedDesaturate_AlphaHack, this->pixelShaderTexturedDesaturate_AlphaHack, this->vertexShaderTextured);
		LOAD_PROGRAM(this->shaderTexturedSepia_AlphaHack, this->pixelShaderTexturedSepia_AlphaHack, this->vertexShaderTextured);
		LOAD_PROGRAM(this->shaderColoredTexturedMultiply_AlphaHack, this->pixelShaderColoredTexturedMultiply_AlphaHack, this->vertexShaderColoredTextured);
		LOAD_PROGRAM(this->shaderColoredTexturedLerp_AlphaHack, this->pixelShaderColoredTexturedLerp_AlphaHack, this->vertexShaderColoredTextured);
		LOAD_PROGRAM(this->shaderColoredTexturedDesaturate_AlphaHack, this->pixelShaderColoredTexturedDesaturate_AlphaHack, this->vertexShaderColoredTextured);
		LOAD_PROGRAM(this->shaderColoredTexturedSepia_AlphaHack, this->pixelShaderColoredTexturedSepia_AlphaHack, this->vertexShaderColoredTextured);
#endif
	}

	void OpenGLES_RenderSystem::_destroyShaders()
	{
		DELETE_VERTEX_SHADER(this->vertexShaderPlain);
		DELETE_VERTEX_SHADER(this->vertexShaderTextured);
		DELETE_VERTEX_SHADER(this->vertexShaderColored);
		DELETE_VERTEX_SHADER(this->vertexShaderColoredTextured);
		DELETE_PIXEL_SHADER(this->pixelShaderMultiply);
		DELETE_PIXEL_SHADER(this->pixelShaderAlphaMap);
		DELETE_PIXEL_SHADER(this->pixelShaderLerp);
		DELETE_PIXEL_SHADER(this->pixelShaderDesaturate);
		DELETE_PIXEL_SHADER(this->pixelShaderSepia);
		DELETE_PIXEL_SHADER(this->pixelShaderColoredMultiply);
		DELETE_PIXEL_SHADER(this->pixelShaderColoredAlphaMap);
		DELETE_PIXEL_SHADER(this->pixelShaderColoredLerp);
		DELETE_PIXEL_SHADER(this->pixelShaderColoredDesaturate);
		DELETE_PIXEL_SHADER(this->pixelShaderColoredSepia);
		DELETE_PIXEL_SHADER(this->pixelShaderTexturedMultiply);
		DELETE_PIXEL_SHADER(this->pixelShaderTexturedAlphaMap);
		DELETE_PIXEL_SHADER(this->pixelShaderTexturedLerp);
		DELETE_PIXEL_SHADER(this->pixelShaderTexturedDesaturate);
		DELETE_PIXEL_SHADER(this->pixelShaderTexturedSepia);
		DELETE_PIXEL_SHADER(this->pixelShaderColoredTexturedMultiply);
		DELETE_PIXEL_SHADER(this->pixelShaderColoredTexturedAlphaMap);
		DELETE_PIXEL_SHADER(this->pixelShaderColoredTexturedLerp);
		DELETE_PIXEL_SHADER(this->pixelShaderColoredTexturedDesaturate);
		DELETE_PIXEL_SHADER(this->pixelShaderColoredTexturedSepia);
#ifdef __ANDROID__
		DELETE_PIXEL_SHADER(this->pixelShaderTexturedMultiply_AlphaHack);
		DELETE_PIXEL_SHADER(this->pixelShaderTexturedLerp_AlphaHack);
		DELETE_PIXEL_SHADER(this->pixelShaderTexturedDesaturate_AlphaHack);
		DELETE_PIXEL_SHADER(this->pixelShaderTexturedSepia_AlphaHack);
		DELETE_PIXEL_SHADER(this->pixelShaderColoredTexturedMultiply_AlphaHack);
		DELETE_PIXEL_SHADER(this->pixelShaderColoredTexturedLerp_AlphaHack);
		DELETE_PIXEL_SHADER(this->pixelShaderColoredTexturedDesaturate_AlphaHack);
		DELETE_PIXEL_SHADER(this->pixelShaderColoredTexturedSepia_AlphaHack);
#endif
		_HL_TRY_DELETE(this->shaderMultiply);
		_HL_TRY_DELETE(this->shaderAlphaMap);
		_HL_TRY_DELETE(this->shaderLerp);
		_HL_TRY_DELETE(this->shaderDesaturate);
		_HL_TRY_DELETE(this->shaderSepia);
		_HL_TRY_DELETE(this->shaderColoredMultiply);
		_HL_TRY_DELETE(this->shaderColoredAlphaMap);
		_HL_TRY_DELETE(this->shaderColoredLerp);
		_HL_TRY_DELETE(this->shaderColoredDesaturate);
		_HL_TRY_DELETE(this->shaderColoredSepia);
		_HL_TRY_DELETE(this->shaderTexturedMultiply);
		_HL_TRY_DELETE(this->shaderTexturedAlphaMap);
		_HL_TRY_DELETE(this->shaderTexturedLerp);
		_HL_TRY_DELETE(this->shaderTexturedDesaturate);
		_HL_TRY_DELETE(this->shaderTexturedSepia);
		_HL_TRY_DELETE(this->shaderColoredTexturedMultiply);
		_HL_TRY_DELETE(this->shaderColoredTexturedAlphaMap);
		_HL_TRY_DELETE(this->shaderColoredTexturedLerp);
		_HL_TRY_DELETE(this->shaderColoredTexturedDesaturate);
		_HL_TRY_DELETE(this->shaderColoredTexturedSepia);
#ifdef __ANDROID__
		_HL_TRY_DELETE(this->shaderTexturedMultiply_AlphaHack);
		_HL_TRY_DELETE(this->shaderTexturedLerp_AlphaHack);
		_HL_TRY_DELETE(this->shaderTexturedDesaturate_AlphaHack);
		_HL_TRY_DELETE(this->shaderTexturedSepia_AlphaHack);
		_HL_TRY_DELETE(this->shaderColoredTexturedMultiply_AlphaHack);
		_HL_TRY_DELETE(this->shaderColoredTexturedLerp_AlphaHack);
		_HL_TRY_DELETE(this->shaderColoredTexturedDesaturate_AlphaHack);
		_HL_TRY_DELETE(this->shaderColoredTexturedSepia_AlphaHack);
#endif
	}

	void OpenGLES_RenderSystem::_updateDeviceState(RenderState* state, bool forceUpdate)
	{
		OpenGL_RenderSystem::_updateDeviceState(state, forceUpdate);
		this->_updateShader(forceUpdate);
	}

	bool OpenGLES_RenderSystem::canUseLowLevelCalls() const
	{
		return (!april::application->isSuspended());
	}

	void OpenGLES_RenderSystem::_setDeviceModelviewMatrix(const gmat4& matrix)
	{
		this->deviceState_matrixChanged = true;
	}

	void OpenGLES_RenderSystem::_setDeviceProjectionMatrix(const gmat4& matrix)
	{
		this->deviceState_matrixChanged = true;
	}

	void OpenGLES_RenderSystem::_setDeviceBlendMode(const BlendMode& blendMode)
	{
#if !defined(_WIN32) || defined(_UWP)
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

	void OpenGLES_RenderSystem::_setDeviceTexture(Texture* texture)
	{
#ifdef __ANDROID__
		if (texture != NULL)
		{
			OpenGLES_Texture* currentTexture = (OpenGLES_Texture*)texture;
			if (currentTexture->alphaTextureId != 0)
			{
				GL_SAFE_CALL(glActiveTexture, (GL_TEXTURE1));
				GL_SAFE_CALL(glBindTexture, (GL_TEXTURE_2D, currentTexture->alphaTextureId));
				GL_SAFE_CALL(glActiveTexture, (GL_TEXTURE0));
			}
		}
#endif
		OpenGL_RenderSystem::_setDeviceTexture(texture);
	}

	void OpenGLES_RenderSystem::_setDeviceColorMode(const ColorMode& colorMode, float colorModeFactor, bool useTexture, bool useColor, const Color& systemColor)
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
		if (this->deviceState->colorMode == ColorMode::Multiply)
		{
			shader = _SELECT_SHADER(this->deviceState->useTexture, this->deviceState->useColor, Multiply);
		}
		else if (this->deviceState->colorMode == ColorMode::AlphaMap)
		{
			shader = _SELECT_SHADER(this->deviceState->useTexture, this->deviceState->useColor, AlphaMap);
		}
		else if (this->deviceState->colorMode == ColorMode::Lerp)
		{
			shader = _SELECT_SHADER(this->deviceState->useTexture, this->deviceState->useColor, Lerp);
		}
		else if (this->deviceState->colorMode == ColorMode::Desaturate)
		{
			shader = _SELECT_SHADER(this->deviceState->useTexture, this->deviceState->useColor, Desaturate);
		}
		else if (this->deviceState->colorMode == ColorMode::Sepia)
		{
			shader = _SELECT_SHADER(this->deviceState->useTexture, this->deviceState->useColor, Sepia);
		}
		else
		{
			hlog::warn(logTag, "Trying to set unsupported color mode!");
		}
#ifdef __ANDROID__
		OpenGLES_Texture* currentTexture = (OpenGLES_Texture*)this->deviceState->texture;
		bool useAlphaHack = (this->deviceState->useTexture && currentTexture != NULL && currentTexture->alphaTextureId != 0);
		if (useAlphaHack)
		{
			if (shader == this->shaderTexturedMultiply)
			{
				shader = this->shaderTexturedMultiply_AlphaHack;
			}
			else if (shader == this->shaderTexturedLerp)
			{
				shader = this->shaderTexturedLerp_AlphaHack;
			}
			else if (shader == this->shaderTexturedDesaturate)
			{
				shader = this->shaderTexturedDesaturate_AlphaHack;
			}
			else if (shader == this->shaderTexturedSepia)
			{
				shader = this->shaderTexturedSepia_AlphaHack;
			}
			else if (shader == this->shaderColoredTexturedMultiply)
			{
				shader = this->shaderColoredTexturedMultiply_AlphaHack;
			}
			else if (shader == this->shaderColoredTexturedLerp)
			{
				shader = this->shaderColoredTexturedLerp_AlphaHack;
			}
			else if (shader == this->shaderColoredTexturedDesaturate)
			{
				shader = this->shaderColoredTexturedDesaturate_AlphaHack;
			}
			else if (shader == this->shaderColoredTexturedSepia)
			{
				shader = this->shaderColoredTexturedSepia_AlphaHack;
			}
		}
#endif
		if (this->deviceState_shader != shader)
		{
			forceUpdate = true;
		}
		if (forceUpdate)
		{
			this->deviceState_shader = shader;
			if (this->deviceState_shader != NULL)
			{
				GL_SAFE_CALL(glUseProgram, (this->deviceState_shader->glShaderProgram));
			}
			if (this->deviceState->useTexture)
			{
				GL_SAFE_CALL(glActiveTexture, (GL_TEXTURE0));
			}
			if (this->deviceState_shader != NULL)
			{
				GL_SAFE_CALL(int samplerLocation = glGetUniformLocation, (this->deviceState_shader->glShaderProgram, "sampler2d"));
				if (samplerLocation >= 0)
				{
					GL_SAFE_CALL(glUniform1i, (samplerLocation, 0));
				}
#ifdef __ANDROID__
				if (useAlphaHack) // will only be true if this->deviceState->useTexture is true
				{
					GL_SAFE_CALL(samplerLocation = glGetUniformLocation, (this->deviceState_shader->glShaderProgram, "sampler2dAlpha"));
					if (samplerLocation >= 0)
					{
						GL_SAFE_CALL(glUniform1i, (samplerLocation, 1));
					}
				}
#endif
			}
		}
		if (this->deviceState_shader != NULL)
		{
			if (forceUpdate || this->deviceState_matrixChanged)
			{
				GL_SAFE_CALL(int matrixLocation = glGetUniformLocation, (this->deviceState_shader->glShaderProgram, "transformationMatrix"));
				GL_SAFE_CALL(glUniformMatrix4fv, (matrixLocation, 1, GL_FALSE, (this->deviceState->projectionMatrix * this->deviceState->modelviewMatrix).data));
				this->deviceState_matrixChanged = false;
			}
			if (forceUpdate || this->deviceState_systemColorChanged)
			{
				GL_SAFE_CALL(int systemColorLocation = glGetUniformLocation, (this->deviceState_shader->glShaderProgram, "systemColor"));
				if (systemColorLocation >= 0)
				{
					static float shaderSystemColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
					shaderSystemColor[0] = this->deviceState->systemColor.r_f();
					shaderSystemColor[1] = this->deviceState->systemColor.g_f();
					shaderSystemColor[2] = this->deviceState->systemColor.b_f();
					shaderSystemColor[3] = this->deviceState->systemColor.a_f();
					GL_SAFE_CALL(glUniform4fv, (systemColorLocation, 1, shaderSystemColor));
				}
				this->deviceState_systemColorChanged = false;
			}
			if (forceUpdate || this->deviceState_colorModeFactorChanged)
			{
				GL_SAFE_CALL(int lerpLocation = glGetUniformLocation, (this->deviceState_shader->glShaderProgram, "lerpAlpha"));
				if (lerpLocation >= 0)
				{
					static float shaderColorModeFactor = 1.0f;
					shaderColorModeFactor = this->deviceState->colorModeFactor;
					GL_SAFE_CALL(glUniform1fv, (lerpLocation, 1, &shaderColorModeFactor));
				}
				this->deviceState_colorModeFactorChanged = false;
			}
		}
	}

	void OpenGLES_RenderSystem::_devicePresentFrame(bool systemEnabled)
	{
		if (this->_currentIntermediateRenderTexture != NULL)
		{
			GL_SAFE_CALL(glBindFramebuffer, (GL_FRAMEBUFFER, this->framebufferId));
#ifdef _IOS // this is only required for iOS
			GL_SAFE_CALL(glBindRenderbuffer, (GL_RENDERBUFFER, this->renderbufferId));
#endif
			this->_presentIntermediateRenderTexture();
		}
		OpenGL_RenderSystem::_devicePresentFrame(systemEnabled);
		this->_updateIntermediateRenderTextures();
		if (this->_currentIntermediateRenderTexture != NULL)
		{
			GL_SAFE_CALL(glBindFramebuffer, (GL_FRAMEBUFFER, ((OpenGLES_Texture*)this->_currentIntermediateRenderTexture)->framebufferId));
#ifdef _IOS // this is only required for iOS
			GL_SAFE_CALL(glBindRenderbuffer, (GL_RENDERBUFFER, 0));
#endif
		}
	}

	void OpenGLES_RenderSystem::_deviceCopyRenderTargetData(Texture* source, Texture* destination)
	{
		if (source->getType() != Texture::Type::RenderTarget)
		{
			hlog::error(logTag, "Cannot copy render target data, source texture is not a render target!");
			return;
		}
		if (destination->getType() != Texture::Type::RenderTarget)
		{
			hlog::error(logTag, "Cannot copy render target data, destination texture is not a render target!");
			return;
		}
		unsigned int previousFramebufferId = 0;
		GL_SAFE_CALL(glGetIntegerv, (GL_FRAMEBUFFER_BINDING, (GLint*)&previousFramebufferId));
		GL_SAFE_CALL(glBindFramebuffer, (GL_FRAMEBUFFER, ((OpenGLES_Texture*)destination)->framebufferId));
		this->_intermediateState->viewport.setSize(source->getWidth(), source->getHeight());
		this->_intermediateState->projectionMatrix.setOrthoProjection(
			grectf(1.0f - 2.0f * this->pixelOffset / source->getWidth(), 1.0f - 2.0f * this->pixelOffset / source->getHeight(), 2.0f, 2.0f));
		this->_intermediateState->texture = source;
		RenderState deviceState(*this->deviceState);
		this->_updateDeviceState(this->_intermediateState, true);
		this->_deviceRender(RenderOperation::TriangleList, this->_intermediateRenderVertices, APRIL_INTERMEDIATE_TEXTURE_VERTICES_COUNT);
		GL_SAFE_CALL(glBindFramebuffer, (GL_FRAMEBUFFER, previousFramebufferId));
		this->_updateDeviceState(&deviceState, true);
	}

	void OpenGLES_RenderSystem::_setGlTextureEnabled(bool enabled)
	{
		if (enabled)
		{
			GL_SAFE_CALL(glEnableVertexAttribArray, (TEXTURE_ARRAY));
		}
		else
		{
			GL_SAFE_CALL(glDisableVertexAttribArray, (TEXTURE_ARRAY));
		}
	}

	void OpenGLES_RenderSystem::_setGlColorEnabled(bool enabled)
	{
		if (enabled)
		{
			GL_SAFE_CALL(glEnableVertexAttribArray, (COLOR_ARRAY));
		}
		else
		{
			GL_SAFE_CALL(glDisableVertexAttribArray, (COLOR_ARRAY));
		}
	}

	void OpenGLES_RenderSystem::_setGlVertexPointer(int stride, const void* pointer)
	{
		GL_SAFE_CALL(glVertexAttribPointer, (VERTEX_ARRAY, 3, GL_FLOAT, GL_FALSE, stride, pointer));
	}

	void OpenGLES_RenderSystem::_setGlTexturePointer(int stride, const void* pointer)
	{
		GL_SAFE_CALL(glVertexAttribPointer, (TEXTURE_ARRAY, 2, GL_FLOAT, GL_TRUE, stride, pointer));
	}

	void OpenGLES_RenderSystem::_setGlColorPointer(int stride, const void* pointer)
	{
		GL_SAFE_CALL(glVertexAttribPointer, (COLOR_ARRAY, 4, GL_UNSIGNED_BYTE, GL_TRUE, stride, pointer));
	}

	// TODOa - these need to be refactored, they can't be called directly like this
	Texture* OpenGLES_RenderSystem::_getRenderTarget()
	{
		return this->renderTarget;
	}

	void OpenGLES_RenderSystem::_deviceSetRenderTarget(Texture* source)
	{
		OpenGLES_Texture* texture = (OpenGLES_Texture*)source;
		if (texture == NULL)
		{
			if (this->_currentIntermediateRenderTexture != NULL)
			{
				GL_SAFE_CALL(glBindFramebuffer, (GL_FRAMEBUFFER, ((OpenGLES_Texture*)this->_currentIntermediateRenderTexture)->framebufferId));
			}
			else
			{
				GL_SAFE_CALL(glBindFramebuffer, (GL_FRAMEBUFFER, this->framebufferId));
			}
		}
		else
		{
			GL_SAFE_CALL(glBindFramebuffer, (GL_FRAMEBUFFER, texture->framebufferId));
		}
		this->renderTarget = texture;
	}

}
#endif
