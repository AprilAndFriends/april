/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @version 1.31
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/// 
/// @section DESCRIPTION
/// 
/// Defines a DirectX render system.

#ifdef _DIRECTX9
#ifndef APRIL_DIRECTX9_RENDER_SYSTEM_H
#define APRIL_DIRECTX9_RENDER_SYSTEM_H

#include "RenderSystem.h"

struct IDirect3DSurface9;

namespace april
{
	class DirectX9_Texture;
	class Window;

	class DirectX9_RenderSystem : public RenderSystem
	{
	public:
		DirectX9_RenderSystem(chstr options);
		~DirectX9_RenderSystem();

		void assignWindow(Window* window);
		
		void configureDevice();
		
		// object creation
		Texture* loadTexture(chstr filename,bool dynamic);
		Texture* createTextureFromMemory(unsigned char* rgba, int w, int h);
		Texture* createEmptyTexture(int w, int h, TextureFormat fmt, TextureType type);

		VertexShader* createVertexShader();
		PixelShader* createPixelShader();
		void setVertexShader(VertexShader* vertexShader);
		void setPixelShader(PixelShader* pixelShader);

		void setBlendMode(BlendMode mode);
		void setColorMode(ColorMode mode, unsigned char alpha = 255);
		void setTextureFilter(TextureFilter filter);
		void setTextureWrapping(bool wrap);
		void setResolution(int w, int h);
		// caps
		float getPixelOffset();
		hstr getName();
		// rendering
		void clear(bool useColor = true, bool depth = false);
		void clear(bool useColor, bool depth, grect rect, Color color = APRIL_COLOR_CLEAR);
		ImageSource* grabScreenshot(int bpp = 3);
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
		
		void presentFrame();
		harray<DisplayMode> getSupportedDisplayModes();

		static DirectX9_RenderSystem* create(chstr options);
		
	protected:
		bool mZBufferEnabled;
		bool mTexCoordsEnabled;
		bool mColorEnabled;
		hstr mTitle;
		DirectX9_Texture* mRenderTarget;
		IDirect3DSurface9* mBackBuffer;

		void _setModelviewMatrix(const gmat4& matrix);
		void _setProjectionMatrix(const gmat4& matrix);
		
	};

}
#endif
#endif
