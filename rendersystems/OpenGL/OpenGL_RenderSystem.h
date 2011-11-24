/************************************************************************************\
This source file is part of the Awesome Portable Rendering Interface Library         *
For latest info, see http://libapril.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes                                                     *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#if defined(_OPENGL) || _OPENGLES1
#ifndef APRIL_OPENGL_RENDERSYSTEM_H
#define APRIL_OPENGL_RENDERSYSTEM_H

#ifdef __APPLE__
#include <TargetConditionals.h>
#endif
#if TARGET_OS_MAC && !TARGET_OS_IPHONE
#include <OpenGL/gl.h>
#elif TARGET_OS_IPHONE
#include <OpenGLES/ES1/gl.h>
#elif _OPENGLES1
#include <GLES/gl.h>
#else
#include <GL/gl.h>
#endif

#include "RenderSystem.h"

namespace april
{
	class OpenGL_RenderSystem : public RenderSystem
	{
	public:
		OpenGL_RenderSystem();
		~OpenGL_RenderSystem();

		void assignWindow(Window* window);
		
		// object creation
		Texture* loadTexture(chstr filename, bool dynamic);
		Texture* createTextureFromMemory(unsigned char* rgba, int w, int h);
		Texture* createEmptyTexture(int w, int h, TextureFormat fmt, TextureType type);


		// modelview matrix transformation
		void setBlendMode(BlendMode mode);
		void setColorMode(ColorMode mode);
		void setTextureFilter(TextureFilter filter);
		void setTextureWrapping(bool wrap);
		void setResolution(int w, int h);
		void setColorMode(ColorMode mode, unsigned char alpha);
		// caps
		float getPixelOffset();
		hstr getName();
        
        ImageSource* grabScreenshot(int bpp = 3);

		// rendering
		void clear(bool useColor = true, bool depth = false);
		void clear(bool useColor, bool depth, grect rect, Color color = APRIL_COLOR_CLEAR);
		void setTexture(Texture* t);
		void render(RenderOp renderOp, PlainVertex* v, int nVertices);
		void render(RenderOp renderOp, PlainVertex* v, int nVertices, Color color);
		void render(RenderOp renderOp, TexturedVertex* v, int nVertices);
		void render(RenderOp renderOp, TexturedVertex* v, int nVertices, Color color);
		void render(RenderOp renderOp, ColoredVertex* v, int nVertices);
		void render(RenderOp renderOp, ColoredTexturedVertex* v, int nVertices);
		
		Texture* getRenderTarget();
		void setRenderTarget(Texture* source);
		void beginFrame();

		harray<DisplayMode> getSupportedDisplayModes();
		
		static OpenGL_RenderSystem* create(chstr options);

	protected:
		bool mTexCoordsEnabled;
		bool mColorEnabled;
		void _setModelviewMatrix(const gmat4& matrix);
		void _setProjectionMatrix(const gmat4& matrix);
		
	};
	
}

#endif
#endif
