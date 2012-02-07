/// @file
/// @author  Kresimir Spes
/// @author  Ivan Vucica
/// @author  Boris Mikic
/// @version 1.33
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
#ifndef _ANDROID
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
#else
#include <OpenGL/gl.h>
#endif
#endif

#include <gtypes/Vector2.h>
#include <hltypes/hltypesUtil.h>

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
#else
	static EGLDisplay eglDisplay = 0;
	static EGLConfig eglConfig	= 0;
	static EGLSurface eglSurface = 0;
	static EGLContext eglContext = 0;
	static EGLint pi32ConfigAttribs[128] = {EGL_RED_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_BLUE_SIZE, 8, EGL_ALPHA_SIZE, 0, EGL_SURFACE_TYPE, EGL_WINDOW_BIT, EGL_NONE};
#endif
#endif

	void win_mat_invert()
	{
		glMatrixMode(GL_PROJECTION);
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
		GL_TRIANGLES,      // ROP_TRIANGLE_LIST
		GL_TRIANGLE_STRIP, // ROP_TRIANGLE_STRIP
		GL_TRIANGLE_FAN,   // ROP_TRIANGLE_FAN
		GL_LINES,          // ROP_LINE_LIST
		GL_LINE_STRIP,     // ROP_LINE_STRIP
		GL_POINTS,         // ROP_POINTS
	};
	
	OpenGL_RenderSystem::OpenGL_RenderSystem(hstr params) : RenderSystem(),
		mTexCoordsEnabled(false), mColorEnabled(false)
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
		glViewport(0, 0, window->getWidth(), window->getHeight());
		glClearColor(0, 0, 0, 1);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		
		glEnableClientState(GL_VERTEX_ARRAY);
        
		glEnable(GL_TEXTURE_2D);
        
		// DevIL defaults
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
#ifndef _OPENGLES1
		glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
		glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
		glPixelStorei(GL_UNPACK_SWAP_BYTES, GL_FALSE);
