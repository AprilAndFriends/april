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
/// Defines a generic OpenGL render system.

#ifdef _OPENGL
#ifndef APRIL_OPENGL_RENDER_SYSTEM_H
#define APRIL_OPENGL_RENDER_SYSTEM_H

#include <hltypes/hplatform.h>
#include <hltypes/hstring.h>

#include "OpenGL_State.h"
#include "RenderSystem.h"

namespace april
{
	class OpenGL_Texture;
	class Window;

	class OpenGL_RenderSystem : public RenderSystem
	{
	public:
		OpenGL_RenderSystem();
		~OpenGL_RenderSystem();
		bool create(chstr options);
		bool destroy();

		void reset();
		void assignWindow(Window* window);

		float getPixelOffset() { return 0.0f; }
		int getMaxTextureSize();
		grect getViewport();
		void setViewport(grect value);
		harray<DisplayMode> getSupportedDisplayModes();

		void clear(bool useColor = true, bool depth = false);
		void clear(bool depth, grect rect, Color color = Color::Clear);

		void bindTexture(unsigned int textureId);
		void setResolution(int w, int h);

		void setMatrixMode(unsigned int mode);
		void setTexture(Texture* texture);
		Texture* getRenderTarget();
		void setRenderTarget(Texture* texture);
		void setPixelShader(PixelShader* pixelShader);
		void setVertexShader(VertexShader* vertexShader);
		void setTextureBlendMode(BlendMode mode);
		void setTextureColorMode(ColorMode textureColorMode, unsigned char alpha = 255);
		void setTextureFilter(Texture::Filter textureFilter);
		void setTextureAddressMode(Texture::AddressMode textureAddressMode);

		PixelShader* createPixelShader();
		PixelShader* createPixelShader(chstr filename);
		VertexShader* createVertexShader();
		VertexShader* createVertexShader(chstr filename);

		void render(RenderOp renderOp, PlainVertex* v, int nVertices);
		void render(RenderOp renderOp, PlainVertex* v, int nVertices, Color color);
		void render(RenderOp renderOp, TexturedVertex* v, int nVertices);
		void render(RenderOp renderOp, TexturedVertex* v, int nVertices, Color color);
		void render(RenderOp renderOp, ColoredVertex* v, int nVertices);
		void render(RenderOp renderOp, ColoredTexturedVertex* v, int nVertices);
		
		void setParam(chstr name, chstr value);
		Image* takeScreenshot(int bpp = 3);

	protected:
		OpenGL_State deviceState;
		OpenGL_State state;
		hstr options;
		OpenGL_Texture* activeTexture;

		virtual void _setupDefaultParameters();
		virtual void _applyStateChanges();
		void _setClientState(unsigned int type, bool enabled);
		void _setModelviewMatrix(const gmat4& matrix);
		void _setProjectionMatrix(const gmat4& matrix);

		virtual void _setTextureBlendMode(BlendMode textureBlendMode);
		virtual void _setTextureColorMode(ColorMode mode, unsigned char alpha = 255);
		virtual void _setTextureFilter(Texture::Filter textureFilter);
		virtual void _setTextureAddressMode(Texture::AddressMode textureAddressMode);

		virtual void _setVertexPointer(int stride, const void* pointer) = 0;
		virtual void _setTexCoordPointer(int stride, const void* pointer);
		virtual void _setColorPointer(int stride, const void* pointer);

#ifdef _WIN32
		HWND hWnd;
		HDC hDC;

		bool _initWin32(Window* window);
		virtual void _releaseWindow();
	public:
		HDC getHDC() { return this->hDC; }

#endif

	};
	
}
#endif
#endif
