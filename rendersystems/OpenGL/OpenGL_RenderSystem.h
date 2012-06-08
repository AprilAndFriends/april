/// @file
/// @author  Kresimir Spes
/// @author  Ivan Vucica
/// @author  Boris Mikic
/// @version 1.86
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
	class OpenGL_RenderState
	{
	public:
		OpenGL_RenderState();

		bool texCoordsEnabled;
		bool colorEnabled;
		unsigned int texId;
		Color systemColor;
		bool modelviewMatrixSet, projectionMatrixSet;
		gmat4 modelviewMatrix, projectionMatrix;
		BlendMode blendMode;
		ColorMode colorMode;
		unsigned char colorModeAlpha;
	};
	
	class OpenGL_RenderSystem : public RenderSystem
	{
	public:
		OpenGL_RenderSystem(hstr params);
		~OpenGL_RenderSystem();

		void assignWindow(Window* window);
		void restore();
		
		// object creation
		Texture* loadTexture(chstr filename, bool dynamic);
		Texture* createTextureFromMemory(unsigned char* rgba, int w, int h);
		Texture* createEmptyTexture(int w, int h, TextureFormat fmt, TextureType type = AT_NORMAL);

		VertexShader* createVertexShader();
		PixelShader* createPixelShader();
		void setVertexShader(VertexShader* vertexShader);
		void setPixelShader(PixelShader* pixelShader);
		grect getViewport();
		void setViewport(grect rect);
		
		void setParam(chstr name, chstr value);
		
		void _setBlendMode(BlendMode mode);
		void setBlendMode(BlendMode mode);
		void setTextureFilter(TextureFilter filter);
		void setTextureWrapping(bool wrap);
		void setResolution(int w, int h);
		void _setColorMode(ColorMode mode, unsigned char alpha);
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

		void setMatrixMode(unsigned int mode);
		void bindTexture(unsigned int tex_id);
		void applyStateChanges();
	protected:
		hstr mParams;
		OpenGL_RenderState mState, mDeviceState;

		void _setVertexPointer(int stride, const void *pointer);
		void _setTexCoordPointer(int stride, const void *pointer);
		void _setColorPointer(int stride, const void *pointer);
		void _setModelviewMatrix(const gmat4& matrix);
		void _setProjectionMatrix(const gmat4& matrix);
#ifdef _WIN32
		void _releaseWindow();
#endif
		
	};
	
}

#endif
#endif
