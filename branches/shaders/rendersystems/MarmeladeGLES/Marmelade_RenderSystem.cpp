/// @file
/// @author  Domagoj Cerjan
/// @version 1.31
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef HAVE_MARMELADE

#include <s3e.h>
#include <GLES/gl.h>
#include <GLES/egl.h>

#include <gtypes/Vector2.h>
#include <hltypes/util.h>

#include "ImageSource.h"
#include "Keys.h"
#include "Marmelade_RenderSystem.h"
#include "Marmelade_Texture.h"
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
	
	Marmelade_RenderSystem::Marmelade_RenderSystem() : RenderSystem(),
		mTexCoordsEnabled(false), mColorEnabled(false)
	{		
	}

	Marmelade_RenderSystem::~Marmelade_RenderSystem()
	{
		april::log("Destroying OpenGL Rendersystem");
	}

	void Marmelade_RenderSystem::assignWindow(Window* window)
	{
		mWindow = window;
		
		glViewport(0, 0, window->getWidth(), window->getHeight());
		glClearColor(0, 0, 0, 1);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		
		glEnableClientState(GL_VERTEX_ARRAY);
        
		glEnable(GL_TEXTURE_2D);
        
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glPixelStorei(GL_PACK_ALIGNMENT, 1);

        //glEnable(GL_DEPTH_TEST);
        //glDepthFunc(GL_GREATER);
        //glClearDepth(1.0f);
		//glEnable(GL_CULL_FACE);
	}

	hstr Marmelade_RenderSystem::getName()
	{
		return "Marmelade OpenGL";
	}
	
	float Marmelade_RenderSystem::getPixelOffset()
	{
		return 0.0f;
	}

	Texture* Marmelade_RenderSystem::loadTexture(chstr filename, bool dynamic)
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
		Marmelade_Texture* t = new Marmelade_Texture(name, dynamic);
		if (!dynamic && !t->load())
		{
			delete t;
			return NULL;
		}
		return t;
	}

	Texture* Marmelade_RenderSystem::createTextureFromMemory(unsigned char* rgba, int w, int h)
	{
		april::log("creating user-defined GL texture");
		Marmelade_Texture* t = new Marmelade_Texture(rgba, w, h);
		return t;
	}
	
	Texture* Marmelade_RenderSystem::createEmptyTexture(int w, int h, TextureFormat fmt, TextureType type)
	{
		return NULL; // TODO
	}

	void Marmelade_RenderSystem::setTexture(Texture* t)
	{
		Marmelade_Texture* glt = (Marmelade_Texture*)t;
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

	void Marmelade_RenderSystem::clear(bool color, bool depth)
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

	void Marmelade_RenderSystem::clear(bool useColor, bool depth, grect rect, Color color)
	{
		// TODO
	}

	Texture* Marmelade_RenderSystem::getRenderTarget()
	{
		// TODO
	}
    
    ImageSource* Marmelade_RenderSystem::grabScreenshot(int bpp)
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

	void Marmelade_RenderSystem::_setModelviewMatrix(const gmat4& matrix)
	{
		glMatrixMode(GL_MODELVIEW);
		glLoadMatrixf(matrix.data);
	}

	void Marmelade_RenderSystem::_setProjectionMatrix(const gmat4& matrix)
	{
		glMatrixMode(GL_PROJECTION);
		glLoadMatrixf(matrix.data);
		glMatrixMode(GL_MODELVIEW);
	}

	void Marmelade_RenderSystem::setBlendMode(BlendMode mode)
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
	
	void Marmelade_RenderSystem::setTextureFilter(TextureFilter filter)
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

	void Marmelade_RenderSystem::setTextureWrapping(bool wrap)
	{
		if (wrap)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		}
		else	
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		}
		mTextureWrapping = wrap;
	}

	void Marmelade_RenderSystem::render(RenderOp renderOp, TexturedVertex* v, int nVertices)
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
		glVertexPointer(3, GL_FLOAT, sizeof(TexturedVertex), v);
		glTexCoordPointer(2, GL_FLOAT, sizeof(TexturedVertex), &v->u);

		glDrawArrays(gl_render_ops[renderOp], 0, nVertices);

	}

	void Marmelade_RenderSystem::render(RenderOp renderOp, TexturedVertex* v, int nVertices, Color color)
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

	void Marmelade_RenderSystem::render(RenderOp renderOp, PlainVertex* v, int nVertices)
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

		glVertexPointer(3, GL_FLOAT, sizeof(PlainVertex), v);
		glDrawArrays(gl_render_ops[renderOp], 0, nVertices);
	}

	void Marmelade_RenderSystem::render(RenderOp renderOp, PlainVertex* v, int nVertices, Color color)
	{
		if (mTexCoordsEnabled)
		{
			glBindTexture(GL_TEXTURE_2D, 0);
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
			mTexCoordsEnabled = false;
		}
		if (mColorEnabled)
		{
			mColorEnabled = false;
			glDisableClientState(GL_COLOR_ARRAY);
		}
		glColor4f(color.r_f(), color.g_f(), color.b_f(), color.a_f());
		glVertexPointer(3, GL_FLOAT, sizeof(PlainVertex), v);
		glDrawArrays(gl_render_ops[renderOp], 0, nVertices);
	}
	
	void Marmelade_RenderSystem::render(RenderOp renderOp, ColoredVertex* v, int nVertices)
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
		glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(ColoredVertex), &v->color);
		glDrawArrays(gl_render_ops[renderOp], 0, nVertices);
	}
	
	void Marmelade_RenderSystem::render(RenderOp renderOp, ColoredTexturedVertex* v, int nVertices)
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

		glVertexPointer(3, GL_FLOAT, sizeof(ColoredTexturedVertex), v);
		glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(ColoredTexturedVertex), &v->color);
		glTexCoordPointer(2, GL_FLOAT, sizeof(ColoredTexturedVertex), &v->u);
		glDrawArrays(gl_render_ops[renderOp], 0, nVertices);
	}
	
	void Marmelade_RenderSystem::setRenderTarget(Texture* source)
	{
		// TODO
	}
	
	void Marmelade_RenderSystem::beginFrame()
	{
		// TODO
	}

	void Marmelade_RenderSystem::setAlphaMultiplier(float value)
	{
		// LOL
	}
	
	harray<DisplayMode> Marmelade_RenderSystem::getSupportedDisplayModes()
	{
		return harray<DisplayMode>();
	}

	void clear(bool useColor, bool depth)
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
	
	Marmelade_RenderSystem* Marmelade_RenderSystem::create(chstr options)
	{
		return new Marmelade_RenderSystem();
	}
	
	void Marmelade_RenderSystem::setResolution(int w, int h)
	{
#if !defined(HAVE_MARMELADE)
#warning TODO: OpenGL_RenderSystem::setResolution()
		april::log("WARNING: %s ignored!", __PRETTY_FUNCTION__);
#endif
	}

}

#endif
