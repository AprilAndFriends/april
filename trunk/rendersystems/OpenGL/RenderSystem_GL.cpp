/************************************************************************************\
This source file is part of the Awesome Portable Rendering Interface Library         *
For latest info, see http://libapril.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes, Ivan Vucica                                        *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#ifdef _OPENGL

#ifdef IPHONE_PLATFORM
#include <OpenGLES/ES1/gl.h>
#else
#ifdef _WIN32
#include <windows.h>
#endif
#include <stdlib.h>

#ifndef __APPLE__
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#else // __APPLE__
#include <TargetConditionals.h>
#if TARGET_OS_IPHONE
#include <OpenGLES/ES1/gl.h>
#include <OpenGLES/ES1/glext.h>
#else
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#endif
#endif // __APPLE__

#include <gtypes/Vector2.h>
#include <hltypes/util.h>

#include "ImageSource.h"
#include "Keys.h"
#include "RenderSystem_GL.h"
#include "Timer.h"
#include "Window.h"

april::Timer globalTimer;

namespace april
{
	extern void (*g_logFunction)(chstr);

	float cursor_x = 0.0f;
	float cursor_y = 0.0f;

	unsigned int platformLoadGLTexture(const char* name, int* w, int* h)
	{
		GLuint texid;
		ImageSource* img = loadImage(name);
		if (!img)
		{
			return 0;
		}
		*w = img->w;
		*h = img->h;
		glGenTextures(1, &texid);
		glBindTexture(GL_TEXTURE_2D, texid);
		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		
		switch (img->format)
		{
#if TARGET_OS_IPHONE
		case GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG:
		case GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG:
			int mipmaplevel = 0;
			glCompressedTexImage2D(GL_TEXTURE_2D, mipmaplevel, img->format, img->w, img->h, 0, img->compressedLength, img->data);
			break;
#endif
		default:
			glTexImage2D(GL_TEXTURE_2D, 0, img->bpp == 4 ? GL_RGBA : GL_RGB, img->w,img->h, 0, img->format, GL_UNSIGNED_BYTE,img->data);
			break;
		}
		delete img;
		
		return texid;
	}
	
	void win_mat_invert()
	{
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		float mat[16];
		glGetFloatv(GL_PROJECTION_MATRIX, mat);
		hswap(&mat[1], &mat[0]);
		hswap(&mat[4], &mat[5]);
		mat[5] = -mat[5];
		mat[13] = -mat[13];
		glLoadMatrixf(mat);
	}
#endif
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
	
	GLTexture::GLTexture(chstr filename, bool dynamic)
	{
		mWidth = 0;
		mHeight = 0;
		mFilename = filename;
		mDynamic = dynamic;
		mTexId = 0;
	}

	GLTexture::GLTexture(unsigned char* rgba, int w, int h)
	{
		mWidth = w;
		mHeight = h;
		mDynamic = 0;
		mFilename = "UserTexture";
		glGenTextures(1, &mTexId);
		glBindTexture(GL_TEXTURE_2D, mTexId);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, 4, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba);

	}

	GLTexture::~GLTexture()
	{
		unload();
	}

	bool GLTexture::load()
	{
		mUnusedTimer = 0;
		if (mTexId)
		{
			return true;
		}
		april::log("loading GL texture '" + mFilename + "'");
		mTexId = platformLoadGLTexture(mFilename.c_str(), &mWidth, &mHeight);
		if (!mTexId)
		{
			april::log("Failed to load texture: " + mFilename);
			return false;
		}
		foreach (Texture*, it, mDynamicLinks)
		{
			((GLTexture*)(*it))->load();
		}
		return true;
	}

	bool GLTexture::isLoaded()
	{
		return (mTexId != 0);
	}

	void GLTexture::unload()
	{
		if (mTexId != 0)
		{
			april::log("unloading GL texture '" + mFilename + "'");
			glDeleteTextures(1, &mTexId);
			mTexId = 0;
		}
	}

	int GLTexture::getSizeInBytes()
	{
		return (mWidth * mHeight * 3);
	}


	GLRenderSystem::GLRenderSystem(Window* window) :
		mTexCoordsEnabled(false), mColorEnabled(false), RenderSystem()
	{		
		mWindow = window;
		
		glViewport(0, 0, window->getWindowWidth(), window->getWindowHeight());
		glClearColor(0, 0, 0, 1);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		
		glEnableClientState(GL_VERTEX_ARRAY);
        //glEnable(GL_DEPTH_TEST);
        //glDepthFunc(GL_GREATER);
        //glClearDepth(1.0f);
        
		//glEnable(GL_CULL_FACE);
	}

	GLRenderSystem::~GLRenderSystem()
	{
		april::log("Destroying OpenGL Rendersystem");
	}

	hstr GLRenderSystem::getName()
	{
		return "OpenGL";
	}
	
	float GLRenderSystem::getPixelOffset()
	{
		return 0.0f;
	}

	Texture* GLRenderSystem::loadTexture(chstr filename, bool dynamic)
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
		GLTexture* t = new GLTexture(name, dynamic);
		if (!dynamic && !t->load())
		{
			delete t;
			return NULL;
		}
		return t;
	}

	Texture* GLRenderSystem::createTextureFromMemory(unsigned char* rgba, int w, int h)
	{
		april::log("creating user-defined GL texture");
		GLTexture* t = new GLTexture(rgba, w, h);
		return t;
	}
	
	Texture* GLRenderSystem::createEmptyTexture(int w, int h, TextureFormat fmt, TextureType type)
	{
		return NULL; // TODO
	}

	void GLRenderSystem::setTexture(Texture* t)
	{
		GLTexture* glt = (GLTexture*)t;
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

	void GLRenderSystem::clear(bool color, bool depth)
	{
		GLbitfield mask = 0;
		if (color)
		{
			mask |= GL_COLOR_BUFFER_BIT;
		}
		if (depth)
		{
			mask |= GL_DEPTH_BUFFER_BIT;
		}
		glClear(mask);
	}
    
    ImageSource* GLRenderSystem::grabScreenshot()
    {
        april::log("grabbing screenshot");
        int w = mWindow->getWindowWidth();
		int h = mWindow->getWindowHeight();
        ImageSource* img = new ImageSource();
        img->w = w;
		img->h = h;
		img->bpp = 3;
		img->format = AT_RGB;
        img->data = (unsigned char*)malloc(w * (h + 1) * 4); // 4 just in case some OpenGL implementations don't blit rgba and cause a memory leak
        unsigned char* temp = img->data + w * h * 4;
        
        glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, img->data);
        
        for (int y = 0; y < h / 2; y++)
        {
            memcpy(temp, img->data + y * w * 3, w * 3);
            memcpy(img->data + y * w * 3, img->data + (h - y - 1) * w * 3, w * 3);
            memcpy(img->data + (h - y - 1) * w * 3, temp, w * 3);
        }
        return img;
    }

	void GLRenderSystem::_setModelviewMatrix(const gmat4& matrix)
	{
		glMatrixMode(GL_MODELVIEW);
		glLoadMatrixf(matrix.mat);
	}

	void GLRenderSystem::_setProjectionMatrix(const gmat4& matrix)
	{
		glMatrixMode(GL_PROJECTION);
#if TARGET_OS_IPHONE
		glLoadIdentity();
		glRotatef(getWindow()->prefixRotationAngle(), 0, 0, 1);
		//printf("rotationangle %g\n", getWindow()->prefixRotationAngle());
		glMultMatrixf(matrix.mat);
#else
		glLoadMatrixf(matrix.mat);
#endif
		glMatrixMode(GL_MODELVIEW);
	}

	void GLRenderSystem::setBlendMode(BlendMode mode)
	{
		if (mode == ALPHA_BLEND || mode == DEFAULT)
		{
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}
		else if (mode == ADD)
		{
			glBlendFunc(GL_SRC_ALPHA,GL_ONE);
		}
	}
	
	void GLRenderSystem::setTextureFilter(TextureFilter filter)
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
		mTextureFilter=filter;
	}

	void GLRenderSystem::setTextureWrapping(bool wrap)
	{
		if (wrap)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		}
		else	
		{
#if !(TARGET_OS_IPHONE)
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
#else
#warning Compiling for iPhone, setTextureWrapping cannot use GL_CLAMP
#endif
		}
		mTextureWrapping = wrap;
	}

	void GLRenderSystem::render(RenderOp renderOp, TexturedVertex* v, int nVertices)
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
		glColor4f(1, 1, 1, mAlphaMultiplier); 
		glVertexPointer(3, GL_FLOAT, sizeof(TexturedVertex), v);
		glTexCoordPointer(2, GL_FLOAT, sizeof(TexturedVertex), &v->u);

		glDrawArrays(gl_render_ops[renderOp], 0, nVertices);

	}

	void GLRenderSystem::render(RenderOp renderOp, TexturedVertex* v, int nVertices, Color color)
	{
		if (!mTexCoordsEnabled)
		{
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			mTexCoordsEnabled = true;
		}
#if !(TARGET_OS_IPHONE)
		if (mColorEnabled)
		{
			glDisableClientState(GL_COLOR_ARRAY);
			mColorEnabled = false;
		}
		glColor4f(color.r_f(), color.g_f(), color.b_f(), color.a_f() * mAlphaMultiplier);
#else
		if (!mColorEnabled)
		{
			glEnableClientState(GL_COLOR_ARRAY);
			mColorEnabled = true;
		}
		GLuint colors[nVertices];
		GLbyte colorB[4] = {(GLbyte)color.r, (GLbyte)color.g, (GLbyte)color.b, (GLbyte)color.a};
		GLuint color = *(GLuint*)colorB;
		for (int i = 0; i < nVertices; i++)
		{
			colors[i] = color;
		}
		glColorPointer(4, GL_UNSIGNED_BYTE, 4, colors);
#endif
		glVertexPointer(3, GL_FLOAT, sizeof(TexturedVertex), v);
		glTexCoordPointer(2, GL_FLOAT, sizeof(TexturedVertex), (char*)v + 3 * sizeof(float));
		glDrawArrays(gl_render_ops[renderOp], 0, nVertices);
	}

	void GLRenderSystem::render(RenderOp renderOp, PlainVertex* v, int nVertices)
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
#if !(TARGET_OS_IPHONE)
		glColor4f(1, 1, 1, mAlphaMultiplier);
#endif
		glVertexPointer(3, GL_FLOAT, sizeof(PlainVertex), v);
		glDrawArrays(gl_render_ops[renderOp], 0, nVertices);
	}

	void GLRenderSystem::render(RenderOp renderOp,PlainVertex* v, int nVertices, Color color)
	{
		if (mTexCoordsEnabled)
		{
			glBindTexture(GL_TEXTURE_2D, 0);
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
			mTexCoordsEnabled = false;
		}
#if !(TARGET_OS_IPHONE)
		if (mColorEnabled)
		{
			mColorEnabled = false;
			glDisableClientState(GL_COLOR_ARRAY);
		}
		glColor4f(color.r_f(), color.g_f(), color.b_f(), color.a_f() * mAlphaMultiplier);
#else
		if (!mColorEnabled)
		{
			glEnableClientState(GL_COLOR_ARRAY);
			mColorEnabled = true;
		}
		GLuint colors[nVertices];
		GLbyte colorB[4] = {(GLbyte)color.r, (GLbyte)color.g, (GLbyte)color.b, (GLbyte)(color.a * mAlphaMultiplier};
		GLuint color = *(GLuint*)colorB;
		for (int i = 0; i < nVertices; i++)
		{
			colors[i] = color;
		}
		glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(color), colors);
#endif
		glVertexPointer(3, GL_FLOAT, sizeof(PlainVertex), v);
		glDrawArrays(gl_render_ops[renderOp], 0, nVertices);
	}
	
	void GLRenderSystem::render(RenderOp renderOp, ColoredVertex* v, int nVertices)
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
#if !(TARGET_OS_IPHONE)
		glColor4f(1, 1, 1, mAlphaMultiplier);
#endif
		glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(ColoredVertex), &v->color);
		glDrawArrays(gl_render_ops[renderOp], 0, nVertices);
	}
	
	void GLRenderSystem::render(RenderOp renderOp, ColoredTexturedVertex* v, int nVertices)
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
#if !(TARGET_OS_IPHONE)
		glColor4f(1, 1, 1, mAlphaMultiplier);
#endif
		glVertexPointer(3, GL_FLOAT, sizeof(ColoredTexturedVertex), v);
		glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(ColoredTexturedVertex), &v->color);
		glTexCoordPointer(2, GL_FLOAT, sizeof(ColoredTexturedVertex), &v->u);
		glDrawArrays(gl_render_ops[renderOp], 0, nVertices);
	}
	
	void GLRenderSystem::setRenderTarget(Texture* source)
	{
		// TODO
	}
	
	void GLRenderSystem::beginFrame()
	{
		// TODO
	}

	void GLRenderSystem::setAlphaMultiplier(float value)
	{
		mAlphaMultiplier = value;
		glColor4f(1, 1, 1, value);
	}
	
/***************************************************/

	void createGLRenderSystem(Window* window)
	{
		g_logFunction("Creating OpenGL Rendersystem");
		glEnable(GL_TEXTURE_2D);
		// DevIL defaults
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
#if !(TARGET_OS_IPHONE)
		glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
		glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
		glPixelStorei(GL_UNPACK_SWAP_BYTES, GL_FALSE);
#endif
		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		april::rendersys = new GLRenderSystem(window);
	}

}

#endif
