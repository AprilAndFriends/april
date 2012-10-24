/// @file
/// @author  Kresimir Spes
/// @author  Ivan Vucica
/// @author  Boris Mikic
/// @version 2.43
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _OPENGL
// TODO - should be cleaned up a bit
#if __APPLE__
#include <TargetConditionals.h>
#endif
#if TARGET_OS_IPHONE
#ifdef _OPENGLES1
	#include <OpenGLES/ES1/gl.h>
	#include <OpenGLES/ES1/glext.h>
#elif defined(_OPENGLES2)
	#include <OpenGLES/ES2/gl.h>
	#include <OpenGLES/ES2/glext.h>
	extern GLint _positionSlot;
#endif
#elif defined(_OPENGLES)
#include <GLES/gl.h>
#ifdef _ANDROID
#define GL_GLEXT_PROTOTYPES
#include <GLES/glext.h>
#else
#include <EGL/egl.h>
#endif
#else
#ifdef _WIN32
#include <windows.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifndef __APPLE__
#include <gl/GL.h>
#define GL_GLEXT_PROTOTYPES
#include <gl/glext.h>
#else
#include <OpenGL/gl.h>
#endif
#endif

#include <gtypes/Vector2.h>
#include <hltypes/exception.h>
#include <hltypes/hlog.h>
#include <hltypes/hltypesUtil.h>

#include "april.h"
#include "ImageSource.h"
#include "Keys.h"
#include "OpenGL_RenderSystem.h"
#include "OpenGL_Texture.h"
#include "Platform.h"
#include "Timer.h"
#include "Window.h"

namespace april
{
	static Color lastColor = Color::Black;
#ifdef _WIN32 // if _WIN32
	static HWND hWnd = 0;
	HDC hDC = 0;
#ifndef _OPENGLES // if _WIN32 && GLES
	static HGLRC hRC = 0;
#else
	static EGLDisplay eglDisplay = 0;
	static EGLConfig eglConfig	= 0;
	static EGLSurface eglSurface = 0;
	static EGLContext eglContext = 0;
	static EGLint pi32ConfigAttribs[128] = {EGL_RED_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_BLUE_SIZE, 8, EGL_ALPHA_SIZE, 0, EGL_SURFACE_TYPE, EGL_WINDOW_BIT, EGL_NONE};
#endif
#endif

	// TODO - refactor
	int OpenGL_RenderSystem::_getMaxTextureSize()
	{
#ifdef _WIN32
#ifndef _OPENGLES
		if (hRC == 0)
#else
		if (eglDisplay == 0)
#endif
		{
			return 0;
		}
#endif
		int max;
		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max);
		return max;
	}

	void win_mat_invert()
	{
		((OpenGL_RenderSystem*)april::rendersys)->setMatrixMode(GL_PROJECTION);
		glPushMatrix();
		float mat[16];
		glGetFloatv(GL_PROJECTION_MATRIX, mat);
		hswap(mat[1], mat[0]);
		hswap(mat[4], mat[5]);
		mat[5] = -mat[5];
		mat[13] = -mat[13];
		glLoadMatrixf(mat);
	}
	
	// translation from abstract render ops to gl's render ops
	int gl_render_ops[]=
	{
		0,
		GL_TRIANGLES,		// ROP_TRIANGLE_LIST
		GL_TRIANGLE_STRIP,	// ROP_TRIANGLE_STRIP
		GL_TRIANGLE_FAN,	// ROP_TRIANGLE_FAN
		GL_LINES,			// ROP_LINE_LIST
		GL_LINE_STRIP,		// ROP_LINE_STRIP
		GL_POINTS,			// ROP_POINTS
	};
	
	OpenGL_RenderState::OpenGL_RenderState()
	{
		this->reset();
	}
	
	OpenGL_RenderState::~OpenGL_RenderState()
	{
	}
	
	void OpenGL_RenderState::reset()
	{
		this->textureCoordinatesEnabled = false;
		this->colorEnabled = false;
		this->textureId = 0;
		this->textureFilter = Texture::FILTER_UNDEFINED;
		this->textureAddressMode = Texture::ADDRESS_UNDEFINED;
		this->systemColor = Color::Black;
		this->modelviewMatrixChanged = false;
		this->projectionMatrixChanged = false;
		this->blendMode = (BlendMode)10000;
		this->colorMode = (ColorMode)10000;
		this->colorModeAlpha = 255;
		this->modeMatrix = 0;
		this->strideVertex = 0;
		this->pointerVertex = NULL;
		this->strideTexCoord = 0;
		this->pointerTexCoord = NULL;
		this->strideColor = 0;
		this->pointerColor = NULL;
	}
	
	OpenGL_RenderSystem::OpenGL_RenderSystem() : RenderSystem(), activeTexture(NULL)
	{
		this->name = APRIL_RS_OPENGL;
	}

	OpenGL_RenderSystem::~OpenGL_RenderSystem()
	{
		this->destroy();
	}

	bool OpenGL_RenderSystem::create(chstr options)
	{
		if (!RenderSystem::create(options))
		{
			return false;
		}
		this->activeTexture = NULL;
		this->options = options;
		return true;
	}

	bool OpenGL_RenderSystem::destroy()
	{
		if (!RenderSystem::destroy())
		{
			return false;
		}
		// TODO - should maybe be refactored
#ifdef _WIN32
		this->_releaseWindow();
#endif
		return true;
	}

