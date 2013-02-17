/// @file
/// @author  Kresimir Spes
/// @author  Ivan Vucica
/// @author  Boris Mikic
/// @version 3.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/// 
/// @section DESCRIPTION
/// 
/// Defines an OpenGLES1 render system.

#ifdef _OPENGLES1
#ifndef APRIL_OPENGLES1_RENDER_SYSTEM_H
#define APRIL_OPENGLES1_RENDER_SYSTEM_H

#ifdef __APPLE__
#include <TargetConditionals.h>
#endif

#include "OpenGL_State.h"
#include "OpenGLES_RenderSystem.h"

namespace april
{
	class Image;
	class OpenGLES1_Texture;
	class Window;

	class OpenGLES1_RenderSystem : public OpenGLES_RenderSystem
	{
	public:
		friend class OpenGLES1_Texture;

		OpenGLES1_RenderSystem();
		~OpenGLES1_RenderSystem();
		bool create(chstr options);
		bool destroy();

		void assignWindow(Window* window);
		
		float getPixelOffset() { return 0.0f; }
		grect getViewport();
		void setViewport(grect value);
		harray<DisplayMode> getSupportedDisplayModes();

		void setTextureBlendMode(BlendMode textureBlendMode);

		Texture* createTexture(int w, int h, unsigned char* rgba);
		Texture* createTexture(int w, int h, Texture::Format format, Texture::Type type = Texture::TYPE_NORMAL, Color color = Color::Clear);

		void render(RenderOp renderOp, PlainVertex* v, int nVertices);
		void render(RenderOp renderOp, PlainVertex* v, int nVertices, Color color);
		void render(RenderOp renderOp, TexturedVertex* v, int nVertices);
		void render(RenderOp renderOp, TexturedVertex* v, int nVertices, Color color);
		void render(RenderOp renderOp, ColoredVertex* v, int nVertices);
		void render(RenderOp renderOp, ColoredTexturedVertex* v, int nVertices);
		
		void setParam(chstr name, chstr value);
		Image* takeScreenshot(int bpp = 3);

		// TODO - refactor
		int _getMaxTextureSize();

	protected:
		void _setupDefaultParameters();
		Texture* _createTexture(chstr filename);

		void _setVertexPointer(int stride, const void* pointer);
		void _setTexCoordPointer(int stride, const void* pointer);
		void _setColorPointer(int stride, const void* pointer);
		void _setTextureBlendMode(BlendMode mode);
		void _setModelviewMatrix(const gmat4& matrix);
		void _setProjectionMatrix(const gmat4& matrix);

#ifdef _WIN32
		void _releaseWindow();
#endif
		
	};
	
}

#endif
#endif
