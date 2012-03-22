/// @file
/// @author  Kresimir Spes
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
/// Defines a DirectX render system.

#ifdef _DIRECTX9
#ifndef APRIL_DIRECTX9_RENDER_SYSTEM_H
#define APRIL_DIRECTX9_RENDER_SYSTEM_H

#include "RenderSystem.h"

struct IDirect3D9;
struct IDirect3DDevice9;
struct IDirect3DSurface9;

namespace april
{
	class DirectX9_PixelShader;
	class DirectX9_Texture;
	class DirectX9_VertexShader;
	class Window;

	class DirectX9_RenderSystem : public RenderSystem
	{
	public:
		friend class DirectX9_PixelShader;
		friend class DirectX9_Texture;
		friend class DirectX9_VertexShader;

		DirectX9_RenderSystem();
		~DirectX9_RenderSystem();
		bool create(chstr options);
		bool destroy();

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
		grect getViewport();
		void setViewport(grect rect);

		void setBlendMode(BlendMode mode);
		void setColorMode(ColorMode mode, unsigned char alpha = 255);
		void setTextureFilter(TextureFilter filter);
		void setTextureWrapping(bool wrap);
		void setResolution(int w, int h);
		// caps
		float getPixelOffset() { return 0.5f; }
		hstr getName();
		// rendering
		void clear(bool useColor = true, bool depth = false);
		void clear(bool useColor, bool depth, grect rect, Color color = APRIL_COLOR_CLEAR);
		ImageSource* grabScreenshot(int bpp = 3);
		void setTexture(Texture* texture);
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

	protected:
		bool zBufferEnabled;
		bool textureCoordinatesEnabled;
		bool colorEnabled;
		IDirect3D9* d3d;
		IDirect3DDevice9* d3dDevice;
		DirectX9_Texture* activeTexture;
		DirectX9_Texture* renderTarget;
		IDirect3DSurface9* backBuffer;

		void _setModelviewMatrix(const gmat4& matrix);
		void _setProjectionMatrix(const gmat4& matrix);
		
	};

}
#endif
#endif
