/************************************************************************************\
This source file is part of the Awesome Portable Rendering Interface Library         *
For latest info, see http://libapril.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes                                                     *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#ifdef _OPENGL
#ifndef APRIL_OPENGL_RENDERSYSTEM_H
#define APRIL_OPNEGL_RENDERSYSTEM_H

#ifdef __APPLE__
#include <TargetConditionals.h>
#endif
#if (TARGET_OS_MAC) && !(TARGET_OS_IPHONE)
#include <OpenGL/gl.h>
#elif (TARGET_OS_IPHONE)
#include <OpenGLES/ES1/gl.h>
#else
#include <GL/gl.h>
#endif

#include "RenderSystem.h"

namespace april
{
	class OpenGL_RenderSystem : public RenderSystem
	{
	public:
		OpenGL_RenderSystem(Window* window);
		~OpenGL_RenderSystem();
		
		// object creation
		Texture* loadTexture(chstr filename, bool dynamic);
		Texture* createTextureFromMemory(unsigned char* rgba, int w, int h);
		Texture* createEmptyTexture(int w, int h, TextureFormat fmt, TextureType type);


		// modelview matrix transformation
		void setBlendMode(BlendMode mode);
		void setTextureFilter(TextureFilter filter);
		void setTextureWrapping(bool wrap);
		// caps
		float getPixelOffset();
		hstr getName();
        
        ImageSource* grabScreenshot();

		// rendering
		void clear(bool color, bool depth);
		void setTexture(Texture* t);
		void render(RenderOp renderOp, TexturedVertex* v, int nVertices);
		void render(RenderOp renderOp, ColoredTexturedVertex* v, int nVertices);
		void render(RenderOp renderOp, TexturedVertex* v, int nVertices, Color color);
		void render(RenderOp renderOp, PlainVertex* v, int nVertices);
		void render(RenderOp renderOp, PlainVertex* v, int nVertices, Color color);
		void render(RenderOp renderOp, ColoredVertex* v, int nVertices);

		void setAlphaMultiplier(float value);
		void setRenderTarget(Texture* source);
		void beginFrame();
		
	protected:
		bool mTexCoordsEnabled;
		bool mColorEnabled;
		void _setModelviewMatrix(const gmat4& matrix);
		void _setProjectionMatrix(const gmat4& matrix);
		
	};

	void createOpenGL_RenderSystem(Window* window);
	
}

#endif
#endif