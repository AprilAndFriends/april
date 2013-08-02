/// @file
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
/// Defines a DirectX11 render system.

#ifdef _DIRECTX11
#ifndef APRIL_DIRECTX11_RENDER_SYSTEM_H
#define APRIL_DIRECTX11_RENDER_SYSTEM_H

#include <d3d11_1.h>
#include <windows.ui.xaml.media.dxinterop.h>

#include <gtypes/Matrix4.h>
#include <gtypes/Quaternion.h>
#include <gtypes/Rectangle.h>
#include <hltypes/harray.h>
#include <hltypes/hplatform.h>
#include <hltypes/hstring.h>

#include "RenderSystem.h"

using namespace Microsoft::WRL;
using namespace Windows::UI::Core;
using namespace Windows::UI::Xaml::Controls;

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

		enum ColorInputLayout
		{
			CIL_IGNORE = 0,
			CIL_MULTIPLY = 1,
			CIL_PER_VERTEX = 2,
			CIL_UNDEFINED = 0x7FFFFFFF
		};
	
		struct ConstantBuffer
		{
			gmat4 matrix;
			gquat color;
			gquat colorModeData;
		};

		DirectX11_RenderSystem();
		~DirectX11_RenderSystem();
		bool create(Options options);
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
		BlendMode activeTextureBlendMode;
		ColorMode activeTextureColorMode;
		unsigned char activeTextureColorModeAlpha;
		Texture::Filter activeTextureFilter;
		Texture::AddressMode activeTextureAddressMode;
		DirectX11_Texture* activeTexture;
		DirectX11_VertexShader* activeVertexShader;
		DirectX11_PixelShader* activePixelShader;
		DirectX11_Texture* renderTarget;
		harray<DisplayMode> supportedDisplayModes;

		DirectX11_VertexShader* vertexShaderDefault;
		DirectX11_PixelShader* pixelShaderDefault;

		void _configureDevice();
		void _createSwapChain(int width, int height);
		
		void _setResolution(int w, int h, bool fullscreen);

		Texture* _createTexture(chstr filename);
		Texture* _createTexture(int w, int h, unsigned char* rgba);
		Texture* _createTexture(int w, int h, Texture::Format format, Texture::Type type = Texture::TYPE_NORMAL, Color color = Color::Clear);

		void _setModelviewMatrix(const gmat4& matrix);
		void _setProjectionMatrix(const gmat4& matrix);
		void _setPixelShader(DirectX11_PixelShader* shader);
		void _setVertexShader(DirectX11_VertexShader* shader);

		void _render(RenderOp renderOp, PlainVertex* v, int nVertices, Color color, ColorInputLayout colorInputLayout);
		void _render(RenderOp renderOp, TexturedVertex* v, int nVertices, Color color, ColorInputLayout colorInputLayout);

	private:
		ComPtr<ID3D11Device1> d3dDevice;
		ComPtr<ID3D11DeviceContext1> d3dDeviceContext;
		ComPtr<IDXGISwapChain1> swapChain;
		ComPtr<ISwapChainBackgroundPanelNative>	swapChainNative;

		ComPtr<ID3D11RasterizerState> rasterState;
		ComPtr<ID3D11RenderTargetView> renderTargetView;
		ComPtr<ID3D11BlendState> blendStateAlpha;
		ComPtr<ID3D11BlendState> blendStateAdd;
		ComPtr<ID3D11BlendState> blendStateSubtract;
		ComPtr<ID3D11BlendState> blendStateOverwrite;
		ComPtr<ID3D11SamplerState> samplerLinearWrap;
		ComPtr<ID3D11SamplerState> samplerLinearClamp;
		ComPtr<ID3D11SamplerState> samplerNearestWrap;
		ComPtr<ID3D11SamplerState> samplerNearestClamp;

		D3D11_BUFFER_DESC vertexBufferDesc;
		D3D11_SUBRESOURCE_DATA vertexBufferData;
		D3D11_MAPPED_SUBRESOURCE mappedSubResource;
		ComPtr<ID3D11Buffer> vertexBuffer;

		ComPtr<ID3D11Buffer> constantBuffer;
		ConstantBuffer constantBufferData;

		ComPtr<ID3D11InputLayout> inputLayout;

		DirectX11_VertexShader* _currentVertexShader;
		DirectX11_PixelShader* _currentPixelShader;
		BlendMode _currentTextureBlendMode;
		Texture::Filter _currentTextureFilter;
		Texture::AddressMode _currentTextureAddressMode;
		RenderOp _currentRenderOp;
		ID3D11Buffer** _currentVertexBuffer;
		
		bool matrixDirty;

		void _setRenderOp(RenderOp renderOp);
		void _updateVertexBuffer(int nVertices, void* data);
		void _updateConstantBuffer(Color color, bool useTexture, ColorInputLayout colorInputLayout);
		void _updateBlendMode();
		void _updateTexture(bool use);

	};

}
#endif
#endif
