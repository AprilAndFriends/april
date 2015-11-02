/// @file
/// @version 4.0
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

		class ShaderComposition
		{
		public:
			friend class DirectX11_RenderSystem;

			ShaderComposition(ComPtr<ID3D11InputLayout> inputLayout, DirectX11_VertexShader* vertexShader, DirectX11_PixelShader* pixelShader);
			~ShaderComposition();

		protected:
			ComPtr<ID3D11InputLayout> inputLayout;
			DirectX11_VertexShader* vertexShader;
			DirectX11_PixelShader* pixelShader;

		};

		struct ConstantBuffer
		{
			gmat4 matrix;
			gquat systemColor;
			gquat lerpAlpha; // must be, because of 16 byte alignment of constant buffer size
		};

		DirectX11_RenderSystem();
		~DirectX11_RenderSystem();

		float getPixelOffset();
		int getVRam();
		
		Image::Format getNativeTextureFormat(Image::Format format);
		unsigned int getNativeColorUInt(const april::Color& color);
		Image* takeScreenshot(Image::Format format);
		void presentFrame();

		void updateOrientation();

		void trim(); // needed by Win 8.1

		// TODOa - implement
		Texture* getRenderTarget();
		void setRenderTarget(Texture* source);
		void setPixelShader(PixelShader* pixelShader);
		void setVertexShader(VertexShader* vertexShader);

	protected:
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

		ComPtr<ID3D11InputLayout> inputLayoutPlain;
		ComPtr<ID3D11InputLayout> inputLayoutTextured;
		ComPtr<ID3D11InputLayout> inputLayoutColored;
		ComPtr<ID3D11InputLayout> inputLayoutColoredTextured;

		DirectX11_VertexShader* vertexShaderPlain;
		DirectX11_VertexShader* vertexShaderTextured;
		DirectX11_VertexShader* vertexShaderColored;
		DirectX11_VertexShader* vertexShaderColoredTextured;
		DirectX11_PixelShader* pixelShaderMultiply;
		DirectX11_PixelShader* pixelShaderLerp;
		DirectX11_PixelShader* pixelShaderAlphaMap;
		DirectX11_PixelShader* pixelShaderTexturedMultiply;
		DirectX11_PixelShader* pixelShaderTexturedLerp;
		DirectX11_PixelShader* pixelShaderTexturedAlphaMap;
		ShaderComposition* shaderMultiply;
		ShaderComposition* shaderLerp;
		ShaderComposition* shaderAlphaMap;
		ShaderComposition* shaderTexturedMultiply;
		ShaderComposition* shaderTexturedLerp;
		ShaderComposition* shaderTexturedAlphaMap;
		ShaderComposition* shaderColoredMultiply;
		ShaderComposition* shaderColoredLerp;
		ShaderComposition* shaderColoredAlphaMap;
		ShaderComposition* shaderColoredTexturedMultiply;
		ShaderComposition* shaderColoredTexturedLerp;
		ShaderComposition* shaderColoredTexturedAlphaMap;

		bool deviceState_constantBufferChanged;
		ShaderComposition* deviceState_shader;
		ComPtr<ID3D11SamplerState> deviceState_sampler;
		RenderOperation deviceState_renderOperation;

		void _deviceInit();
		bool _deviceCreate(Options options);
		bool _deviceDestroy();
		void _deviceAssignWindow(Window* window);
		void _deviceReset();
		void _deviceSetupCaps();
		void _deviceSetup();

		void _createSwapChain(int width, int height);
		void _resizeSwapChain(int width, int height);
		void _configureSwapChain();
		void _configureDevice();
		
		Texture* _deviceCreateTexture(bool fromResource);
		PixelShader* _deviceCreatePixelShader();
		VertexShader* _deviceCreateVertexShader();

		void _deviceChangeResolution(int w, int h, bool fullscreen);

		void _setDeviceViewport(const grect& rect);
		void _setDeviceModelviewMatrix(const gmat4& matrix);
		void _setDeviceProjectionMatrix(const gmat4& matrix);
		void _setDeviceDepthBuffer(bool enabled, bool writeEnabled);
		void _setDeviceRenderMode(bool useTexture, bool useColor);
		void _setDeviceTexture(Texture* texture);
		void _setDeviceTextureFilter(Texture::Filter textureFilter);
		void _setDeviceTextureAddressMode(Texture::AddressMode textureAddressMode);
		void _setDeviceBlendMode(BlendMode blendMode);
		void _setDeviceColorMode(ColorMode colorMode, float colorModeFactor, bool useTexture, bool useColor, const Color& systemColor);

		void _updateDeviceState(bool forceUpdate);
		void _updateShader(bool forceUpdate);

		void _deviceClear(bool depth);
		void _deviceClear(april::Color color, bool depth);
		void _deviceClearDepth();
		void _deviceRender(RenderOperation renderOperation, PlainVertex* v, int nVertices);
		void _deviceRender(RenderOperation renderOperation, TexturedVertex* v, int nVertices);
		void _deviceRender(RenderOperation renderOperation, ColoredVertex* v, int nVertices);
		void _deviceRender(RenderOperation renderOperation, ColoredTexturedVertex* v, int nVertices);

		void _setDX11VertexBuffer(RenderOperation renderOperation, void* data, int nVertices, unsigned int vertexSize);

		static D3D11_PRIMITIVE_TOPOLOGY _dx11RenderOperations[];

	};

}
#endif
#endif
