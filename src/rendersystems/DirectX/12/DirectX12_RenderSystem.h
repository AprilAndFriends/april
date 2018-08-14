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
/// Defines a DirectX12 render system.

#ifdef _DIRECTX12
#ifndef APRIL_DIRECTX12_RENDER_SYSTEM_H
#define APRIL_DIRECTX12_RENDER_SYSTEM_H

#include <agile.h>
#include <dxgi1_5.h>
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

#define MAX_BACKBUFFER_COUNT 3
#define MAX_COMMAND_LISTS 5
#define ALIGNED_CONSTANT_BUFFER_SIZE ((sizeof(ConstantBuffer) + 255) & ~255)
#define INPUT_LAYOUT_COUNT 4
#define PIXEL_SHADER_COUNT 5
#define BLEND_STATE_COUNT 4
#define TEXTURE_STATE_COUNT 2
#define PRIMITIVE_TOPOLOGY_COUNT 3
#define DEPTH_ENABLED_COUNT 2
#define MAX_VERTEX_BUFFERS 100 // should be enough vertex buffers to handle all cases

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

		struct ConstantBuffer
		{
			gmat4 matrix;
			gquat systemColor;
			gquat lerpAlpha; // must be, because of 16 byte alignment of constant buffer size
		};

		DirectX12_RenderSystem();
		~DirectX12_RenderSystem();

		int getVRam() const;
		
		Image::Format getNativeTextureFormat(Image::Format format) const;
		unsigned int getNativeColorUInt(const april::Color& color) const;
		Image* takeScreenshot(Image::Format format); // TODOuwp - implement screenshots properly

		void updateDeviceReset();

		void executeCurrentCommand();
		void waitForAllCommands();
		void waitForLastCommand();
		void prepareNewCommands();

		// TODOuwp - implement
		Texture* getRenderTarget();
		void setRenderTarget(Texture* source);
		void setPixelShader(PixelShader* pixelShader);
		void setVertexShader(VertexShader* vertexShader);

	protected:
		ComPtr<IDXGIFactory4> dxgiFactory;
		ComPtr<ID3D12Device> d3dDevice;
		ComPtr<IDXGISwapChain4> swapChain;
		Platform::Agile<CoreWindow> coreWindow;
		ComPtr<ID3D12RootSignature> rootSignatures[TEXTURE_STATE_COUNT];

		ComPtr<ID3D12Resource> renderTargets[MAX_BACKBUFFER_COUNT];
		ComPtr<ID3D12Resource> depthStencil;
		ComPtr<ID3D12CommandQueue> commandQueue;
		ComPtr<ID3D12DescriptorHeap> rtvHeap; // render target view heap
		ComPtr<ID3D12DescriptorHeap> cbvSrvUavHeaps[MAX_COMMAND_LISTS]; // constant buffer view, shader resource view, unordered access view heap
		ComPtr<ID3D12DescriptorHeap> samplerHeaps[MAX_COMMAND_LISTS]; // sampler heap
		ComPtr<ID3D12DescriptorHeap> dsvHeap; // depth stencil view heap
		ComPtr<ID3D12CommandAllocator> commandAllocators[MAX_COMMAND_LISTS][MAX_BACKBUFFER_COUNT];
		ComPtr<ID3D12GraphicsCommandList> commandList[MAX_COMMAND_LISTS];
		int commandListIndex;
		int commandListSize;

		harray<D3D12_INPUT_LAYOUT_DESC> inputLayoutDescs;
		harray<DirectX12_VertexShader*> vertexShaders;
		harray<DirectX12_PixelShader*> pixelShaders;
		harray<D3D12_RENDER_TARGET_BLEND_DESC> blendStateRenderTargets;
		harray<D3D12_PRIMITIVE_TOPOLOGY_TYPE> primitiveTopologyTypes;
		ComPtr<ID3D12PipelineState> pipelineStates[INPUT_LAYOUT_COUNT][PIXEL_SHADER_COUNT][BLEND_STATE_COUNT][PRIMITIVE_TOPOLOGY_COUNT][DEPTH_ENABLED_COUNT];

		// CPU/GPU synchronization
		ComPtr<ID3D12Fence> fence;
		UINT64 fenceLimits[MAX_BACKBUFFER_COUNT];
		UINT64 fenceValues[MAX_BACKBUFFER_COUNT];
		HANDLE fenceEvent;

		unsigned int currentFrame;
		unsigned int rtvDescSize;
		unsigned int cbvSrvUavDescSize;
		unsigned int samplerDescSize;
		unsigned int dsvDescSize;

		// cached device properties
		Size logicalSize;
		DisplayOrientations nativeOrientation;
		DisplayOrientations currentOrientation;
		float dpi;

		DXGI_MODE_ROTATION _getDxgiRotation() const;

		CD3DX12_HEAP_PROPERTIES uploadHeapProperties;
		D3D12_RESOURCE_DESC vertexBufferDesc;
		D3D12_VERTEX_BUFFER_VIEW vertexBufferViews[MAX_VERTEX_BUFFERS];
		int vertexBufferIndex;
		ComPtr<ID3D12Resource> vertexBuffers[MAX_VERTEX_BUFFERS];
		ComPtr<ID3D12Resource> vertexBufferUploads[MAX_VERTEX_BUFFERS];
		D3D12_RESOURCE_DESC constantBufferDesc;
		ComPtr<ID3D12Resource> constantBuffers[MAX_COMMAND_LISTS];
		ConstantBuffer constantBufferData;
		unsigned char* mappedConstantBuffers[MAX_COMMAND_LISTS];

		DirectX12_VertexShader* vertexShaderPlain;
		DirectX12_VertexShader* vertexShaderTextured;
		DirectX12_VertexShader* vertexShaderColored;
		DirectX12_VertexShader* vertexShaderColoredTextured;
		DirectX12_PixelShader* pixelShaderMultiply;
		DirectX12_PixelShader* pixelShaderAlphaMap;
		DirectX12_PixelShader* pixelShaderLerp;
		DirectX12_PixelShader* pixelShaderDesaturate;
		DirectX12_PixelShader* pixelShaderSepia;
		DirectX12_PixelShader* pixelShaderTexturedMultiply;
		DirectX12_PixelShader* pixelShaderTexturedAlphaMap;
		DirectX12_PixelShader* pixelShaderTexturedLerp;
		DirectX12_PixelShader* pixelShaderTexturedDesaturate;
		DirectX12_PixelShader* pixelShaderTexturedSepia;

		ComPtr<ID3D12PipelineState> deviceState_pipelineState;
		ComPtr<ID3D12RootSignature> deviceState_rootSignature;
		bool deviceState_constantBufferChanged;
		bool deviceState_colorModeChanged;
		bool deviceState_textureChanged;
		D3D12_VIEWPORT deviceViewport;
		D3D12_RECT deviceScissorRect;

		int _getBackbufferCount() const;

		void _deviceInit();
		bool _deviceCreate(Options options);
		bool _deviceDestroy();
		void _deviceAssignWindow(Window* window);
		void _deviceReset();
		void _deviceSetupCaps();
		void _deviceSetup();

		void _createD3dDevice();
		void _createHeapDescriptors();
		void _setupSwapChain();
		void _createSwapChain(int width, int height);
		void _resizeSwapChain(int width, int height);
		void _configureSwapChain(int width, int height);
		void _createRootSignatures();
		void _createShaders();
		void _createPipeline();

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

		void _deviceClear(bool depth);
		void _deviceClear(const Color& color, bool depth);
		void _deviceClearDepth();
		void _deviceRender(const RenderOperation& renderOperation, const PlainVertex* vertices, int count);
		void _deviceRender(const RenderOperation& renderOperation, const TexturedVertex* vertices, int count);
		void _deviceRender(const RenderOperation& renderOperation, const ColoredVertex* vertices, int count);
		void _deviceRender(const RenderOperation& renderOperation, const ColoredTexturedVertex* vertices, int count);
		void _devicePresentFrame(bool systemEnabled);
		void _deviceRepeatLastFrame();
		void _deviceCopyRenderTargetData(Texture* source, Texture* destination);

		void _updatePipelineState(const RenderOperation& renderOperation);
		void _renderDX12VertexBuffer(const RenderOperation& renderOperation, const void* data, int count, unsigned int vertexSize);

		void _waitForGpu();

		static D3D_PRIMITIVE_TOPOLOGY _dx12RenderOperations[];

	};

}
#endif
#endif
