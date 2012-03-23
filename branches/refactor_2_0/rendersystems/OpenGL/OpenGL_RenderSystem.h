/// @file
/// @author  Kresimir Spes
/// @author  Ivan Vucica
/// @author  Boris Mikic
/// @version 2.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/// 
/// @section DESCRIPTION
/// 
/// Defines an OpenGL render system.

#if defined(_OPENGL) || defined(_OPENGLES1)
#ifndef APRIL_OPENGL_RENDER_SYSTEM_H
#define APRIL_OPENGL_RENDER_SYSTEM_H

#ifdef __APPLE__
#include <TargetConditionals.h>
#endif

#include "RenderSystem.h"

namespace april
{
	class OpenGL_RenderSystem : public RenderSystem
	{
	public:
		OpenGL_RenderSystem();
		~OpenGL_RenderSystem();
		bool create(chstr options);
		bool destroy();

		void assignWindow(Window* window);
		void reset();
		
		// object creation
		Texture* loadTexture(chstr filename, bool dynamic);
		Texture* createTextureFromMemory(unsigned char* rgba, int w, int h);
		Texture* createEmptyTexture(int w, int h, TextureFormat fmt, TextureType type);

		VertexShader* createVertexShader();
		PixelShader* createPixelShader();
		void setVertexShader(VertexShader* vertexShader);
		void setPixelShader(PixelShader* pixelShader);
		grect getViewport();
		void setViewport(grect rect);
		
		void setParam(chstr name, chstr value);
		
		// modelview matrix transformation
		void setBlendMode(BlendMode mode);
		void setColorMode(ColorMode mode);
		void setTextureFilter(TextureFilter filter);
		void setTextureWrapping(bool wrap);
		void setResolution(int w, int h);
		void setColorMode(ColorMode mode, unsigned char alpha);
		// caps
		float getPixelOffset() { return 0.0f; }
        
        ImageSource* takeScreenshot(int bpp = 3);

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
		
	protected:
		hstr options;
		bool textureCoordinatesEnabled;
		bool colorEnabled;

		void _setModelviewMatrix(const gmat4& matrix);
		void _setProjectionMatrix(const gmat4& matrix);
#ifdef _WIN32
		void _releaseWindow();
#endif
		
	};
	
}

#endif
#endif
