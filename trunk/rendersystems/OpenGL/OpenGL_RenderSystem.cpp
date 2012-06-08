/// @file
/// @author  Kresimir Spes
/// @author  Ivan Vucica
/// @author  Boris Mikic
/// @version 1.86
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _OPENGL

#if __APPLE__
#include <TargetConditionals.h>
#endif
#if TARGET_OS_IPHONE
#include <OpenGLES/ES1/gl.h>
#include <OpenGLES/ES1/glext.h>
#elif defined(_OPENGLES1)
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
#include <hltypes/hltypesUtil.h>

#include "april.h"
#include "ImageSource.h"
#include "Keys.h"
#include "OpenGL_RenderSystem.h"
#include "OpenGL_Texture.h"
#include "Timer.h"
#include "Window.h"

april::Timer globalTimer;

namespace april
{
#ifdef _WIN32
	static HWND hWnd = 0;
	HDC hDC = 0;
#ifndef _OPENGLES1
	static HGLRC hRC = 0;

	int _impl_getMaxTextureSize()
	{
		if (hRC == 0) return 0;
		int max;
		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max);
		return max;
	}

#else
	static EGLDisplay eglDisplay = 0;
	static EGLConfig eglConfig	= 0;
	static EGLSurface eglSurface = 0;
	static EGLContext eglContext = 0;
	static EGLint pi32ConfigAttribs[128] = {EGL_RED_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_BLUE_SIZE, 8, EGL_ALPHA_SIZE, 0, EGL_SURFACE_TYPE, EGL_WINDOW_BIT, EGL_NONE};

	int _impl_getMaxTextureSize()
	{
		if (eglDisplay == 0) return 0;
		int max;
		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max);
		return max;
	}

#endif
#endif

	void win_mat_invert()
	{
		((OpenGL_RenderSystem*) april::rendersys)->setMatrixMode(GL_PROJECTION);
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
		GL_TRIANGLES,	  // ROP_TRIANGLE_LIST
		GL_TRIANGLE_STRIP, // ROP_TRIANGLE_STRIP
		GL_TRIANGLE_FAN,   // ROP_TRIANGLE_FAN
		GL_LINES,		  // ROP_LINE_LIST
		GL_LINE_STRIP,	 // ROP_LINE_STRIP
		GL_POINTS,		 // ROP_POINTS
	};
	
	OpenGL_RenderState::OpenGL_RenderState() : 
		systemColor(0, 0, 0 ,255)
	{
		colorEnabled = texCoordsEnabled = modelviewMatrixSet = projectionMatrixSet = false;
		blendMode = (BlendMode) 10000;
		colorMode = (ColorMode) 10000;
		colorModeAlpha = 255;
		texId = 0;
	}
	
	OpenGL_RenderSystem::OpenGL_RenderSystem(hstr params) : RenderSystem()
	{
		mParams = params;
	}

	OpenGL_RenderSystem::~OpenGL_RenderSystem()
	{
		april::log("Destroying OpenGL Rendersystem");
#ifdef _WIN32
		_releaseWindow();
#endif
	}

#ifdef _WIN32
	void OpenGL_RenderSystem::_releaseWindow()
	{
#ifndef _OPENGLES1
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
 		mWindow = window;
#ifdef _WIN32
		hWnd = (HWND)mWindow->getIDFromBackend();
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
			april::log("can't create a GL device context");
			return;
		}
		GLuint pixelFormat = ChoosePixelFormat(hDC, &pfd);
		if (pixelFormat == 0)
		{
			april::log("can't find a suitable pixel format");
			_releaseWindow();
			return;
		}
		if (SetPixelFormat(hDC, pixelFormat, &pfd) == 0)
		{
			april::log("can't set the pixel format");
			_releaseWindow();
			return;
		}
