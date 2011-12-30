/// @file
/// @author  Kresimir Spes
/// @version 1.31
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/// 
/// @section DESCRIPTION
/// 
/// Defines an OpenGL render system.

#if defined(_OPENGL) || _OPENGLES1
#ifndef APRIL_OPENGL_RENDER_SYSTEM_H
#define APRIL_OPENGL_RENDER_SYSTEM_H

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
		hstr mParams;
	public:
		OpenGL_RenderSystem(hstr params);
		~OpenGL_RenderSystem();

		void assignWindow(Window* window);
		
		// object creation
		Texture* loadTexture(chstr filename, bool dynamic);
		Texture* createTextureFromMemory(unsigned char* rgba, int w, int h);
		Texture* createEmptyTexture(int w, int h, TextureFormat fmt, TextureType type);

		void setParam(chstr name, chstr value);
		
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
