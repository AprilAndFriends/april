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
#elif _OPENGLES1
 #include <GLES/gl.h>
#else
 #ifdef _WIN32
  #include <windows.h>
 #endif
 #include <stdlib.h>
 #include <string.h>

 #ifndef __APPLE__
  #include <GL/gl.h>
  #include <GL/glu.h>
  #if HAVE_GLUT
   #include <GL/glut.h>
  #endif
 #else // __APPLE__
  #include <TargetConditionals.h>
  #if TARGET_OS_IPHONE
   #include <OpenGLES/ES1/gl.h>
   #include <OpenGLES/ES1/glext.h>
  #else
   #include <OpenGL/gl.h>
   #include <OpenGL/glu.h>
   #include <GLUT/glut.h>
  #endif // TARGET_OS_IPHONE
 #endif // __APPLE__
#endif // IPHONE_PLATFORM, _OPENGLES1, ...

#include <gtypes/Vector2.h>
#include <hltypes/util.h>

#include "ImageSource.h"
#include "Keys.h"
#include "OpenGL_RenderSystem.h"
#include "OpenGL_Texture.h"
#include "Timer.h"
#include "Window.h"

april::Timer globalTimer;

namespace april
{
	extern void (*g_logFunction)(chstr);

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
	
	OpenGL_RenderSystem::OpenGL_RenderSystem(Window* window) :
		mTexCoordsEnabled(false), mColorEnabled(false), RenderSystem()
	{		
		mWindow = window;
		
		glViewport(0, 0, window->getWidth(), window->getHeight());
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

	OpenGL_RenderSystem::~OpenGL_RenderSystem()
	{
		april::log("Destroying OpenGL Rendersystem");
	}

	hstr OpenGL_RenderSystem::getName()
	{
		return "OpenGL";
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
		april::log("creating user-defined GL texture");
		OpenGL_Texture* t = new OpenGL_Texture(rgba, w, h);
		return t;
	}
	
	Texture* OpenGL_RenderSystem::createEmptyTexture(int w, int h, TextureFormat fmt, TextureType type)
	{
		return NULL; // TODO
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

	void OpenGL_RenderSystem::clear(bool color, bool depth)
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
#if TARGET_OS_IPHONE || _OPENGLES1
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
			glBlendFunc(GL_SRC_ALPHA,GL_ONE);
		}
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
		mTextureFilter=filter;
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
#if !(TARGET_OS_IPHONE) && !(_OPENGLES1)
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
#else
#warning Compiling for an OpenGL ES target, setTextureWrapping cannot use GL_CLAMP
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
		glColor4f(1, 1, 1, mAlphaMultiplier); 
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
#if !(TARGET_OS_IPHONE) && !(_OPENGLES1)
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
		GLuint _color = *(GLuint*)colorB;
		for (int i = 0; i < nVertices; i++)
		{
			colors[i] = _color;
		}
		glColorPointer(4, GL_UNSIGNED_BYTE, 4, colors);
#endif
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
#if !(TARGET_OS_IPHONE) && !(_OPENGLES1)
		glColor4f(1, 1, 1, mAlphaMultiplier);
#endif
		glVertexPointer(3, GL_FLOAT, sizeof(PlainVertex), v);
		glDrawArrays(gl_render_ops[renderOp], 0, nVertices);
	}

	void OpenGL_RenderSystem::render(RenderOp renderOp,PlainVertex* v, int nVertices, Color color)
	{
		if (mTexCoordsEnabled)
		{
			glBindTexture(GL_TEXTURE_2D, 0);
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
			mTexCoordsEnabled = false;
		}
#if !(TARGET_OS_IPHONE) && !(_OPENGLES1)
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
		GLbyte colorB[4] = {(GLbyte)color.r, (GLbyte)color.g, (GLbyte)color.b, (GLbyte)(color.a * mAlphaMultiplier)};
		GLuint _color = *(GLuint*)colorB;
		for (int i = 0; i < nVertices; i++)
		{
			colors[i] = _color;
		}
		glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(color), colors);
#endif
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
#if !(TARGET_OS_IPHONE) && !(_OPENGLES1)
		glColor4f(1, 1, 1, mAlphaMultiplier);
#endif
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
#if !(TARGET_OS_IPHONE) && !(_OPENGLES1)
		glColor4f(1, 1, 1, mAlphaMultiplier);
#endif
		glVertexPointer(3, GL_FLOAT, sizeof(ColoredTexturedVertex), v);
		glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(ColoredTexturedVertex), &v->color);
		glTexCoordPointer(2, GL_FLOAT, sizeof(ColoredTexturedVertex), &v->u);
		glDrawArrays(gl_render_ops[renderOp], 0, nVertices);
	}
	
	void OpenGL_RenderSystem::setRenderTarget(Texture* source)
	{
		// TODO
	}
	
	void OpenGL_RenderSystem::beginFrame()
	{
		// TODO
	}

	void OpenGL_RenderSystem::setAlphaMultiplier(float value)
	{
		mAlphaMultiplier = value;
		glColor4f(1, 1, 1, value);
	}
	
/***************************************************/

	void createOpenGL_RenderSystem(Window* window)
	{
		april::log("Creating OpenGL Rendersystem");
		glEnable(GL_TEXTURE_2D);
		// DevIL defaults
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
#if !(TARGET_OS_IPHONE) && !(_OPENGLES1)
		glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
		glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
		glPixelStorei(GL_UNPACK_SWAP_BYTES, GL_FALSE);
#endif
		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		april::rendersys = new OpenGL_RenderSystem(window);
	}

}

#endif
