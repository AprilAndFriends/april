/// @file
/// @version 4.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Defines a DirectX12 render system.

#ifdef _DIRECTX12
#ifndef APRIL_DirectX12_RENDER_SYSTEM_H
#define APRIL_DirectX12_RENDER_SYSTEM_H

#include <agile.h>
#include <dxgi1_4.h>
#include <d3d12.h>
#include "d3dx12.h"
#include <pix.h>

#include <gtypes/Matrix4.h>
#include <gtypes/Quaternion.h>
#include <gtypes/Rectangle.h>
#include <hltypes/harray.h>
#include <hltypes/hplatform.h>
#include <hltypes/hstring.h>

#include "DirectX_RenderSystem.h"
#include "Window.h"

#define FRAME_COUNT 3
#define ALIGNED_CONSTANT_BUFFER_SIZE ((sizeof(ConstantBuffer) + 255) & ~255)

using namespace Microsoft::WRL;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Display;
using namespace Windows::UI::Core;

namespace april
{
	class DirectX12_PixelShader;
	class DirectX12_Texture;
	class DirectX12_VertexShader;
	class Window;

	class DirectX12_RenderSystem : public DirectX_RenderSystem
	{
	public:
		friend class DirectX12_PixelShader;
		friend class DirectX12_Texture;
		friend class DirectX12_VertexShader;

		class PipelineState
		{
		public:
			friend class DirectX12_RenderSystem;

			PipelineState();// ComPtr<ID3D12InputLayout> inputLayout, DirectX12_VertexShader* vertexShader, DirectX12_PixelShader* pixelShader);
			~PipelineState();

		protected:
			//ComPtr<ID3D12InputLayout> inputLayout;
			//DirectX12_VertexShader* vertexShader;
			//DirectX12_PixelShader* pixelShader;

		};

		struct ConstantBuffer
		{
			gmat4 matrix;
			gquat systemColor;
			gquat lerpAlpha; // must be, because of 16 byte alignment of constant buffer size
		};

		DirectX12_RenderSystem();
		~DirectX12_RenderSystem();

		int getVRam() const;
		ID3D12CommandQueue* getCommandQueue() const { return this->commandQueue.Get(); }
		
		Image::Format getNativeTextureFormat(Image::Format format) const;
		unsigned int getNativeColorUInt(const april::Color& color) const;
		Image* takeScreenshot(Image::Format format);
		void presentFrame();

		void updateWindowSize(bool reconfigureIfChanged = true);
		void updateDeviceReset();

		void trim(); // needed by Win 8.1

		// TODOa - implement
		Texture* getRenderTarget();
		void setRenderTarget(Texture* source);
		void setPixelShader(PixelShader* pixelShader);
		void setVertexShader(VertexShader* vertexShader);

	protected:
		ComPtr<IDXGIFactory4> dxgiFactory;
		ComPtr<ID3D12Device> d3dDevice;
		ComPtr<IDXGISwapChain3> swapChain;
		Platform::Agile<CoreWindow> coreWindow;
		ComPtr<ID3D12RootSignature> rootSignature;

		ComPtr<ID3D12Resource> renderTargets[FRAME_COUNT];
		ComPtr<ID3D12Resource> depthStencil;
		ComPtr<ID3D12CommandQueue> commandQueue;
		ComPtr<ID3D12DescriptorHeap> rtvHeap; // render target view heap
		ComPtr<ID3D12DescriptorHeap> dsvHeap; // depth stencil view heap
		ComPtr<ID3D12DescriptorHeap> cbvHeap; // constant buffer view heap
		ComPtr<ID3D12CommandAllocator> commandAllocators[FRAME_COUNT];
		ComPtr<ID3D12GraphicsCommandList> commandList;
		D3D12_VIEWPORT screenViewport;

		//PipelineState pipelineStates[4];
		ComPtr<ID3D12PipelineState> pipelineState;

		// CPU/GPU synchronization
		ComPtr<ID3D12Fence> fence;
		UINT64 fenceValues[FRAME_COUNT];
		HANDLE fenceEvent;

		unsigned int currentFrame;
		unsigned int rtvDescriptorSize;
		unsigned int cbvDescriptorSize;

		// cached device properties
		Size d3dRenderTargetSize;
		Size outputSize;
		Size logicalSize;
		DisplayOrientations nativeOrientation;
		DisplayOrientations currentOrientation;
		float dpi;

		DXGI_MODE_ROTATION _getDxgiRotation() const;

