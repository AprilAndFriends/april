/************************************************************************************\
This source file is part of the Awesome Portable Rendering Interface Library         *
For latest info, see http://libapril.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes (kreso@cateia.com)                                  *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#ifndef RENDERSYSTEM_GL_H
#ifdef _OPENGL
#define RENDERSYSTEM_GL_H

#include "RenderSystem.h"
#include "Timer.h"

#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

namespace April
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
		GLRenderSystem(int w,int h);
		~GLRenderSystem();
		
		// object creation
		Texture* loadTexture(chstr filename,bool dynamic);
		Texture* createTextureFromMemory(unsigned char* rgba,int w,int h);
		Texture* createEmptyTexture(int w,int h,TextureFormat fmt,TextureType type);


		// modelview matrix transformation
		void setBlendMode(BlendMode mode);
		void setTextureFilter(TextureFilter filter);
		// caps
		float getPixelOffset();
		hstr getName();

		// rendering
		void clear(bool color,bool depth);
		void setTexture(Texture* t);
		void render(RenderOp renderOp,TexturedVertex* v,int nVertices);
		void render(RenderOp renderOp,ColoredTexturedVertex* v,int nVertices);
		void render(RenderOp renderOp,TexturedVertex* v,int nVertices,float r,float g,float b,float a);
		void render(RenderOp renderOp,PlainVertex* v,int nVertices);
		void render(RenderOp renderOp,PlainVertex* v,int nVertices,float r,float g,float b,float a);
		void render(RenderOp renderOp,ColoredVertex* v,int nVertices);

		void setAlphaMultiplier(float value);
		void setRenderTarget(Texture* source);
		void beginFrame();
		
		void setWindowTitle(chstr title);
		gtypes::Vector2 getCursorPos();
		void showSystemCursor(bool b);
		bool isSystemCursorShown();

		int getWindowWidth();
		int getWindowHeight();
		
		void presentFrame();
		
		bool triggerUpdate(float time_increase);
		bool triggerKeyEvent(bool down,unsigned int keycode);
		void triggerQuitEvent();
		bool triggerMouseEvent(int event,float x,float y,int button);
		
		void enterMainLoop();
		void terminateMainLoop();
	};

	void createGLRenderSystem(int w,int h,bool fullscreen,chstr title);
}
#endif
#endif
