/************************************************************************************\
This source file is part of the Awesome Portable Rendering Interface Library         *
For latest info, see http://libapril.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes                                                     *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#ifndef APRIL_ANDROID_GLES_RENDERSYSTEM_H
#define APRIL_ANDROID_GLES_RENDERSYSTEM_H

#include <android/log.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include "RenderSystem.h"
#include "gltypes/Matrix4.h"

namespace april
{
	class AndroidGLES_RenderSystem : public RenderSystem
	{
	public:
		AndroidGLES_RenderSystem(Window* window);
		~AndroidGLES_RenderSystem();
		
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
        
        ImageSource* grabScreenshot(int bpp = 3);

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
		
		GLuint loadShader(GLenum shaderType, const char *pSource);
		GLuint createProgram(const char *vertexSource, const char *fragmentSource);
		void useProgram(const char *programName); 
		
	protected:
	
		gmat4 mModelViewMatrix; // we are using shaders, this is nececsary because gles2 has no software storage for modelview matrix
		gmat4 mProjectionMatrix; // we are using shaders, this is nececsary because gles2 has no software storage for projection matrix
		
		hmap<hstr, GLuint> mShaders; // map of all shaders available
		GLuint mFallbackShader; // generic fallback shader in case some of the shaders fail to compile and/or load
	
		bool mTexCoordsEnabled;
		bool mColorEnabled;
		void _setModelviewMatrix(const gmat4& matrix);
		void _setProjectionMatrix(const gmat4& matrix);
		
	};

	void createOpenGL_RenderSystem(Window* window);
	
}

#endif
#endif