		/*
		ComPtr<ID3D12DeviceContext2> d3dDeviceContext;

		ComPtr<ID3D12RasterizerState> rasterState;
		ComPtr<ID3D12RenderTargetView> renderTargetView;
		ComPtr<ID3D12BlendState> blendStateAlpha;
		ComPtr<ID3D12BlendState> blendStateAdd;
		ComPtr<ID3D12BlendState> blendStateSubtract;
		ComPtr<ID3D12BlendState> blendStateOverwrite;
		ComPtr<ID3D12SamplerState> samplerLinearWrap;
		ComPtr<ID3D12SamplerState> samplerLinearClamp;
		ComPtr<ID3D12SamplerState> samplerNearestWrap;
		ComPtr<ID3D12SamplerState> samplerNearestClamp;
		*/
		D3D12_HEAP_PROPERTIES uploadHeapProperties;
		D3D12_RESOURCE_DESC vertexBufferDescriptor;
		//D3D12_SUBRESOURCE_DATA vertexBufferDatas[2];
		D3D12_VERTEX_BUFFER_VIEW vertexBufferViews[2];
		//D3D12_MAPPED_SUBRESOURCE mappedSubResource;
		int vertexBuffersIndex;
		ComPtr<ID3D12Resource> vertexBuffers[2];
		ComPtr<ID3D12Resource> vertexBufferUploads[2];
		D3D12_RESOURCE_DESC constantBufferDescriptor;
		ComPtr<ID3D12Resource> constantBuffer;
		ConstantBuffer constantBufferData;
		unsigned char* mappedConstantBuffer;

		D3D12_RESOURCE_BARRIER resourceBarrier = {};
		/*
		ComPtr<ID3D12InputLayout> inputLayoutPlain;
		ComPtr<ID3D12InputLayout> inputLayoutTextured;
		ComPtr<ID3D12InputLayout> inputLayoutColored;
		ComPtr<ID3D12InputLayout> inputLayoutColoredTextured;
		//*/

		DirectX12_VertexShader* vertexShaderPlain;
		DirectX12_VertexShader* vertexShaderTextured;
		DirectX12_VertexShader* vertexShaderColored;
		DirectX12_VertexShader* vertexShaderColoredTextured;
		DirectX12_PixelShader* pixelShaderMultiply;
		DirectX12_PixelShader* pixelShaderLerp;
		DirectX12_PixelShader* pixelShaderAlphaMap;
		DirectX12_PixelShader* pixelShaderTexturedMultiply;
		DirectX12_PixelShader* pixelShaderTexturedLerp;
		DirectX12_PixelShader* pixelShaderTexturedAlphaMap;
		/*
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
		*/

		bool deviceState_constantBufferChanged;
		//ShaderComposition* deviceState_shader;
		//ComPtr<ID3D12SamplerState> deviceState_sampler;
		RenderOperation deviceState_renderOperation;

		void _deviceInit();
		bool _deviceCreate(Options options);
		bool _deviceDestroy();
		void _deviceAssignWindow(Window* window);
		void _deviceReset();
		void _deviceSetupCaps();
		void _deviceSetup();

		void _configureDevice();
		void _createSwapChain(int width, int height);
		void _resizeSwapChain(int width, int height);
		void _configureSwapChain(int width, int height);

		Texture* _deviceCreateTexture(bool fromResource);
		PixelShader* _deviceCreatePixelShader();
		VertexShader* _deviceCreateVertexShader();

		void _deviceChangeResolution(int w, int h, bool fullscreen);

		void _setDeviceViewport(cgrect rect);
		void _setDeviceModelviewMatrix(const gmat4& matrix);
		void _setDeviceProjectionMatrix(const gmat4& matrix);
		void _setDeviceDepthBuffer(bool enabled, bool writeEnabled);
		void _setDeviceRenderMode(bool useTexture, bool useColor);
		void _setDeviceTexture(Texture* texture);
		void _setDeviceTextureFilter(const Texture::Filter& textureFilter);
		void _setDeviceTextureAddressMode(const Texture::AddressMode& textureAddressMode);
		void _setDeviceBlendMode(const BlendMode& blendMode);
		void _setDeviceColorMode(const ColorMode& colorMode, float colorModeFactor, bool useTexture, bool useColor, const Color& systemColor);

		void _updateDeviceState(bool forceUpdate);
		void _updateShader(bool forceUpdate);

		void _deviceClear(bool depth);
		void _deviceClear(const Color& color, bool depth);
		void _deviceClearDepth();
		void _deviceRender(const RenderOperation& renderOperation, const PlainVertex* vertices, int count);
		void _deviceRender(const RenderOperation& renderOperation, const TexturedVertex* vertices, int count);
		void _deviceRender(const RenderOperation& renderOperation, const ColoredVertex* vertices, int count);
		void _deviceRender(const RenderOperation& renderOperation, const ColoredTexturedVertex* vertices, int count);

		void _setDX11VertexBuffer(const RenderOperation& renderOperation, const void* data, int count, unsigned int vertexSize);

		void _waitForGpu();

		static D3D_PRIMITIVE_TOPOLOGY _dx12RenderOperations[];

	};

}
#endif
#endif