#endif
		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		
		if (mParams.contains("zbuffer"))
		{
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_LEQUAL);
		}
		mOrthoProjection.setSize((float)window->getWidth(), (float)window->getHeight());
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
		return new OpenGL_Texture(w, h);
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

	void OpenGL_RenderSystem::setTexture(Texture* t)
	{
		OpenGL_Texture* glt = (OpenGL_Texture*)t;
		if (t == NULL)
		{
			glBindTexture(GL_TEXTURE_2D, 0);
		}
		else
		{
			if (glt->mTexId == 0 && glt->isDynamic())
			{
				glt->load();
			}
			glt->_resetUnusedTimer();
			glBindTexture(GL_TEXTURE_2D, glt->mTexId);
			
			TextureFilter filter = t->getTextureFilter();
			if (filter != mTextureFilter)
			{
				setTextureFilter(filter);
			}
			bool wrapping = t->isTextureWrappingEnabled();
			if (mTextureWrapping != wrapping)
			{
				setTextureWrapping(wrapping);
			}
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
		glClearColor(color.r_f(), color.b_f(), color.g_f(), color.a_f());
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
		img->format = (bpp == 4 ? AT_RGBA : AT_RGB);
        img->data = (unsigned char*)malloc(w * (h + 1) * 4); // 4 just in case some OpenGL implementations don't blit rgba and cause a memory leak
        unsigned char* temp = img->data + w * h * 4;
        
	    glReadPixels(0, 0, w, h, (bpp == 4 ? GL_RGBA : GL_RGB), GL_UNSIGNED_BYTE, img->data);
		for (int y = 0; y < h / 2; y++)
		{
			memcpy(temp, img->data + y * w * bpp, w * bpp);
			memcpy(img->data + y * w * bpp, img->data + (h - y - 1) * w * bpp, w * bpp);
			memcpy(img->data + (h - y - 1) * w * bpp, temp, w * bpp);
		}
        return img;
    }

	void OpenGL_RenderSystem::_setModelviewMatrix(const gmat4& matrix)
	{
		glMatrixMode(GL_MODELVIEW);
		glLoadMatrixf(matrix.data);
	}

	void OpenGL_RenderSystem::_setProjectionMatrix(const gmat4& matrix)
	{
		glMatrixMode(GL_PROJECTION);
#ifdef _OPENGLES1
		glLoadIdentity();
		glRotatef(getWindow()->prefixRotationAngle(), 0, 0, 1);
		//printf("rotationangle %g\n", getWindow()->prefixRotationAngle());
		glMultMatrixf(matrix.data);
#else
		glLoadMatrixf(matrix.data);
#endif
		glMatrixMode(GL_MODELVIEW);
	}

	void OpenGL_RenderSystem::setBlendMode(BlendMode mode)
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
	
	void OpenGL_RenderSystem::setColorMode(ColorMode mode)
	{
		throw "NOT IMPLEMENTED";
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

	void OpenGL_RenderSystem::render(RenderOp renderOp, TexturedVertex* v, int nVertices)
	{
		if (!mTexCoordsEnabled)
		{
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			mTexCoordsEnabled = true;
		}
		if (mColorEnabled)
		{
			glDisableClientState(GL_COLOR_ARRAY);
			mColorEnabled = false;
		}
		glColor4f(1, 1, 1, 1); 
		glVertexPointer(3, GL_FLOAT, sizeof(TexturedVertex), v);
		glTexCoordPointer(2, GL_FLOAT, sizeof(TexturedVertex), &v->u);

		glDrawArrays(gl_render_ops[renderOp], 0, nVertices);

	}

	void OpenGL_RenderSystem::render(RenderOp renderOp, TexturedVertex* v, int nVertices, Color color)
	{
		if (!mTexCoordsEnabled)
		{
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			mTexCoordsEnabled = true;
		}
		if (mColorEnabled)
		{
			glDisableClientState(GL_COLOR_ARRAY);
			mColorEnabled = false;
		}
		glColor4f(color.r_f(), color.g_f(), color.b_f(), color.a_f());
		glVertexPointer(3, GL_FLOAT, sizeof(TexturedVertex), v);
		glTexCoordPointer(2, GL_FLOAT, sizeof(TexturedVertex), (char*)v + 3 * sizeof(float));
		glDrawArrays(gl_render_ops[renderOp], 0, nVertices);
	}

	void OpenGL_RenderSystem::render(RenderOp renderOp, PlainVertex* v, int nVertices)
	{
		if (mTexCoordsEnabled)
		{
			glBindTexture(GL_TEXTURE_2D, 0);
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
			mTexCoordsEnabled = false;
		}
		if (mColorEnabled)
		{
			glDisableClientState(GL_COLOR_ARRAY);
			mColorEnabled = false;
		}
		glColor4f(1, 1, 1, 1);
		glVertexPointer(3, GL_FLOAT, sizeof(PlainVertex), v);
		glDrawArrays(gl_render_ops[renderOp], 0, nVertices);
	}

	void OpenGL_RenderSystem::render(RenderOp renderOp, PlainVertex* v, int nVertices, Color color)
	{
		if (mTexCoordsEnabled)
		{
			glBindTexture(GL_TEXTURE_2D, 0);
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
			mTexCoordsEnabled = false;
		}
		if (mColorEnabled)
		{
			glDisableClientState(GL_COLOR_ARRAY);
			mColorEnabled = false;
		}
		glColor4f(color.r_f(), color.g_f(), color.b_f(), color.a_f());
		glVertexPointer(3, GL_FLOAT, sizeof(PlainVertex), v);
		glDrawArrays(gl_render_ops[renderOp], 0, nVertices);
	}
	
	void OpenGL_RenderSystem::render(RenderOp renderOp, ColoredVertex* v, int nVertices)
	{
		if (mTexCoordsEnabled)
		{
			glBindTexture(GL_TEXTURE_2D, 0);
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
			mTexCoordsEnabled = false;
		}
		if (!mColorEnabled)
		{
			glEnableClientState(GL_COLOR_ARRAY);
			mColorEnabled = true;
		}
		glEnableClientState(GL_COLOR_ARRAY);
		glVertexPointer(3, GL_FLOAT, sizeof(ColoredVertex), v);
		glColor4f(1, 1, 1, 1);
		glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(ColoredVertex), &v->color);
		glDrawArrays(gl_render_ops[renderOp], 0, nVertices);
	}
	
	void OpenGL_RenderSystem::render(RenderOp renderOp, ColoredTexturedVertex* v, int nVertices)
	{
		if (!mTexCoordsEnabled)
		{
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			mTexCoordsEnabled = true;
		}
		if (!mColorEnabled)
		{
			mColorEnabled = true;
			glEnableClientState(GL_COLOR_ARRAY);
		}
		for (int i = 0; i < nVertices; i++)
		{
			// making sure this is in AGBR order
			v[i].color = (((v[i].color & 0xFF000000) >> 24) | ((v[i].color & 0x00FF0000) >> 8) | ((v[i].color & 0x0000FF00) << 8) | ((v[i].color & 0x000000FF) << 24));
		}
		glVertexPointer(3, GL_FLOAT, sizeof(ColoredTexturedVertex), v);
		glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(ColoredTexturedVertex), &v->color);
		glTexCoordPointer(2, GL_FLOAT, sizeof(ColoredTexturedVertex), &v->u);
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
	void OpenGL_RenderSystem::setColorMode(ColorMode mode, unsigned char alpha)
	{
		// TODO not implemented in OpenGL yet
	}
	
	OpenGL_RenderSystem* OpenGL_RenderSystem::create(chstr options)
	{
		return new OpenGL_RenderSystem(options);
	}
}

#endif
