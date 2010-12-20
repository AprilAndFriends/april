/************************************************************************************\
This source file is part of the Awesome Portable Rendering Interface Library         *
For latest info, see http://libapril.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes                                                     *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#ifndef RENDERSYSTEM_GL_H
#ifdef _OPENGL
#define RENDERSYSTEM_GL_H

#include "RenderSystem.h"

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

namespace april
{
	class GLTexture : public Texture
	{
	public:
		GLuint mTexId;
		
		GLTexture(chstr filename,bool dynamic);
		GLTexture(unsigned char* rgba,int w,int h);
		~GLTexture();
		
		bool load();
		void unload();
		bool isLoaded();
		int getSizeInBytes();
	};

	class GLRenderSystem : public RenderSystem
	{
		bool mTexCoordsEnabled,mColorEnabled;
		void _setModelviewMatrix(const gtypes::Matrix4& matrix);
		void _setProjectionMatrix(const gtypes::Matrix4& matrix);
	public:
		GLRenderSystem(Window* window);
		~GLRenderSystem();
		
		// object creation
		Texture* loadTexture(chstr filename,bool dynamic);
		Texture* createTextureFromMemory(unsigned char* rgba,int w,int h);
		Texture* createEmptyTexture(int w,int h,TextureFormat fmt,TextureType type);


		// modelview matrix transformation
		void setBlendMode(BlendMode mode);
		void setTextureFilter(TextureFilter filter);
		void setTextureWrapping(bool wrap);
		// caps
		float getPixelOffset();
		hstr getName();
        
        ImageSource* grabScreenshot();

		// rendering
		void clear(bool color,bool depth);
		void setTexture(Texture* t);
		void render(RenderOp renderOp,TexturedVertex* v,int nVertices);
		void render(RenderOp renderOp,ColoredTexturedVertex* v,int nVertices);
		void render(RenderOp renderOp,TexturedVertex* v,int nVertices,Color color);
		void render(RenderOp renderOp,PlainVertex* v,int nVertices);
		void render(RenderOp renderOp,PlainVertex* v,int nVertices,Color color);
		void render(RenderOp renderOp,ColoredVertex* v,int nVertices);

		void setAlphaMultiplier(float value);
		void setRenderTarget(Texture* source);
		void beginFrame();
		
	};

	void createGLRenderSystem(Window* window);
}
#endif
#endif