#ifndef _OPENGLES1
		hRC = wglCreateContext(hDC);
		if (hRC == 0)
		{
			april::log("can't create a GL rendering context");
			_releaseWindow();
			return;
		}
		if (wglMakeCurrent(hDC, hRC) == 0)
		{
			april::log("can't activate the GL rendering context");
			_releaseWindow();
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
			april::log("can't get EGL display");
			_releaseWindow();
			return;
		}
		EGLint majorVersion;
		EGLint minorVersion;
		if (!eglInitialize(eglDisplay, &majorVersion, &minorVersion))
		{
			april::log("can't initialize EGL");
			_releaseWindow();
			return;
		}
		EGLint configs;
		EGLBoolean result = eglChooseConfig(eglDisplay, pi32ConfigAttribs, &eglConfig, 1, &configs);
		if (!result || configs == 0)
		{
			april::log("can't choose EGL config");
			_releaseWindow();
			return;
		}

		eglSurface = eglCreateWindowSurface(eglDisplay, eglConfig, (NativeWindowType)hWnd, NULL);
		if (eglGetError() != EGL_SUCCESS)
		{
			april::log("can't create window surface");
			_releaseWindow();
			return;
		}
		eglContext = eglCreateContext(eglDisplay, eglConfig, EGL_NO_CONTEXT, NULL);
		if (eglGetError() != EGL_SUCCESS)
		{
			april::log("can't create EGL context");
			_releaseWindow();
			return;
		}
		eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext);
		if (eglGetError() != EGL_SUCCESS)
		{
			april::log("can't make context current");
			_releaseWindow();
			return;
		}
#endif
#endif
		glViewport(0, 0, mWindow->getWidth(), mWindow->getHeight());
		glClearColor(0, 0, 0, 1);
		glColor4f(0, 0, 0, 1); // default color, to ensure systemColor works properly
		setMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		setMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnable(GL_TEXTURE_2D);
		// pixel data
#ifndef _OPENGLES1
		glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
		glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
		glPixelStorei(GL_UNPACK_SWAP_BYTES, GL_FALSE);