#ifdef _WIN32
	void OpenGL_RenderSystem::_releaseWindow()
	{
#ifndef _OPENGLES
		if (hRC != 0)
		{
			wglMakeCurrent(NULL, NULL);
			wglDeleteContext(hRC);
			hRC = 0;
		}
#else
		if (eglDisplay != 0)
		{
			eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
			eglTerminate(eglDisplay);
			eglDisplay = 0;
		}
#endif
		if (hDC != 0)
		{
			ReleaseDC(hWnd, hDC);
			hDC = 0;
		}
	}
#endif

	void OpenGL_RenderSystem::assignWindow(Window* window)
	{
#ifdef _WIN32
		hWnd = (HWND)april::window->getBackendId();
		PIXELFORMATDESCRIPTOR pfd;
		memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
		pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
		pfd.nVersion = 1;
		pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL |	PFD_DOUBLEBUFFER;
		pfd.iPixelType = PFD_TYPE_RGBA;
		pfd.cColorBits = 24;
		pfd.cStencilBits = 16;
		pfd.dwLayerMask = PFD_MAIN_PLANE;

		hDC = GetDC(hWnd);
		if (hDC == 0)
		{
			hlog::error(april::logTag, "Can't create a GL device context!");
			return;
		}
		GLuint pixelFormat = ChoosePixelFormat(hDC, &pfd);
		if (pixelFormat == 0)
		{
			hlog::error(april::logTag, "Can't find a suitable pixel format!");
			this->_releaseWindow();
			return;
		}
		if (SetPixelFormat(hDC, pixelFormat, &pfd) == 0)
		{
			hlog::error(april::logTag, "Can't set the pixel format!");
			this->_releaseWindow();
			return;
		}
#ifndef _OPENGLES
		hRC = wglCreateContext(hDC);
		if (hRC == 0)
		{
			hlog::error(april::logTag, "Can't create a GL rendering context!");
			this->_releaseWindow();
			return;
		}
		if (wglMakeCurrent(hDC, hRC) == 0)
		{
			hlog::error(april::logTag, "Can't activate the GL rendering context!");
			this->_releaseWindow();
			return;
		}
#else
		eglDisplay = eglGetDisplay((NativeDisplayType)hDC);
		if (eglDisplay == EGL_NO_DISPLAY)
		{
			 eglDisplay = eglGetDisplay((NativeDisplayType)EGL_DEFAULT_DISPLAY);
		}
		if (eglDisplay == EGL_NO_DISPLAY)
		{
			hlog::error(april::logTag, "Can't get EGL display!");
			this->_releaseWindow();
			return;
		}
		EGLint majorVersion;
		EGLint minorVersion;
		if (!eglInitialize(eglDisplay, &majorVersion, &minorVersion))
		{
			hlog::error(april::logTag, "Can't initialize EGL!");
			this->_releaseWindow();
			return;
		}
		EGLint configs;
		EGLBoolean result = eglChooseConfig(eglDisplay, pi32ConfigAttribs, &eglConfig, 1, &configs);
		if (!result || configs == 0)
		{
			hlog::error(april::logTag, "Can't choose EGL config!");
			this->_releaseWindow();
			return;
		}

		eglSurface = eglCreateWindowSurface(eglDisplay, eglConfig, (NativeWindowType)hWnd, NULL);
		if (eglGetError() != EGL_SUCCESS)
		{
			hlog::error(april::logTag, "Can't create window surface!");
			this->_releaseWindow();
			return;
		}
		eglContext = eglCreateContext(eglDisplay, eglConfig, EGL_NO_CONTEXT, NULL);
		if (eglGetError() != EGL_SUCCESS)
		{
			hlog::error(april::logTag, "Can't create EGL context!");
			this->_releaseWindow();
			return;
		}
		eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext);
		if (eglGetError() != EGL_SUCCESS)
		{
			hlog::error(april::logTag, "Can't make context current!");
			this->_releaseWindow();
			return;
		}
