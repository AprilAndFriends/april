/// @file
/// @version 5.2
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

#include <d3d11_4.h>

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

		int getVRam() const;
		
		Image::Format getNativeTextureFormat(Image::Format format) const;
		unsigned int getNativeColorUInt(const april::Color& color) const;
		Image* takeScreenshot(Image::Format format);

		void updateOrientation();

		// TODOa - implement
		Texture* getRenderTarget();
		void setRenderTarget(Texture* source);
		void setPixelShader(PixelShader* pixelShader);
		void setVertexShader(VertexShader* vertexShader);

	protected:
		ComPtr<IDXGIFactory4> dxgiFactory;
		ComPtr<ID3D11Device3> d3dDevice;
		ComPtr<ID3D11DeviceContext2> d3dDeviceContext;
		ComPtr<IDXGISwapChain3> swapChain;

		ComPtr<ID3D11RasterizerState> rasterState;
		ComPtr<ID3D11Texture2D> renderTarget;
		ComPtr<ID3D11RenderTargetView> renderTargetView;
		ComPtr<ID3D11Texture2D> depthBuffer;
		ComPtr<ID3D11DepthStencilView> depthBufferView;

		ComPtr<ID3D11BlendState> blendStateAlpha;
		ComPtr<ID3D11BlendState> blendStateAdd;
		ComPtr<ID3D11BlendState> blendStateSubtract;
		ComPtr<ID3D11BlendState> blendStateOverwrite;
		ComPtr<ID3D11SamplerState> samplerLinearWrap;
		ComPtr<ID3D11SamplerState> samplerLinearClamp;
		ComPtr<ID3D11SamplerState> samplerNearestWrap;
		ComPtr<ID3D11SamplerState> samplerNearestClamp;
		ComPtr<ID3D11DepthStencilState> depthState;

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
		DirectX11_PixelShader* pixelShaderAlphaMap;
		DirectX11_PixelShader* pixelShaderLerp;
		DirectX11_PixelShader* pixelShaderDesaturate;
		DirectX11_PixelShader* pixelShaderSepia;
		DirectX11_PixelShader* pixelShaderTexturedMultiply;
		DirectX11_PixelShader* pixelShaderTexturedAlphaMap;
		DirectX11_PixelShader* pixelShaderTexturedLerp;
		DirectX11_PixelShader* pixelShaderTexturedDesaturate;
		DirectX11_PixelShader* pixelShaderTexturedSepia;
		ShaderComposition* shaderMultiply;
		ShaderComposition* shaderAlphaMap;
		ShaderComposition* shaderLerp;
		ShaderComposition* shaderDesaturate;
		ShaderComposition* shaderSepia;
		ShaderComposition* shaderTexturedMultiply;
		ShaderComposition* shaderTexturedAlphaMap;
		ShaderComposition* shaderTexturedLerp;
		ShaderComposition* shaderTexturedDesaturate;
		ShaderComposition* shaderTexturedSepia;
		ShaderComposition* shaderColoredMultiply;
		ShaderComposition* shaderColoredAlphaMap;
		ShaderComposition* shaderColoredLerp;
		ShaderComposition* shaderColoredDesaturate;
		ShaderComposition* shaderColoredSepia;
		ShaderComposition* shaderColoredTexturedMultiply;
		ShaderComposition* shaderColoredTexturedAlphaMap;
		ShaderComposition* shaderColoredTexturedLerp;
		ShaderComposition* shaderColoredTexturedDesaturate;
		ShaderComposition* shaderColoredTexturedSepia;

		bool deviceState_constantBufferChanged;
		bool deviceState_colorModeChanged;
		ShaderComposition* deviceState_shader;
		ComPtr<ID3D11SamplerState> deviceState_sampler;
		RenderOperation deviceState_renderOperation;

		int _getBackbufferCount() const;

		void _deviceInit();
		bool _deviceCreate(Options options);
		bool _deviceDestroy();
		void _deviceAssignWindow(Window* window);
		void _deviceReset();
		void _deviceSuspend();
		void _deviceSetupCaps();
		void _deviceSetup();
		void _getAdapter(IDXGIAdapter1** adapter, bool hardware = true);

		void _createSwapChain(int width, int height);
		void _resizeSwapChain(int width, int height);
		void _configureSwapChain();
		void _configureDevice();
		
		Texture* _deviceCreateTexture(bool fromResource);
		PixelShader* _deviceCreatePixelShader();
		VertexShader* _deviceCreateVertexShader();

		void _deviceChangeResolution(int w, int h, bool fullscreen);

		void _setDeviceViewport(cgrecti rect);
		void _setDeviceModelviewMatrix(const gmat4& matrix);
		void _setDeviceProjectionMatrix(const gmat4& matrix);
		void _setDeviceDepthBuffer(bool enabled, bool writeEnabled);
		void _setDeviceRenderMode(bool useTexture, bool useColor);
		void _setDeviceTexture(Texture* texture);
		void _setDeviceTextureFilter(const Texture::Filter& textureFilter);
		void _setDeviceTextureAddressMode(const Texture::AddressMode& textureAddressMode);
		void _setDeviceBlendMode(const BlendMode& blendMode);
		void _setDeviceColorMode(const ColorMode& colorMode, float colorModeFactor, bool useTexture, bool useColor, const Color& systemColor);

		void _updateDeviceState(RenderState* state, bool forceUpdate);
		void _updateShader(bool forceUpdate);

		void _deviceClear(bool depth);
		void _deviceClear(const Color& color, bool depth);
		void _deviceClearDepth();
		void _deviceRender(const RenderOperation& renderOperation, const PlainVertex* vertices, int count);
		void _deviceRender(const RenderOperation& renderOperation, const TexturedVertex* vertices, int count);
		void _deviceRender(const RenderOperation& renderOperation, const ColoredVertex* vertices, int count);
		void _deviceRender(const RenderOperation& renderOperation, const ColoredTexturedVertex* vertices, int count);
		void _devicePresentFrame(bool systemEnabled);

		void _setDX11VertexBuffer(const RenderOperation& renderOperation, const void* data, int count, unsigned int vertexSize);

		static D3D11_PRIMITIVE_TOPOLOGY _dx11RenderOperations[];

	};

}
#endif
#endif
