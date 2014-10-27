/// @file
/// @version 3.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Defines a DirectX11 render system.

#ifdef _DIRECTX11
#ifndef APRIL_DIRECTX11_RENDER_SYSTEM_H
#define APRIL_DIRECTX11_RENDER_SYSTEM_H

#include <windows.ui.xaml.media.dxinterop.h>
using namespace Windows::UI::Xaml::Controls;

#include <d3d11_2.h>

#include <gtypes/Matrix4.h>
#include <gtypes/Quaternion.h>
#include <gtypes/Rectangle.h>
#include <hltypes/harray.h>
#include <hltypes/hplatform.h>
#include <hltypes/hstring.h>

#include "DirectX_RenderSystem.h"
#include "Window.h"

using namespace Microsoft::WRL;
using namespace Windows::UI::Core;

namespace april
{
	class DirectX11_PixelShader;
	class DirectX11_Texture;
	class DirectX11_VertexShader;
	class Window;

	class DirectX11_RenderSystem : public DirectX_RenderSystem
	{
	public:
		friend class DirectX11_PixelShader;
		friend class DirectX11_Texture;
		friend class DirectX11_VertexShader;

		struct ConstantBuffer
		{
			gmat4 matrix;
			gquat lerpAlpha; // must be, because of 16 byte alignment of constant buffer size
		};

		DirectX11_RenderSystem();
		~DirectX11_RenderSystem();
		bool create(Options options);
		bool destroy();

		void assignWindow(Window* window);
		void reset();

		inline float getPixelOffset() { return 0.0f; }
		int getVRam();
		void setViewport(grect value);

		void setTextureBlendMode(BlendMode textureBlendMode);
		void setTextureColorMode(ColorMode colorMode, float factor = 1.0f);
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
		void render(RenderOperation renderOperation, PlainVertex* v, int nVertices);
		void render(RenderOperation renderOperation, PlainVertex* v, int nVertices, Color color);
		void render(RenderOperation renderOperation, TexturedVertex* v, int nVertices);
		void render(RenderOperation renderOperation, TexturedVertex* v, int nVertices, Color color);
		void render(RenderOperation renderOperation, ColoredVertex* v, int nVertices);
		void render(RenderOperation renderOperation, ColoredTexturedVertex* v, int nVertices);

		Image::Format getNativeTextureFormat(Image::Format format);
		unsigned int getNativeColorUInt(const april::Color& color);
		Image* takeScreenshot(Image::Format format);
		void presentFrame();

		void updateOrientation();

		void trim(); // needed by Win 8.1

	protected:
		BlendMode activeTextureBlendMode;
		ColorMode activeTextureColorMode;
		unsigned char activeTextureColorModeAlpha;
		DirectX11_Texture* activeTexture;
		DirectX11_VertexShader* activeVertexShader;
		DirectX11_PixelShader* activePixelShader;
		DirectX11_Texture* renderTarget;
		harray<DisplayMode> supportedDisplayModes;
		grect viewport;

		DirectX11_VertexShader* vertexShaderDefault;
		DirectX11_PixelShader* pixelShaderTexturedMultiply;
		DirectX11_PixelShader* pixelShaderTexturedAlphaMap;
		DirectX11_PixelShader* pixelShaderTexturedLerp;
		DirectX11_PixelShader* pixelShaderMultiply;
		DirectX11_PixelShader* pixelShaderAlphaMap;
		DirectX11_PixelShader* pixelShaderLerp;

		void _createSwapChain(int width, int height);
		void _resizeSwapChain(int width, int height);
		void _configureSwapChain();
		void _configureDevice();
		
		void _setupCaps();
		void _setResolution(int w, int h, bool fullscreen);

		Texture* _createTexture(bool fromResource);

		void _setModelviewMatrix(const gmat4& matrix);
		void _setProjectionMatrix(const gmat4& matrix);
		void _updatePixelShader(bool useTexture);
		void _updateVertexShader();

	private:
		ComPtr<ID3D11Device2> d3dDevice;
		ComPtr<ID3D11DeviceContext2> d3dDeviceContext;
		ComPtr<IDXGISwapChain2> swapChain;
		ComPtr<ISwapChainPanelNative> swapChainNative;

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
		DirectX11_Texture* _currentTexture;
		BlendMode _currentTextureBlendMode;
		ColorMode _currentTextureColorMode;
		Texture::Filter _currentTextureFilter;
		Texture::AddressMode _currentTextureAddressMode;
		RenderOperation _currentRenderOperation;
		ID3D11Buffer** _currentVertexBuffer;
		
		bool matrixDirty;
		
		void _setRenderOperation(RenderOperation renderOperation);
		void _updateVertexBuffer(int nVertices, void* data);
		void _updateConstantBuffer();
		void _updateBlendMode();
		void _updateTexture(bool use);

	};

}
#endif
#endif