#endif
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		if (mParams.contains("zbuffer"))
		{
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_LEQUAL);
		}
		mOrthoProjection.setSize((float)mWindow->getWidth(), (float)mWindow->getHeight());
	}

	void OpenGL_RenderSystem::restore()
	{
		RenderSystem::restore();
		glViewport(0, 0, mWindow->getWidth(), mWindow->getHeight());
		glClearColor(0, 0, 0, 1);
		_setModelviewMatrix(mModelviewMatrix);
		_setProjectionMatrix(mProjectionMatrix);
		// GL defaults
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisableClientState(GL_COLOR_ARRAY);
		// pixel data
#ifndef _OPENGLES1
		glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
		glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
		glPixelStorei(GL_UNPACK_SWAP_BYTES, GL_FALSE);
#endif
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		// other
		if (mParams.contains("zbuffer"))
		{
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_LEQUAL);
		}
	}

	hstr OpenGL_RenderSystem::getName()
	{
		return "OpenGL";
	}
	
	void OpenGL_RenderSystem::setParam(chstr name, chstr value)
	{
		if (name == "zbuffer")
		{
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_LEQUAL);
		}
	}
	
	float OpenGL_RenderSystem::getPixelOffset()
	{
		return 0.0f;
	}

	Texture* OpenGL_RenderSystem::loadTexture(chstr filename, bool dynamic)
	{
		hstr name = findTextureFile(filename);
		if (name == "")
		{
			return NULL;
		}
		if (mDynamicLoading)
		{
			dynamic = true;
		}
		if (dynamic)
		{
			april::log("creating dynamic GL texture '" + name + "'");
		}
		OpenGL_Texture* t = new OpenGL_Texture(name, dynamic);
		if (!dynamic && !t->load())
		{
			delete t;
			return NULL;
		}
		return t;
	}

	Texture* OpenGL_RenderSystem::createTextureFromMemory(unsigned char* rgba, int w, int h)
	{
		return new OpenGL_Texture(rgba, w, h);
	}
	
	Texture* OpenGL_RenderSystem::createEmptyTexture(int w, int h, TextureFormat fmt, TextureType type)
	{
		return new OpenGL_Texture(w, h, fmt, type);
	}

	VertexShader* OpenGL_RenderSystem::createVertexShader()
	{
		return NULL;
	}

	PixelShader* OpenGL_RenderSystem::createPixelShader()
	{
		return NULL;
	}

	void OpenGL_RenderSystem::setVertexShader(VertexShader* vertexShader)
	{
	}

	void OpenGL_RenderSystem::setPixelShader(PixelShader* pixelShader)
	{
	}

	grect OpenGL_RenderSystem::getViewport()
	{
		static float params[4];
		glGetFloatv(GL_VIEWPORT, params);
		return grect(params[0], mWindow->getHeight() - params[3] - params[1], params[2], params[3]);
	}
	
	void OpenGL_RenderSystem::setViewport(grect rect)
	{
		// because GL has to defy screen logic and has (0,0) in the bottom left corner
		glViewport((int)rect.x, (int)(mWindow->getHeight() - rect.h - rect.y), (int)rect.w, (int)rect.h);
	}
	
	void OpenGL_RenderSystem::applyStateChanges()
	{
		if (mState.texCoordsEnabled != mDeviceState.texCoordsEnabled)
		{
			if (mState.texCoordsEnabled) glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			else                         glDisableClientState(GL_TEXTURE_COORD_ARRAY);
			mDeviceState.texCoordsEnabled = mState.texCoordsEnabled;
		}
		if (mState.colorEnabled != mDeviceState.colorEnabled)
		{
			if (mState.colorEnabled) glEnableClientState(GL_COLOR_ARRAY);
			else                     glDisableClientState(GL_COLOR_ARRAY);
			mDeviceState.colorEnabled = mState.colorEnabled;
		}
		if (mState.systemColor != mDeviceState.systemColor)
		{
			glColor4f(mState.systemColor.r_f(), mState.systemColor.g_f(), mState.systemColor.b_f(), mState.systemColor.a_f());
			mDeviceState.systemColor = mState.systemColor;
		}

		if (mState.texId != mDeviceState.texId)
		{
			glBindTexture(GL_TEXTURE_2D, mState.texId);
			mDeviceState.texId = mState.texId;
		}
		
		if (mState.blendMode != mDeviceState.blendMode)
		{
			_setBlendMode(mState.blendMode);
			mDeviceState.blendMode = mState.blendMode;
		}

		if (mState.colorMode != mDeviceState.colorMode || mState.colorModeAlpha != mDeviceState.colorModeAlpha)
		{
			_setColorMode(mState.colorMode, mState.colorModeAlpha);
			mDeviceState.colorMode = mState.colorMode;
			mDeviceState.colorModeAlpha = mState.colorModeAlpha;
		}

		if (mState.projectionMatrixSet && mProjectionMatrix != mDeviceState.projectionMatrix)
		{
			setMatrixMode(GL_PROJECTION);
			glLoadMatrixf(mProjectionMatrix.data);
			mDeviceState.projectionMatrix = mProjectionMatrix;
			mState.projectionMatrixSet = 0;
		}
		
		if (mState.modelviewMatrixSet && mModelviewMatrix != mDeviceState.modelviewMatrix)
		{
			setMatrixMode(GL_MODELVIEW);
			glLoadMatrixf(mModelviewMatrix.data);
			mDeviceState.modelviewMatrix = mModelviewMatrix;
			mState.modelviewMatrixSet = 0;
		}
	}
	
	void OpenGL_RenderSystem::bindTexture(unsigned int tex_id)
	{
		mState.texId = tex_id;
	}

	void OpenGL_RenderSystem::setTexture(Texture* t)
	{
		OpenGL_Texture* texture = (OpenGL_Texture*) t;
		if (texture == NULL)
		{
			bindTexture(0);
		}
		else
		{
			TextureFilter filter = texture->getTextureFilter();
			if (filter != mTextureFilter)
			{
				setTextureFilter(filter);
			}
			bool wrapping = texture->isTextureWrappingEnabled();
			if (mTextureWrapping != wrapping)
			{
				setTextureWrapping(wrapping);
			}
			// filtering and wrapping applied before loading texture data, iOS OpenGL guidelines suggest it as an optimization
			if (texture->mTexId == 0)
			{
				texture->load();
			}
			texture->_resetUnusedTimer();
			bindTexture(texture->mTexId);
		}
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

	void OpenGL_RenderSystem::clear(bool useColor, bool depth, grect rect, Color color)
	{
		static Color last_color(0, 0, 0, 255);
		if (color != last_color) // use this to minimize rendundant calls to OpenGL
		{
			glClearColor(color.r_f(), color.b_f(), color.g_f(), color.a_f());
			last_color = color;
		}
		clear(useColor, depth);
	}
	
	ImageSource* OpenGL_RenderSystem::grabScreenshot(int bpp)
	{
		april::log("grabbing screenshot");
		int w = mWindow->getWidth();
		int h = mWindow->getHeight();
		ImageSource* img = new ImageSource();
		img->w = w;
		img->h = h;
		img->bpp = bpp;
		img->format = (bpp == 4 ? AF_RGBA : AF_RGB);
		img->data = (unsigned char*)malloc(w * (h + 1) * 4); // 4 just in case some OpenGL implementations don't blit rgba and cause a memory leak
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

	void OpenGL_RenderSystem::_setModelviewMatrix(const gmat4& matrix)
	{
		mState.modelviewMatrix = matrix;
		mState.modelviewMatrixSet = true;
	}

	void OpenGL_RenderSystem::_setProjectionMatrix(const gmat4& matrix)
	{
		mState.projectionMatrix = matrix;
		mState.projectionMatrixSet = true;

	}

	void OpenGL_RenderSystem::_setBlendMode(BlendMode mode)
	{
		static int blendSeparationSupported = -1;
		if (blendSeparationSupported == -1)
		{
			// determine if blend separation is possible on first call to this function
			hstr extensions = (const char*)glGetString(GL_EXTENSIONS);
#ifdef _OPENGLES1
			blendSeparationSupported = extensions.contains("OES_blend_equation_separate") && extensions.contains("OES_blend_func_separate");
#else
			blendSeparationSupported = extensions.contains("GL_EXT_blend_equation_separate") && extensions.contains("GL_EXT_blend_func_separate");
#endif
		}
		if (blendSeparationSupported)
		{
			switch (mode)
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
			}
		}
		else
		{
			if (mode == ALPHA_BLEND || mode == DEFAULT)
			{
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			}
			else if (mode == ADD)
			{
				glBlendFunc(GL_SRC_ALPHA, GL_ONE);
			}
		}
	}
	
	void OpenGL_RenderSystem::setBlendMode(BlendMode mode)
	{
		mState.blendMode = mode;
	}
	
	void OpenGL_RenderSystem::_setColorMode(ColorMode mode, unsigned char alpha)
	{
		float constColor[4] = {alpha / 255.0f, alpha / 255.0f, alpha / 255.0f, alpha / 255.0f};
		switch (mode)
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
		}
	}

	void OpenGL_RenderSystem::setColorMode(ColorMode mode, unsigned char alpha)
	{
		mState.colorMode = mode;
		mState.colorModeAlpha = alpha;
	}
	
	void OpenGL_RenderSystem::setTextureFilter(TextureFilter filter)
	{
		if (filter == Linear)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		}
		else if (filter == Nearest)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		}
		else
		{
			april::log("trying to set unsupported texture filter!");
		}
		mTextureFilter = filter;
	}

	void OpenGL_RenderSystem::setTextureWrapping(bool wrap)
	{
		if (wrap)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		}
		else	
		{
#ifndef _OPENGLES1
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
#else
//warning Compiling for an OpenGL ES target, setTextureWrapping cannot use GL_CLAMP
#endif
		}
		mTextureWrapping = wrap;
	}

	void OpenGL_RenderSystem::setMatrixMode(unsigned int mode)
	{
		// performance call, minimize redundant calls to setMatrixMode
		static unsigned int _mode = 0;
		if (_mode != mode)
		{
			_mode = mode;
			glMatrixMode(mode);
		}
	}
	
	void OpenGL_RenderSystem::_setVertexPointer(int stride, const void *pointer)
	{
		static int _stride = 0;
		static const void *_pointer = 0;
		
		if (_stride == stride && _pointer == pointer) return;
		
		glVertexPointer(3, GL_FLOAT, stride, pointer);
		
		_stride = stride;
		_pointer = _pointer;
	}
	
	void OpenGL_RenderSystem::_setTexCoordPointer(int stride, const void *pointer)
	{
		static int _stride = 0;
		static const void *_pointer = 0;
		
		if (_stride == stride && _pointer == pointer) return;
		
		glTexCoordPointer(2, GL_FLOAT, stride, pointer);
		
		_stride = stride;
		_pointer = _pointer;
	}
	
	void OpenGL_RenderSystem::_setColorPointer(int stride, const void *pointer)
	{
		static int _stride = 0;
		static const void *_pointer = 0;
		
		if (_stride == stride && _pointer == pointer) return;
		
		glColorPointer(4, GL_UNSIGNED_BYTE, stride, pointer);
		
		_stride = stride;
		_pointer = _pointer;
	}

	void OpenGL_RenderSystem::render(RenderOp renderOp, TexturedVertex* v, int nVertices)
	{
		mState.texCoordsEnabled = true;
		mState.colorEnabled = false;
		mState.systemColor = april::Color(255, 255, 255, 255);
		applyStateChanges();
		_setVertexPointer(sizeof(TexturedVertex), v);
		_setTexCoordPointer(sizeof(TexturedVertex), &v->u);
		glDrawArrays(gl_render_ops[renderOp], 0, nVertices);
	}

	void OpenGL_RenderSystem::render(RenderOp renderOp, TexturedVertex* v, int nVertices, Color color)
	{
		mState.texCoordsEnabled = true;
		mState.colorEnabled = false;
		mState.systemColor = color;
		applyStateChanges();
	
		_setVertexPointer(sizeof(TexturedVertex), v);
		_setTexCoordPointer(sizeof(TexturedVertex), (char*)v + 3 * sizeof(float));
		glDrawArrays(gl_render_ops[renderOp], 0, nVertices);
	}

	void OpenGL_RenderSystem::render(RenderOp renderOp, PlainVertex* v, int nVertices)
	{
		mState.texId = 0;
		mState.texCoordsEnabled = false;
		mState.colorEnabled = false;
		mState.systemColor = april::Color(255, 255, 255, 255);
		applyStateChanges();

		_setVertexPointer(sizeof(PlainVertex), v);
		glDrawArrays(gl_render_ops[renderOp], 0, nVertices);
	}

	void OpenGL_RenderSystem::render(RenderOp renderOp, PlainVertex* v, int nVertices, Color color)
	{
		mState.texId = 0;
		mState.texCoordsEnabled = false;
		mState.colorEnabled = false;
		mState.systemColor = color;
		applyStateChanges();
	
		_setVertexPointer(sizeof(PlainVertex), v);
		glDrawArrays(gl_render_ops[renderOp], 0, nVertices);
	}
	
	void OpenGL_RenderSystem::render(RenderOp renderOp, ColoredVertex* v, int nVertices)
	{
		mState.texId = 0;
		mState.texCoordsEnabled = false;
		mState.colorEnabled = true;
		mState.systemColor = april::Color(255, 255, 255, 255);
		applyStateChanges();
	
		_setVertexPointer(sizeof(ColoredVertex), v);
		_setColorPointer(sizeof(ColoredVertex), &v->color);
		glDrawArrays(gl_render_ops[renderOp], 0, nVertices);
	}
	
	void OpenGL_RenderSystem::render(RenderOp renderOp, ColoredTexturedVertex* v, int nVertices)
	{
		mState.texCoordsEnabled = true;
		mState.colorEnabled = true;
		mState.systemColor = april::Color(255, 255, 255, 255);
		applyStateChanges();

		for_iter (i, 0, nVertices)
		{
			// making sure this is in AGBR order
			v[i].color = (((v[i].color & 0xFF000000) >> 24) | ((v[i].color & 0x00FF0000) >> 8) | ((v[i].color & 0x0000FF00) << 8) | ((v[i].color & 0x000000FF) << 24));
		}
		_setVertexPointer(sizeof(ColoredTexturedVertex), v);
		_setColorPointer(sizeof(ColoredTexturedVertex), &v->color);
		_setTexCoordPointer(sizeof(ColoredTexturedVertex), &v->u);
		glDrawArrays(gl_render_ops[renderOp], 0, nVertices);
	}
	
	Texture* OpenGL_RenderSystem::getRenderTarget()
	{
		// TODO
		return NULL;
	}
	
	void OpenGL_RenderSystem::setRenderTarget(Texture* source)
	{
		// TODO
	}
	
	void OpenGL_RenderSystem::beginFrame()
	{
		// TODO ?
	}

	harray<DisplayMode> OpenGL_RenderSystem::getSupportedDisplayModes()
	{
		// TODO
		return harray<DisplayMode>();
	}
	
	void OpenGL_RenderSystem::setResolution(int w, int h)
	{
		// TODO: OpenGL_RenderSystem::setResolution()
		april::log("WARNING: setResolution ignored!");
	}

	OpenGL_RenderSystem* OpenGL_RenderSystem::create(chstr options)
	{
		return new OpenGL_RenderSystem(options);
	}

}

#endif