#endif
#endif
		this->_setupDefaultParameters();
		this->setMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		this->setMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		this->orthoProjection.setSize((float)april::window->getWidth(), (float)april::window->getHeight());
	}

	void OpenGL_RenderSystem::reset()
	{
		RenderSystem::reset();
		this->state.reset();
		this->deviceState.reset();
		this->_setupDefaultParameters();
		this->state.modelviewMatrixChanged = true;
		this->state.projectionMatrixChanged = true;
		this->_applyStateChanges();
	}

	void OpenGL_RenderSystem::_setupDefaultParameters()
	{
		glViewport(0, 0, april::window->getWidth(), april::window->getHeight());
		glClearColor(0, 0, 0, 1);
		lastColor.set(0, 0, 0, 255);
		// GL defaults
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnable(GL_TEXTURE_2D);
		// pixel data
#ifndef _OPENGLES
		glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
		glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
		glPixelStorei(GL_UNPACK_SWAP_BYTES, GL_FALSE);
#endif
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		// other
		if (this->options.contains("zbuffer"))
		{
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_LEQUAL);
		}
		this->_setClientState(GL_TEXTURE_COORD_ARRAY, this->deviceState.textureCoordinatesEnabled);
		this->_setClientState(GL_COLOR_ARRAY, this->deviceState.colorEnabled);
		glColor4f(this->deviceState.systemColor.r_f(), this->deviceState.systemColor.g_f(), this->deviceState.systemColor.b_f(), this->deviceState.systemColor.a_f());
		glBindTexture(GL_TEXTURE_2D, this->deviceState.textureId);
		this->state.textureFilter = april::Texture::FILTER_LINEAR;
		this->state.textureAddressMode = april::Texture::ADDRESS_WRAP;
		this->state.blendMode = april::DEFAULT;
		this->state.colorMode = april::NORMAL;
	}

	harray<DisplayMode> OpenGL_RenderSystem::getSupportedDisplayModes()
	{
		// TODO
		harray<DisplayMode> result;
		gvec2 resolution = april::getSystemInfo().displayResolution;
		DisplayMode displayMode;
		displayMode.width = (int)resolution.x;
		displayMode.height = (int)resolution.y;
		displayMode.refreshRate = 60;
		result += displayMode;
		return result;
	}
	
	grect OpenGL_RenderSystem::getViewport()
	{
		static float params[4];
		glGetFloatv(GL_VIEWPORT, params);
		return grect(params[0], april::window->getHeight() - params[3] - params[1], params[2], params[3]);
	}
	
	void OpenGL_RenderSystem::setViewport(grect rect)
	{
		// because GL has to defy screen logic and has (0,0) in the bottom left corner
		glViewport((int)rect.x, (int)(april::window->getHeight() - rect.h - rect.y), (int)rect.w, (int)rect.h);
	}
	
	void OpenGL_RenderSystem::setTextureBlendMode(BlendMode mode)
	{
		this->state.blendMode = mode;
	}
	
	void OpenGL_RenderSystem::_setTextureBlendMode(BlendMode textureBlendMode)
	{
		// TODO - is there a way to make this work on Win32?
#ifndef _WIN32
		static int blendSeparationSupported = -1;
		if (blendSeparationSupported == -1)
		{
			// determine if blend separation is possible on first call to this function
			hstr extensions = (const char*)glGetString(GL_EXTENSIONS);
#ifdef _OPENGLES
			blendSeparationSupported = extensions.contains("OES_blend_equation_separate") && extensions.contains("OES_blend_func_separate");
#else
			blendSeparationSupported = extensions.contains("GL_EXT_blend_equation_separate") && extensions.contains("GL_EXT_blend_func_separate");
#endif
		}
		if (blendSeparationSupported)
		{
			// blending for the new generations
			switch (textureBlendMode)
			{
			case DEFAULT:
			case ALPHA_BLEND:
#ifdef _OPENGLES1
				glBlendEquationSeparateOES(GL_FUNC_ADD_OES, GL_FUNC_ADD_OES);
				glBlendFuncSeparateOES(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
#else
				glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
				glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
#endif
				break;
			case ADD:
#ifdef _OPENGLES1
				glBlendEquationSeparateOES(GL_FUNC_ADD_OES, GL_FUNC_ADD_OES);
				glBlendFuncSeparateOES(GL_SRC_ALPHA, GL_ONE, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
#else
				glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
				glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
#endif
				break;
			case SUBTRACT:
#ifdef _OPENGLES1
				glBlendEquationSeparateOES(GL_FUNC_REVERSE_SUBTRACT_OES, GL_FUNC_ADD_OES);
				glBlendFuncSeparateOES(GL_SRC_ALPHA, GL_ONE, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
					
#else
				glBlendEquationSeparate(GL_FUNC_REVERSE_SUBTRACT, GL_FUNC_ADD);
				glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
#endif
				break;
			case OVERWRITE:
#ifdef _OPENGLES1
				glBlendEquationSeparateOES(GL_FUNC_ADD_OES, GL_FUNC_ADD_OES);
				glBlendFuncSeparateOES(GL_ONE, GL_ZERO, GL_ONE, GL_ZERO);				
#else
				glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
				glBlendFuncSeparate(GL_ONE, GL_ZERO, GL_ONE, GL_ZERO);
#endif
				break;
			default:
				hlog::warn(april::logTag, "Trying to set unsupported blend mode!");
				break;
			}
		}
		else
#endif
		{
			// old-school blending mode for your mom
			if (textureBlendMode == ALPHA_BLEND || textureBlendMode == DEFAULT)
			{
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			}
			else if (textureBlendMode == ADD)
			{
				glBlendFunc(GL_SRC_ALPHA, GL_ONE);
			}
			else
			{
				hlog::warn(april::logTag, "Trying to set unsupported blend mode!");
			}
		}
	}
	
	void OpenGL_RenderSystem::setTextureColorMode(ColorMode textureColorMode, unsigned char alpha)
	{
		this->state.colorMode = textureColorMode;
		this->state.colorModeAlpha = alpha;
	}

	void OpenGL_RenderSystem::_setTextureColorMode(ColorMode textureColorMode, unsigned char alpha)
	{
		static float constColor[4];
		for_iter (i, 0, 4)
		{
			constColor[i] = alpha / 255.0f;
		}
		switch (textureColorMode)
		{
		case NORMAL:
		case MULTIPLY:
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_TEXTURE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_ALPHA, GL_PRIMARY_COLOR);
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_PRIMARY_COLOR);
			break;
		case LERP:
			glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, constColor);
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_TEXTURE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_ALPHA, GL_CONSTANT);
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_PRIMARY_COLOR);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_TEXTURE);
			break;
		case ALPHA_MAP:
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_TEXTURE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_ALPHA, GL_PRIMARY_COLOR);
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_REPLACE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_PRIMARY_COLOR);
			glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_PRIMARY_COLOR);
			break;
		default:
			hlog::warn(april::logTag, "Trying to set unsupported color mode!");
			break;
		}
	}

	void OpenGL_RenderSystem::setTextureFilter(Texture::Filter textureFilter)
	{
		this->state.textureFilter = textureFilter;
	}

	void OpenGL_RenderSystem::_setTextureFilter(Texture::Filter textureFilter)
	{
		switch (textureFilter)
		{
		case Texture::FILTER_LINEAR:
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			break;
		case Texture::FILTER_NEAREST:
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			break;
		default:
			hlog::warn(april::logTag, "Trying to set unsupported texture filter!");
			break;
		}
		this->textureFilter = textureFilter;
	}

	void OpenGL_RenderSystem::setTextureAddressMode(Texture::AddressMode textureAddressMode)
	{
		this->state.textureAddressMode = textureAddressMode;
	}

	void OpenGL_RenderSystem::_setTextureAddressMode(Texture::AddressMode textureAddressMode)
	{
		switch (textureAddressMode)
		{
		case Texture::ADDRESS_WRAP:
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			break;
		case Texture::ADDRESS_CLAMP:
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			break;
		default:
			hlog::warn(april::logTag, "Trying to set unsupported texture address mode!");
			break;
		}
		this->textureAddressMode = textureAddressMode;
	}

	Texture* OpenGL_RenderSystem::getRenderTarget()
	{
		return NULL;
	}
	
	void OpenGL_RenderSystem::setRenderTarget(Texture* texture)
	{
		// TODO
	}
	
	void OpenGL_RenderSystem::setTexture(Texture* texture)
	{
		this->activeTexture = (OpenGL_Texture*)texture;
		if (this->activeTexture == NULL)
		{
			this->bindTexture(0);
		}
		else
		{
			this->setTextureFilter(this->activeTexture->getFilter());
			this->setTextureAddressMode(this->activeTexture->getAddressMode());
			// filtering and wrapping applied before loading texture data, iOS OpenGL guidelines suggest it as an optimization
			this->activeTexture->load();
			this->bindTexture(this->activeTexture->textureId);
		}
	}

	void OpenGL_RenderSystem::setPixelShader(PixelShader* pixelShader)
	{
		hlog::warn(april::logTag, "Pixel shaders are not implemented!");
	}

	void OpenGL_RenderSystem::setVertexShader(VertexShader* vertexShader)
	{
		hlog::warn(april::logTag, "Vertex shaders are not implemented!");
	}

	void OpenGL_RenderSystem::setResolution(int w, int h)
	{
		hlog::warn(april::logTag, "setResolution() is not implemented!");
	}

	void OpenGL_RenderSystem::_applyStateChanges()
	{
		if (this->state.textureCoordinatesEnabled != this->deviceState.textureCoordinatesEnabled)
		{
			this->_setClientState(GL_TEXTURE_COORD_ARRAY, this->state.textureCoordinatesEnabled);
			this->deviceState.textureCoordinatesEnabled = this->state.textureCoordinatesEnabled;
		}
		if (this->state.colorEnabled != this->deviceState.colorEnabled)
		{
			this->_setClientState(GL_COLOR_ARRAY, this->state.colorEnabled);
			this->deviceState.colorEnabled = this->state.colorEnabled;
		}
		if (this->state.systemColor != this->deviceState.systemColor)
		{
			glColor4f(this->state.systemColor.r_f(), this->state.systemColor.g_f(), this->state.systemColor.b_f(), this->state.systemColor.a_f());
			this->deviceState.systemColor = this->state.systemColor;
		}
		if (this->state.textureId != this->deviceState.textureId)
		{
			glBindTexture(GL_TEXTURE_2D, this->state.textureId);
			this->deviceState.textureId = this->state.textureId;
			// TODO - you should memorize address and filter modes per texture in opengl to avoid unnecesarry calls
			this->deviceState.textureAddressMode = Texture::ADDRESS_UNDEFINED;
			this->deviceState.textureFilter = Texture::FILTER_UNDEFINED;
		}
		// texture has to be bound first or else filter and address mode won't be applied afterwards
		if (this->state.textureFilter != this->deviceState.textureFilter || this->deviceState.textureFilter == Texture::FILTER_UNDEFINED)
		{
			this->_setTextureFilter(this->state.textureFilter);
			this->deviceState.textureFilter = this->state.textureFilter;
		}
		if (this->state.textureAddressMode != this->deviceState.textureAddressMode || this->deviceState.textureAddressMode == Texture::ADDRESS_UNDEFINED)
		{
			this->_setTextureAddressMode(this->state.textureAddressMode);
			this->deviceState.textureAddressMode = this->state.textureAddressMode;
		}
		if (this->state.blendMode != this->deviceState.blendMode)
		{
			this->_setTextureBlendMode(this->state.blendMode);
			this->deviceState.blendMode = this->state.blendMode;
		}
		if (this->state.colorMode != this->deviceState.colorMode || this->state.colorModeAlpha != this->deviceState.colorModeAlpha)
		{
			this->_setTextureColorMode(this->state.colorMode, this->state.colorModeAlpha);
			this->deviceState.colorMode = this->state.colorMode;
			this->deviceState.colorModeAlpha = this->state.colorModeAlpha;
		}
		if (this->state.modelviewMatrixChanged && this->modelviewMatrix != this->deviceState.modelviewMatrix)
		{
			this->setMatrixMode(GL_MODELVIEW);
			glLoadMatrixf(this->modelviewMatrix.data);
			this->deviceState.modelviewMatrix = this->modelviewMatrix;
			this->state.modelviewMatrixChanged = false;
		}
		if (this->state.projectionMatrixChanged && this->projectionMatrix != this->deviceState.projectionMatrix)
		{
			this->setMatrixMode(GL_PROJECTION);
			glLoadMatrixf(this->projectionMatrix.data);
			this->deviceState.projectionMatrix = this->projectionMatrix;
			this->state.projectionMatrixChanged = false;
		}
	}

	void OpenGL_RenderSystem::_setClientState(unsigned int type, bool enabled)
	{
		enabled ? glEnableClientState(type) : glDisableClientState(type);
	}

	void OpenGL_RenderSystem::bindTexture(unsigned int textureId)
	{
		this->state.textureId = textureId;
	}

	Texture* OpenGL_RenderSystem::_createTexture(chstr filename)
	{
		return new OpenGL_Texture(filename);
	}

	Texture* OpenGL_RenderSystem::createTexture(int w, int h, unsigned char* rgba)
	{
		return new OpenGL_Texture(w, h, rgba);
	}
	
	Texture* OpenGL_RenderSystem::createTexture(int w, int h, Texture::Format format, Texture::Type type, Color color)
	{
		return new OpenGL_Texture(w, h, format, type, color);
	}

	PixelShader* OpenGL_RenderSystem::createPixelShader()
	{
		hlog::warn(april::logTag, "Pixel shaders are not implemented!");
		return NULL;
	}

	PixelShader* OpenGL_RenderSystem::createPixelShader(chstr filename)
	{
		hlog::warn(april::logTag, "Pixel shaders are not implemented!");
		return NULL;
	}

	VertexShader* OpenGL_RenderSystem::createVertexShader()
	{
		hlog::warn(april::logTag, "Vertex shaders are not implemented!");
		return NULL;
	}

	VertexShader* OpenGL_RenderSystem::createVertexShader(chstr filename)
	{
		hlog::warn(april::logTag, "Vertex shaders are not implemented!");
		return NULL;
	}

	void OpenGL_RenderSystem::clear(bool useColor, bool depth)
	{
		GLbitfield mask = 0;
		if (useColor)
		{
			mask |= GL_COLOR_BUFFER_BIT;
		}
		if (depth)
		{
			mask |= GL_DEPTH_BUFFER_BIT;
		}
		glClear(mask);
	}

	void OpenGL_RenderSystem::clear(bool depth, grect rect, Color color)
	{
		if (color != lastColor) // used to minimize redundant calls to OpenGL
		{
			glClearColor(color.r_f(), color.g_f(), color.b_f(), color.a_f());
			lastColor = color;
		}
		this->clear(true, depth);
	}
	
	void OpenGL_RenderSystem::setMatrixMode(unsigned int mode)
	{
		// performance call, minimize redundant calls to setMatrixMode
		if (this->deviceState.modeMatrix != mode)
		{
			this->deviceState.modeMatrix = mode;
			glMatrixMode(mode);
		}
	}

	void OpenGL_RenderSystem::_setVertexPointer(int stride, const void* pointer)
	{
		if (this->deviceState.strideVertex != stride || this->deviceState.pointerVertex != pointer)
		{
			this->deviceState.strideVertex = stride;
			this->deviceState.pointerVertex = pointer;
#ifdef _OPENGLES2
			glVertexAttribPointer(_positionSlot, 3, GL_FLOAT, GL_FALSE, stride, pointer);
#else
			glVertexPointer(3, GL_FLOAT, stride, pointer);
#endif
		}
	}
	
	void OpenGL_RenderSystem::_setTexCoordPointer(int stride, const void *pointer)
	{
		if (this->deviceState.strideTexCoord != stride || this->deviceState.pointerTexCoord != pointer)
		{
			this->deviceState.strideTexCoord = stride;
			this->deviceState.pointerTexCoord = pointer;
			glTexCoordPointer(2, GL_FLOAT, stride, pointer);
		}
	}
	
	void OpenGL_RenderSystem::_setColorPointer(int stride, const void *pointer)
	{
		if (this->deviceState.strideColor != stride || this->deviceState.pointerColor != pointer)
		{
			this->deviceState.strideColor = stride;
			this->deviceState.pointerColor = pointer;
			glColorPointer(4, GL_UNSIGNED_BYTE, stride, pointer);
		}
	}

	void OpenGL_RenderSystem::render(RenderOp renderOp, PlainVertex* v, int nVertices)
	{
		this->state.textureId = 0;
		this->state.textureCoordinatesEnabled = false;
		this->state.colorEnabled = false;
		this->state.systemColor.set(255, 255, 255, 255);
		this->_applyStateChanges();
		this->_setVertexPointer(sizeof(PlainVertex), v);
		glDrawArrays(gl_render_ops[renderOp], 0, nVertices);
	}

	void OpenGL_RenderSystem::render(RenderOp renderOp, PlainVertex* v, int nVertices, Color color)
	{
		this->state.textureId = 0;
		this->state.textureCoordinatesEnabled = false;
		this->state.colorEnabled = false;
		this->state.systemColor = color;
		this->_applyStateChanges();
		this->_setVertexPointer(sizeof(PlainVertex), v);
		glDrawArrays(gl_render_ops[renderOp], 0, nVertices);
	}
	
	void OpenGL_RenderSystem::render(RenderOp renderOp, TexturedVertex* v, int nVertices)
	{
		this->state.textureCoordinatesEnabled = true;
		this->state.colorEnabled = false;
		this->state.systemColor.set(255, 255, 255, 255);
		this->_applyStateChanges();
		this->_setVertexPointer(sizeof(TexturedVertex), v);
		this->_setTexCoordPointer(sizeof(TexturedVertex), &v->u);
		glDrawArrays(gl_render_ops[renderOp], 0, nVertices);
	}

	void OpenGL_RenderSystem::render(RenderOp renderOp, TexturedVertex* v, int nVertices, Color color)
	{
		this->state.textureCoordinatesEnabled = true;
		this->state.colorEnabled = false;
		this->state.systemColor = color;
		this->_applyStateChanges();
		this->_setVertexPointer(sizeof(TexturedVertex), v);
		this->_setTexCoordPointer(sizeof(TexturedVertex), (unsigned char*)v + 3 * sizeof(float)); // I forgot why this pointer is like that
		glDrawArrays(gl_render_ops[renderOp], 0, nVertices);
	}

	void OpenGL_RenderSystem::render(RenderOp renderOp, ColoredVertex* v, int nVertices)
	{
		this->state.textureId = 0;
		this->state.textureCoordinatesEnabled = false;
		this->state.colorEnabled = true;
		this->state.systemColor.set(255, 255, 255, 255);
		for_iter (i, 0, nVertices)
		{
			// making sure this is in AGBR order
			v[i].color = (((v[i].color & 0xFF000000) >> 24) | ((v[i].color & 0x00FF0000) >> 8) | ((v[i].color & 0x0000FF00) << 8) | ((v[i].color & 0x000000FF) << 24));
		}
		this->_applyStateChanges();
		this->_setVertexPointer(sizeof(ColoredVertex), v);
		this->_setColorPointer(sizeof(ColoredVertex), &v->color);
		glDrawArrays(gl_render_ops[renderOp], 0, nVertices);
	}
	
	void OpenGL_RenderSystem::render(RenderOp renderOp, ColoredTexturedVertex* v, int nVertices)
	{
		this->state.textureCoordinatesEnabled = true;
		this->state.colorEnabled = true;
		this->state.systemColor.set(255, 255, 255, 255);
		for_iter (i, 0, nVertices)
		{
			// making sure this is in AGBR order
			v[i].color = (((v[i].color & 0xFF000000) >> 24) | ((v[i].color & 0x00FF0000) >> 8) | ((v[i].color & 0x0000FF00) << 8) | ((v[i].color & 0x000000FF) << 24));
		}
		this->_applyStateChanges();
		this->_setVertexPointer(sizeof(ColoredTexturedVertex), v);
		this->_setColorPointer(sizeof(ColoredTexturedVertex), &v->color);
		this->_setTexCoordPointer(sizeof(ColoredTexturedVertex), &v->u);
		glDrawArrays(gl_render_ops[renderOp], 0, nVertices);
	}
	
	void OpenGL_RenderSystem::_setModelviewMatrix(const gmat4& matrix)
	{
		this->state.modelviewMatrix = matrix;
		this->state.modelviewMatrixChanged = true;
	}

	void OpenGL_RenderSystem::_setProjectionMatrix(const gmat4& matrix)
	{
		this->state.projectionMatrix = matrix;
		this->state.projectionMatrixChanged = true;
	}

	void OpenGL_RenderSystem::setParam(chstr name, chstr value)
	{
		if (name == "zbuffer")
		{
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_LEQUAL);
		}
	}
	
	ImageSource* OpenGL_RenderSystem::takeScreenshot(int bpp)
	{
		hlog::write(april::logTag, "Grabbing screenshot...");
		int w = april::window->getWidth();
		int h = april::window->getHeight();
		ImageSource* img = new ImageSource();
		img->w = w;
		img->h = h;
		img->bpp = bpp;
		img->format = (bpp == 4 ? AF_RGBA : AF_RGB);
		img->data = new unsigned char[w * (h + 1) * 4]; // 4 just in case some OpenGL implementations don't blit rgba and cause a memory leak
		unsigned char* temp = img->data + w * h * 4;
		
		glReadPixels(0, 0, w, h, (bpp == 4 ? GL_RGBA : GL_RGB), GL_UNSIGNED_BYTE, img->data);
		for_iter (y, 0, h / 2)
		{
			memcpy(temp, img->data + y * w * bpp, w * bpp);
			memcpy(img->data + y * w * bpp, img->data + (h - y - 1) * w * bpp, w * bpp);
			memcpy(img->data + (h - y - 1) * w * bpp, temp, w * bpp);
		}
		return img;
	}

}

#endif
