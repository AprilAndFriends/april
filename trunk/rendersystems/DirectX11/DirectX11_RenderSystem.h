/// @file
/// @author  Boris Mikic
/// @version 2.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/// 
/// @section DESCRIPTION
/// 
/// Defines a DirectX11 render system.

#ifdef _DIRECTX11
#ifndef APRIL_DIRECTX11_RENDER_SYSTEM_H
#define APRIL_DIRECTX11_RENDER_SYSTEM_H

#include <d3d11_1.h>

#include <hltypes/hplatform.h>

#include "RenderSystem.h"

using namespace Microsoft::WRL;
using namespace Windows::UI::Core;

namespace april
{
	class DirectX11_PixelShader;
	class DirectX11_Texture;
	class DirectX11_VertexShader;
	class Window;

	class DirectX11_RenderSystem : public RenderSystem
	{
	public:
		friend class DirectX11_PixelShader;
		friend class DirectX11_Texture;
		friend class DirectX11_VertexShader;

		DirectX11_RenderSystem();
		~DirectX11_RenderSystem();
		bool create(chstr options);
		bool destroy();

		void assignWindow(Window* window);
		//void reset();

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

		void setResolution(int w, int h);

		Texture* createTexture(int w, int h, unsigned char* rgba);
		Texture* createTexture(int w, int h, Texture::Format format, Texture::Type type = Texture::TYPE_NORMAL, Color color = Color::Clear);
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

		ImageSource* takeScreenshot(int bpp = 3);
		void presentFrame();

		// TODO - refactor
		int _getMaxTextureSize();

	protected:
		bool zBufferEnabled;
		bool textureCoordinatesEnabled;
		bool colorEnabled;
		DirectX11_Texture* activeTexture;
		DirectX11_VertexShader* activeVertexShader;
		DirectX11_PixelShader* activePixelShader;
		DirectX11_Texture* renderTarget;
		harray<DisplayMode> supportedDisplayModes;
		VertexShader* defaultVertexShader;
		PixelShader* defaultPixelShader;

		void _configureDevice();
		void _createSwapChain(int width, int height);
		
		Texture* _createTexture(chstr filename);

		void _setModelviewMatrix(const gmat4& matrix);
		void _setProjectionMatrix(const gmat4& matrix);
		void _setPixelShader(DirectX11_PixelShader* shader);
		void _setVertexShader(DirectX11_VertexShader* shader);

	private:
		ComPtr<ID3D11Device1> d3dDevice;
		ComPtr<ID3D11DeviceContext1> d3dDeviceContext;
		ComPtr<IDXGISwapChain1> swapChain;
		ComPtr<ID3D11RenderTargetView> renderTargetView;
		D3D11_BUFFER_DESC vertexBufferDescription;
		D3D11_SUBRESOURCE_DATA vertexBufferData;
		harray<unsigned short> genericIndices;
		harray<ComPtr<ID3D11Buffer> > vertexBuffers;
		ComPtr<ID3D11Buffer> indexBuffer;

	};

}
#endif
#endif
