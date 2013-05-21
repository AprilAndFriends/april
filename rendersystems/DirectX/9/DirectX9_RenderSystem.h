/// @file
/// @author  Kresimir Spes
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
/// Defines a DirectX render system.

#ifdef _DIRECTX9
#ifndef APRIL_DIRECTX9_RENDER_SYSTEM_H
#define APRIL_DIRECTX9_RENDER_SYSTEM_H

#include <hltypes/hplatform.h>

#include "DirectX_RenderSystem.h"

struct _D3DPRESENT_PARAMETERS_;
struct IDirect3D9;
struct IDirect3DDevice9;
struct IDirect3DSurface9;

namespace april
{
	class DirectX9_PixelShader;
	class DirectX9_Texture;
	class DirectX9_VertexShader;
	class Image;
	class Window;

	class DirectX9_RenderSystem : public DirectX_RenderSystem
	{
	public:
		friend class DirectX9_PixelShader;
		friend class DirectX9_Texture;
		friend class DirectX9_VertexShader;

		DirectX9_RenderSystem();
		~DirectX9_RenderSystem();
		bool create(Options options);
		bool destroy();

		void reset();
		void assignWindow(Window* window);

		float getPixelOffset() { return 0.5f; }
		harray<DisplayMode> getSupportedDisplayModes();
		grect getViewport();
		void setViewport(grect rect);

		void setTextureBlendMode(BlendMode textureBlendMode);
		void setTextureColorMode(ColorMode textureColorMode, unsigned char alpha = 255);
		void setTextureFilter(Texture::Filter textureFilter);
		void setTextureAddressMode(Texture::AddressMode textureAddressMode);
		void setTexture(Texture* texture);
		Texture* getRenderTarget();
		void setRenderTarget(Texture* source);
		void setPixelShader(PixelShader* pixelShader);
		void setVertexShader(VertexShader* vertexShader);

		PixelShader* createPixelShader();
		PixelShader* createPixelShader(chstr filename);
		VertexShader* createVertexShader();
		VertexShader* createVertexShader(chstr filename);

		void clear(bool useColor = true, bool depth = false);
		void clear(bool depth, grect rect, Color color = Color::Clear);
		void render(RenderOp renderOp, PlainVertex* v, int nVertices);
		void render(RenderOp renderOp, PlainVertex* v, int nVertices, Color color);
		void render(RenderOp renderOp, TexturedVertex* v, int nVertices);
		void render(RenderOp renderOp, TexturedVertex* v, int nVertices, Color color);
		void render(RenderOp renderOp, ColoredVertex* v, int nVertices);
		void render(RenderOp renderOp, ColoredTexturedVertex* v, int nVertices);

		Image* takeScreenshot(int bpp = 3);
		void presentFrame();

		// TODO - refactor
		int getMaxTextureSize();

	protected:
		bool textureCoordinatesEnabled;
		bool colorEnabled;
		IDirect3D9* d3d;
		IDirect3DDevice9* d3dDevice;
		DirectX9_Texture* activeTexture;
		DirectX9_Texture* renderTarget;
		IDirect3DSurface9* backBuffer;
		harray<DisplayMode> supportedDisplayModes;
		_D3DPRESENT_PARAMETERS_* d3dpp;

		void _configureDevice();
		void _setResolution(int w, int h, bool fullscreen);

		Texture* _createTexture(chstr filename);
		Texture* _createTexture(int w, int h, unsigned char* rgba);
		Texture* _createTexture(int w, int h, Texture::Format format, Texture::Type type = Texture::TYPE_NORMAL, Color color = Color::Clear);

		void _setModelviewMatrix(const gmat4& matrix);
		void _setProjectionMatrix(const gmat4& matrix);
		
	};

}
#endif
#endif
