/// @file
/// @version 4.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifdef _DIRECTX12
#include <d3d12.h>
#include <dxgi1_4.h>
#include <stdio.h>

#include <gtypes/Rectangle.h>
#include <gtypes/Vector2.h>
#include <hltypes/hexception.h>
#include <hltypes/hlog.h>
#include <hltypes/hplatform.h>
#include <hltypes/hresource.h>
#include <hltypes/hthread.h>

#include "april.h"
#include "DirectX12_PixelShader.h"
#include "DirectX12_RenderSystem.h"
#include "DirectX12_Texture.h"
#include "DirectX12_VertexShader.h"
#include "Image.h"
#include "Keys.h"
#include "Platform.h"
#include "RenderState.h"
#include "Timer.h"
#include "WinUWP.h"
#include "WinUWP_Window.h"

#define SHADER_PATH "april/"
#define VERTEX_BUFFER_COUNT 65536

#define __EXPAND(x) x

#define LOAD_SHADER(name, type, file) \
	if (name == NULL) \
	{ \
		name = (DirectX12_ ## type ## Shader*)this->create ## type ## ShaderFromResource(SHADER_PATH #type "Shader_" #file ".cso"); \
	}

#define CREATE_INPUT_LAYOUT(name, vertexType, layout) \
	if (name == nullptr) \
	{ \
		hr = this->d3dDevice->CreateInputLayout(layout, ARRAYSIZE(layout), \
			(unsigned char*)this->vertexShader ## vertexType->shaderData, (unsigned int)this->vertexShader ## vertexType->shaderData.size(), &name); \
		if (FAILED(hr)) \
		{ \
			throw Exception("Unable to create input layout for vertex shader!"); \
		} \
	}

#define CREATE_COMPOSITION(name, vertexType, pixelType) \
	if (name == NULL) \
	{ \
		name = new ShaderComposition(this->inputLayout ## vertexType, this->vertexShader ## vertexType, this->pixelShader ## pixelType); \
	}

#define _SELECT_SHADER(useTexture, useColor, type) \
	(useTexture ? (useColor ? this->shaderColoredTextured ## type : this->shaderTextured ## type) : (useColor ? this->shaderColored ## type : this->shader ## type));

using namespace Microsoft::WRL;
using namespace Windows::Graphics::Display;

namespace april
{
	static ColoredTexturedVertex static_ctv[VERTEX_BUFFER_COUNT];

	D3D12_PRIMITIVE_TOPOLOGY DirectX12_RenderSystem::_dx11RenderOperations[] =
	{
		D3D12_PRIMITIVE_TOPOLOGY_TRIANGLELIST,	// ROP_TRIANGLE_LIST
		D3D12_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP,	// ROP_TRIANGLE_STRIP
		D3D12_PRIMITIVE_TOPOLOGY_LINELIST,		// ROP_LINE_LIST
		D3D12_PRIMITIVE_TOPOLOGY_LINESTRIP,		// ROP_LINE_STRIP
		D3D12_PRIMITIVE_TOPOLOGY_POINTLIST,		// ROP_POINT_LIST
		D3D12_PRIMITIVE_TOPOLOGY_UNDEFINED,		// triangle fans are deprecated in DX11
	};

	DirectX12_RenderSystem::ShaderComposition::ShaderComposition(ComPtr<ID3D12InputLayout> inputLayout,
		DirectX12_VertexShader* vertexShader, DirectX12_PixelShader* pixelShader)
	{
		this->inputLayout = inputLayout;
		this->vertexShader = vertexShader;
		this->pixelShader = pixelShader;
	}

	DirectX12_RenderSystem::ShaderComposition::~ShaderComposition()
	{
	}

	DirectX12_RenderSystem::DirectX12_RenderSystem() : DirectX_RenderSystem(), deviceState_constantBufferChanged(true),
		deviceState_shader(NULL), deviceState_sampler(nullptr), deviceState_renderOperation(RenderOperation::PointList)
	{
		this->name = april::RenderSystemType::DirectX12.getName();
	}

	DirectX12_RenderSystem::~DirectX12_RenderSystem()
	{
		this->destroy();
	}

	void DirectX12_RenderSystem::_deviceInit()
	{
		this->d3dDevice = nullptr;
		this->d3dDeviceContext = nullptr;
		this->swapChain = nullptr;
		this->swapChainNative = nullptr;
		this->rasterState = nullptr;
		this->renderTargetView = nullptr;
		this->blendStateAlpha = nullptr;
		this->blendStateAdd = nullptr;
		this->blendStateSubtract = nullptr;
		this->blendStateOverwrite = nullptr;
		this->samplerLinearWrap = nullptr;
		this->samplerLinearClamp = nullptr;
		this->samplerNearestWrap = nullptr;
		this->samplerNearestClamp = nullptr;
		this->vertexBuffer = nullptr;
		this->constantBuffer = nullptr;
		this->inputLayoutPlain = nullptr;
		this->inputLayoutTextured = nullptr;
		this->inputLayoutColored = nullptr;
		this->inputLayoutColoredTextured = nullptr;
		this->vertexShaderPlain = NULL;
		this->vertexShaderTextured = NULL;
		this->vertexShaderColored = NULL;
		this->vertexShaderColoredTextured = NULL;
		this->pixelShaderMultiply = NULL;
		this->pixelShaderLerp = NULL;
		this->pixelShaderAlphaMap = NULL;
		this->pixelShaderTexturedMultiply = NULL;
		this->pixelShaderTexturedLerp = NULL;
		this->pixelShaderTexturedAlphaMap = NULL;
		this->shaderMultiply = NULL;
		this->shaderLerp = NULL;
		this->shaderAlphaMap = NULL;
		this->shaderTexturedMultiply = NULL;
		this->shaderTexturedLerp = NULL;
		this->shaderTexturedAlphaMap = NULL;
		this->shaderColoredMultiply = NULL;
		this->shaderColoredLerp = NULL;
		this->shaderColoredAlphaMap = NULL;
		this->shaderColoredTexturedMultiply = NULL;
		this->shaderColoredTexturedLerp = NULL;
		this->shaderColoredTexturedAlphaMap = NULL;
		this->deviceState_constantBufferChanged = true;
		this->deviceState_shader = NULL;
		this->deviceState_sampler = nullptr;
		this->deviceState_renderOperation = RenderOperation::PointList;
	}

	bool DirectX12_RenderSystem::_deviceCreate(Options options)
	{
		this->setViewport(grect(0.0f, 0.0f, april::getSystemInfo().displayResolution));
		return true;
	}

	bool DirectX12_RenderSystem::_deviceDestroy()
	{
		_HL_TRY_DELETE(this->vertexShaderPlain);
		_HL_TRY_DELETE(this->vertexShaderTextured);
		_HL_TRY_DELETE(this->vertexShaderColored);
		_HL_TRY_DELETE(this->vertexShaderColoredTextured);
		_HL_TRY_DELETE(this->pixelShaderMultiply);
		_HL_TRY_DELETE(this->pixelShaderLerp);
		_HL_TRY_DELETE(this->pixelShaderAlphaMap);
		_HL_TRY_DELETE(this->pixelShaderTexturedMultiply);
		_HL_TRY_DELETE(this->pixelShaderTexturedLerp);
		_HL_TRY_DELETE(this->pixelShaderTexturedAlphaMap);
		_HL_TRY_DELETE(this->shaderMultiply);
		_HL_TRY_DELETE(this->shaderLerp);
		_HL_TRY_DELETE(this->shaderAlphaMap);
		_HL_TRY_DELETE(this->shaderTexturedMultiply);
		_HL_TRY_DELETE(this->shaderTexturedLerp);
		_HL_TRY_DELETE(this->shaderTexturedAlphaMap);
		_HL_TRY_DELETE(this->shaderColoredMultiply);
		_HL_TRY_DELETE(this->shaderColoredLerp);
		_HL_TRY_DELETE(this->shaderColoredAlphaMap);
		_HL_TRY_DELETE(this->shaderColoredTexturedMultiply);
		_HL_TRY_DELETE(this->shaderColoredTexturedLerp);
		_HL_TRY_DELETE(this->shaderColoredTexturedAlphaMap);
		this->setViewport(grect(0.0f, 0.0f, april::getSystemInfo().displayResolution));
		return true;
	}

	void DirectX12_RenderSystem::_deviceAssignWindow(Window* window)
	{
		if (this->options.debugInfo)
		{
			ComPtr<ID3D12Debug> debugController;
			if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
			{
				debugController->EnableDebugLayer();
			}
		}
		// get factory first
		HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&this->dxgiFactory));
		if (FAILED(hr))
		{
			throw Exception("Unable to create DXGI factor!");
		}
		// get DX12-capable hardware adapter
		ComPtr<IDXGIAdapter1> adapter = nullptr;
		DXGI_ADAPTER_DESC1 adapterDescription;
		UINT adapterIndex = 0;
		while (this->dxgiFactory->EnumAdapters1(adapterIndex, &adapter) != DXGI_ERROR_NOT_FOUND)
		{
			adapter->GetDesc1(&adapterDescription);
			if ((adapterDescription.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0 && SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_9_1, _uuidof(ID3D12Device), nullptr)))
			{
				break;
			}
			++adapterIndex;
		}
		if (adapter == nullptr)
		{
			throw Exception("Unable to find DX12 adapter!");
		}
		// create actual device
		hr = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_9_1, IID_PPV_ARGS(&this->d3dDevice));
#ifdef _DEBUG
		if (FAILED(hr))
		{
			hlog::write(logTag, "Hardware device not available. Falling back to WARP device.");
			ComPtr<IDXGIAdapter> warpAdapter = nullptr;
			hr = this->dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter));
			if (FAILED(hr))
			{
				throw Exception("Unable to create DX12 device! WARP device not available!");
			}
			hr = D3D12CreateDevice(warpAdapter.Get(), D3D_FEATURE_LEVEL_9_1, IID_PPV_ARGS(&this->d3dDevice));
		}
#endif
		if (FAILED(hr))
		{
			throw Exception("Unable to create DX12 device!");
		}
		// create the command queue
		D3D12_COMMAND_QUEUE_DESC queueDescription = {};
		queueDescription.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDescription.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		hr = this->d3dDevice->CreateCommandQueue(&queueDescription, IID_PPV_ARGS(&this->commandQueue));
		if (FAILED(hr))
		{
			throw Exception("Unable to create command queue!");
		}
		// create descriptor heaps for render target views and depth stencil views
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDescription = {};
		rtvHeapDescription.NumDescriptors = FRAME_COUNT;
		rtvHeapDescription.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDescription.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		hr = this->d3dDevice->CreateDescriptorHeap(&rtvHeapDescription, IID_PPV_ARGS(&this->rtvHeap));
		if (FAILED(hr))
		{
			throw Exception("Unable to create RTV heap!");
		}
		this->rtvDescriptorSize = this->d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDescription = {};
		dsvHeapDescription.NumDescriptors = 1;
		dsvHeapDescription.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		dsvHeapDescription.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		hr = this->d3dDevice->CreateDescriptorHeap(&dsvHeapDescription, IID_PPV_ARGS(&this->dsvHeap));
		if (FAILED(hr))
		{
			throw Exception("Unable to create DSV heap!");
		}
		for_iter (i, 0, FRAME_COUNT)
		{
			hr = this->d3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&this->commandAllocators[i]));
			if (FAILED(hr))
			{
				throw Exception(hsprintf("Unable to create command allocator %d!", i));
			}
		}
		// create synchronization objects
		hr = this->d3dDevice->CreateFence(this->fenceValues[this->currentFrame], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&this->fence)));
		++this->fenceValues[this->currentFrame];
		this->fenceEvent = CreateEventEx(nullptr, FALSE, FALSE, EVENT_ALL_ACCESS);
		// configure device
		this->updateWindowSize(false);
		this->_configureDevice();
		// default shaders
		LOAD_SHADER(this->vertexShaderPlain, Vertex, Plain);
		LOAD_SHADER(this->vertexShaderTextured, Vertex, Textured);
		LOAD_SHADER(this->vertexShaderColored, Vertex, Colored);
		LOAD_SHADER(this->vertexShaderColoredTextured, Vertex, ColoredTextured);
		LOAD_SHADER(this->pixelShaderMultiply, Pixel, Multiply);
		LOAD_SHADER(this->pixelShaderLerp, Pixel, Lerp);
		LOAD_SHADER(this->pixelShaderAlphaMap, Pixel, AlphaMap);
		LOAD_SHADER(this->pixelShaderTexturedMultiply, Pixel, TexturedMultiply);
		LOAD_SHADER(this->pixelShaderTexturedLerp, Pixel, TexturedLerp);
		LOAD_SHADER(this->pixelShaderTexturedAlphaMap, Pixel, TexturedAlphaMap);
		// input layouts for default shaders
		static const D3D12_INPUT_ELEMENT_DESC inputLayoutDescPlain[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		};
		static const D3D12_INPUT_ELEMENT_DESC inputLayoutDescTextured[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		};
		static const D3D12_INPUT_ELEMENT_DESC inputLayoutDescColored[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		};
		static const D3D12_INPUT_ELEMENT_DESC inputLayoutDescColoredTextured[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		};
		D3D12_RENDER_TARGET_BLEND_DESC renderTargetAlpha;
		renderTargetAlpha.BlendEnable = true;
		renderTargetAlpha.LogicOpEnable = false;
		renderTargetAlpha.RenderTargetWriteMask = (D3D12_COLOR_WRITE_ENABLE_RED | D3D12_COLOR_WRITE_ENABLE_GREEN | D3D12_COLOR_WRITE_ENABLE_BLUE);
		renderTargetAlpha.BlendOpAlpha = D3D12_BLEND_OP_ADD;
		renderTargetAlpha.SrcBlendAlpha = D3D12_BLEND_ONE;
		renderTargetAlpha.DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
		renderTargetAlpha.BlendOp = D3D12_BLEND_OP_ADD;
		renderTargetAlpha.SrcBlend = D3D12_BLEND_SRC_ALPHA;
		renderTargetAlpha.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
		D3D12_RENDER_TARGET_BLEND_DESC renderTargetAdd;
		renderTargetAdd.BlendEnable = true;
		renderTargetAdd.LogicOpEnable = false;
		renderTargetAdd.RenderTargetWriteMask = (D3D12_COLOR_WRITE_ENABLE_RED | D3D12_COLOR_WRITE_ENABLE_GREEN | D3D12_COLOR_WRITE_ENABLE_BLUE);
		renderTargetAdd.BlendOpAlpha = D3D12_BLEND_OP_ADD;
		renderTargetAdd.SrcBlendAlpha = D3D12_BLEND_ONE;
		renderTargetAdd.DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
		renderTargetAdd.BlendOp = D3D12_BLEND_OP_ADD;
		renderTargetAdd.SrcBlend = D3D12_BLEND_SRC_ALPHA;
		renderTargetAdd.DestBlend = D3D12_BLEND_ONE;
		D3D12_RENDER_TARGET_BLEND_DESC renderTargetSubtract;
		renderTargetSubtract.BlendEnable = true;
		renderTargetSubtract.LogicOpEnable = false;
		renderTargetSubtract.RenderTargetWriteMask = (D3D12_COLOR_WRITE_ENABLE_RED | D3D12_COLOR_WRITE_ENABLE_GREEN | D3D12_COLOR_WRITE_ENABLE_BLUE);
		renderTargetSubtract.BlendOpAlpha = D3D12_BLEND_OP_ADD;
		renderTargetSubtract.SrcBlendAlpha = D3D12_BLEND_ONE;
		renderTargetSubtract.DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
		renderTargetSubtract.BlendOp = D3D12_BLEND_OP_REV_SUBTRACT;
		renderTargetSubtract.SrcBlend = D3D12_BLEND_SRC_ALPHA;
		renderTargetSubtract.DestBlend = D3D12_BLEND_ONE;
		D3D12_RENDER_TARGET_BLEND_DESC renderTargetOverwrite;
		renderTargetOverwrite.BlendEnable = true;
		renderTargetOverwrite.LogicOpEnable = false;
		renderTargetOverwrite.RenderTargetWriteMask = (D3D12_COLOR_WRITE_ENABLE_RED | D3D12_COLOR_WRITE_ENABLE_GREEN | D3D12_COLOR_WRITE_ENABLE_BLUE);
		renderTargetOverwrite.BlendOpAlpha = D3D12_BLEND_OP_ADD;
		renderTargetOverwrite.SrcBlendAlpha = D3D12_BLEND_ONE;
		renderTargetOverwrite.DestBlendAlpha = D3D12_BLEND_ZERO;
		renderTargetOverwrite.BlendOp = D3D12_BLEND_OP_ADD;
		renderTargetOverwrite.SrcBlend = D3D12_BLEND_SRC_ALPHA;
		renderTargetOverwrite.DestBlend = D3D12_BLEND_ZERO;
		const D3D12_DEPTH_STENCILOP_DESC defaultStencilOperation = { D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_COMPARISON_FUNC_ALWAYS };
		// pipeline states
		D3D12_GRAPHICS_PIPELINE_STATE_DESC state = {};
		state.pRootSignature = this->rootSignature.Get();
		state.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
		state.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
		state.RasterizerState.FrontCounterClockwise = false;
		state.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
		state.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
		state.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
		state.RasterizerState.DepthClipEnable = true;
		state.RasterizerState.MultisampleEnable = false;
		state.RasterizerState.AntialiasedLineEnable = false;
		state.RasterizerState.ForcedSampleCount = 0;
		state.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
		for_iter (i, 0, D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT)
		{
			state.RTVFormats[i] = DXGI_FORMAT_B8G8R8A8_UNORM;
		}
		state.DSVFormat = DXGI_FORMAT_D16_UNORM;
		state.SampleMask = UINT_MAX;
		state.NumRenderTargets = 1;
		state.SampleDesc.Count = 1;
		// dynamic properties
		state.InputLayout = { inputLayoutDescPlain, _countof(inputLayoutDescPlain) };
		state.VS.pShaderBytecode = (unsigned char*)this->vertexShaderPlain->shaderData;
		state.VS.BytecodeLength = this->vertexShaderPlain->shaderData.size();
		state.PS.pShaderBytecode = (unsigned char*)this->pixelShaderMultiply->shaderData;
		state.PS.BytecodeLength = this->pixelShaderMultiply->shaderData.size();
		state.BlendState.AlphaToCoverageEnable = false;
		state.BlendState.IndependentBlendEnable = false;
		for_iter (i, 0, D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT)
		{
			state.BlendState.RenderTarget[i] = renderTargetAlpha;
		}

		state.DepthStencilState.DepthEnable = false;
		state.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
		state.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
		state.DepthStencilState.StencilEnable = false;
		state.DepthStencilState.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
		state.DepthStencilState.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
		state.DepthStencilState.FrontFace = defaultStencilOperation;
		state.DepthStencilState.BackFace = defaultStencilOperation;


		//state.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		state.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		/*
		if (name == nullptr) \
		{ \
			hr = this->d3dDevice->CreateInputLayout(layout, ARRAYSIZE(layout), \
			(unsigned char*)this->vertexShader ## vertexType->shaderData, (unsigned int)this->vertexShader ## vertexType->shaderData.size(), &name); \
		if (FAILED(hr)) \
		{ \
			throw Exception("Unable to create input layout for vertex shader!"); \
		} \
	}
	*/

		hr = this->d3dDevice->CreateGraphicsPipelineState(&state, IID_PPV_ARGS(&this->pipelineState));
		if (FAILED(hr))
		{
			throw Exception("Unable to create graphics pipeline state!");
		}
		/*
		CREATE_INPUT_LAYOUT(this->inputLayoutPlain, Plain, inputLayoutDescPlain);
		CREATE_INPUT_LAYOUT(this->inputLayoutTextured, Textured, inputLayoutDescTextured);
		CREATE_INPUT_LAYOUT(this->inputLayoutColored, Colored, inputLayoutDescColored);
		CREATE_INPUT_LAYOUT(this->inputLayoutColoredTextured, ColoredTextured, inputLayoutDescColoredTextured);
		// shader compositions for rendering modes
		CREATE_COMPOSITION(this->shaderMultiply, Plain, Multiply);
		CREATE_COMPOSITION(this->shaderLerp, Plain, Lerp);
		CREATE_COMPOSITION(this->shaderAlphaMap, Plain, AlphaMap);
		CREATE_COMPOSITION(this->shaderTexturedMultiply, Textured, TexturedMultiply);
		CREATE_COMPOSITION(this->shaderTexturedLerp, Textured, TexturedLerp);
		CREATE_COMPOSITION(this->shaderTexturedAlphaMap, Textured, TexturedAlphaMap);
		CREATE_COMPOSITION(this->shaderColoredMultiply, Colored, Multiply);
		CREATE_COMPOSITION(this->shaderColoredLerp, Colored, Lerp);
		CREATE_COMPOSITION(this->shaderColoredAlphaMap, Colored, AlphaMap);
		CREATE_COMPOSITION(this->shaderColoredTexturedMultiply, ColoredTextured, TexturedMultiply);
		CREATE_COMPOSITION(this->shaderColoredTexturedLerp, ColoredTextured, TexturedLerp);
		CREATE_COMPOSITION(this->shaderColoredTexturedAlphaMap, ColoredTextured, TexturedAlphaMap);
		//*/

		
		// Create a command list.
		hr = this->d3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, this->commandAllocators[this->currentFrame].Get(),
			this->pipelineState.Get(), IID_PPV_ARGS(&this->commandList));
		if (FAILED(hr))
		{
			throw Exception("Unable to create graphics pipeline state!");
		}
		this->uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
		this->uploadHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		this->uploadHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		this->uploadHeapProperties.CreationNodeMask = 1;
		this->uploadHeapProperties.VisibleNodeMask = 1;
		//this->vertexBufferData.pSysMem = NULL;
		//this->vertexBufferData.SysMemPitch = 0;
		//this->vertexBufferData.SysMemSlicePitch = 0;
		this->vertexBufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		this->vertexBufferDesc.Alignment = 0;
		this->vertexBufferDesc.Width = 1;
		this->vertexBufferDesc.Height = 1;
		this->vertexBufferDesc.DepthOrArraySize = 1;
		this->vertexBufferDesc.MipLevels = 0;
		this->vertexBufferDesc.Format = DXGI_FORMAT_UNKNOWN;
		this->vertexBufferDesc.SampleDesc.Count = 1;
		this->vertexBufferDesc.SampleDesc.Quality = 0;
		this->vertexBufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		this->vertexBufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
		// initial constant buffer
		//D3D12_SUBRESOURCE_DATA constantSubresourceData = { 0 };
		//constantSubresourceData.pSysMem = &this->constantBufferData;
		//constantSubresourceData.SysMemPitch = 0;
		//constantSubresourceData.SysMemSlicePitch = 0;
		//D3D12_BUFFER_DESC constantBufferDesc = { 0 };
		this->constantBufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		this->constantBufferDesc.Alignment = 0;
		this->constantBufferDesc.Width = FRAME_COUNT * ALIGNED_CONSTANT_BUFFER_SIZE;
		this->constantBufferDesc.Height = 1;
		this->constantBufferDesc.DepthOrArraySize = 1;
		this->constantBufferDesc.MipLevels = 0;
		this->constantBufferDesc.Format = DXGI_FORMAT_UNKNOWN;
		this->constantBufferDesc.SampleDesc.Count = 1;
		this->constantBufferDesc.SampleDesc.Quality = 0;
		this->constantBufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		this->constantBufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;



		hr = this->d3dDevice->CreateCommittedResource(&this->uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &this->constantBufferDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&this->constantBuffer));

		/*
		constantBufferDesc.ByteWidth = sizeof(this->constantBufferData);
		constantBufferDesc.Usage = D3D12_USAGE_DYNAMIC;
		constantBufferDesc.BindFlags = D3D12_BIND_CONSTANT_BUFFER;
		constantBufferDesc.CPUAccessFlags = D3D12_CPU_ACCESS_WRITE;
		constantBufferDesc.MiscFlags = 0;
		constantBufferDesc.StructureByteStride = 0;
		hr = this->d3dDevice->CreateBuffer(&constantBufferDesc, &constantSubresourceData, &this->constantBuffer);
		if (FAILED(hr))
		{
			throw Exception("Unable to create constant buffer!");
		}
		this->d3dDeviceContext->VSSetConstantBuffers(0, 1, this->constantBuffer.GetAddressOf());
		*/



		
		/*
		// Cube vertices. Each vertex has a position and a color.
		VertexPositionColor cubeVertices[] =
		{
			{ XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT3(0.0f, 0.0f, 0.0f) },
			{ XMFLOAT3(-0.5f, -0.5f,  0.5f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
			{ XMFLOAT3(-0.5f,  0.5f, -0.5f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
			{ XMFLOAT3(-0.5f,  0.5f,  0.5f), XMFLOAT3(0.0f, 1.0f, 1.0f) },
			{ XMFLOAT3(0.5f, -0.5f, -0.5f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
			{ XMFLOAT3(0.5f, -0.5f,  0.5f), XMFLOAT3(1.0f, 0.0f, 1.0f) },
			{ XMFLOAT3(0.5f,  0.5f, -0.5f), XMFLOAT3(1.0f, 1.0f, 0.0f) },
			{ XMFLOAT3(0.5f,  0.5f,  0.5f), XMFLOAT3(1.0f, 1.0f, 1.0f) },
		};

		const UINT vertexBufferSize = sizeof(cubeVertices);

		// Create the vertex buffer resource in the GPU's default heap and copy vertex data into it using the upload heap.
		// The upload resource must not be released until after the GPU has finished using it.
		Microsoft::WRL::ComPtr<ID3D12Resource> vertexBufferUpload;

		CD3DX12_HEAP_PROPERTIES defaultHeapProperties(D3D12_HEAP_TYPE_DEFAULT);
		CD3DX12_RESOURCE_DESC vertexBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);
		DX::ThrowIfFailed(d3dDevice->CreateCommittedResource(
			&defaultHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&vertexBufferDesc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&m_vertexBuffer)));

		CD3DX12_HEAP_PROPERTIES uploadHeapProperties(D3D12_HEAP_TYPE_UPLOAD);
		DX::ThrowIfFailed(d3dDevice->CreateCommittedResource(
			&uploadHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&vertexBufferDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&vertexBufferUpload)));

		NAME_D3D12_OBJECT(m_vertexBuffer);

		// Upload the vertex buffer to the GPU.
		{
			D3D12_SUBRESOURCE_DATA vertexData = {};
			vertexData.pData = reinterpret_cast<BYTE*>(cubeVertices);
			vertexData.RowPitch = vertexBufferSize;
			vertexData.SlicePitch = vertexData.RowPitch;

			UpdateSubresources(m_commandList.Get(), m_vertexBuffer.Get(), vertexBufferUpload.Get(), 0, 0, 1, &vertexData);

			CD3DX12_RESOURCE_BARRIER vertexBufferResourceBarrier =
				CD3DX12_RESOURCE_BARRIER::Transition(m_vertexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
			m_commandList->ResourceBarrier(1, &vertexBufferResourceBarrier);
		}

		// Load mesh indices. Each trio of indices represents a triangle to be rendered on the screen.
		// For example: 0,2,1 means that the vertices with indexes 0, 2 and 1 from the vertex buffer compose the
		// first triangle of this mesh.
		unsigned short cubeIndices[] =
		{
			0, 2, 1, // -x
			1, 2, 3,

			4, 5, 6, // +x
			5, 7, 6,

			0, 1, 5, // -y
			0, 5, 4,

			2, 6, 7, // +y
			2, 7, 3,

			0, 4, 6, // -z
			0, 6, 2,

			1, 3, 7, // +z
			1, 7, 5,
		};

		const UINT indexBufferSize = sizeof(cubeIndices);

		// Create the index buffer resource in the GPU's default heap and copy index data into it using the upload heap.
		// The upload resource must not be released until after the GPU has finished using it.
		Microsoft::WRL::ComPtr<ID3D12Resource> indexBufferUpload;

		CD3DX12_RESOURCE_DESC indexBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize);
		DX::ThrowIfFailed(d3dDevice->CreateCommittedResource(
			&defaultHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&indexBufferDesc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&m_indexBuffer)));

		DX::ThrowIfFailed(d3dDevice->CreateCommittedResource(
			&uploadHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&indexBufferDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&indexBufferUpload)));

		NAME_D3D12_OBJECT(m_indexBuffer);

		// Upload the index buffer to the GPU.
		{
			D3D12_SUBRESOURCE_DATA indexData = {};
			indexData.pData = reinterpret_cast<BYTE*>(cubeIndices);
			indexData.RowPitch = indexBufferSize;
			indexData.SlicePitch = indexData.RowPitch;

			UpdateSubresources(m_commandList.Get(), m_indexBuffer.Get(), indexBufferUpload.Get(), 0, 0, 1, &indexData);

			CD3DX12_RESOURCE_BARRIER indexBufferResourceBarrier =
				CD3DX12_RESOURCE_BARRIER::Transition(m_indexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER);
			m_commandList->ResourceBarrier(1, &indexBufferResourceBarrier);
		}

		// Create a descriptor heap for the constant buffers.
		{
			D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
			heapDesc.NumDescriptors = DX::c_frameCount;
			heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			// This flag indicates that this descriptor heap can be bound to the pipeline and that descriptors contained in it can be referenced by a root table.
			heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			DX::ThrowIfFailed(d3dDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_cbvHeap)));

			NAME_D3D12_OBJECT(m_cbvHeap);
		}

		CD3DX12_RESOURCE_DESC constantBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(DX::c_frameCount * c_alignedConstantBufferSize);
		DX::ThrowIfFailed(d3dDevice->CreateCommittedResource(
			&uploadHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&constantBufferDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_constantBuffer)));

		NAME_D3D12_OBJECT(m_constantBuffer);
		*/

		// Create constant buffer views to access the upload buffer.
		D3D12_GPU_VIRTUAL_ADDRESS cbvGpuAddress = m_constantBuffer->GetGPUVirtualAddress();
		CD3DX12_CPU_DESCRIPTOR_HANDLE cbvCpuHandle(m_cbvHeap->GetCPUDescriptorHandleForHeapStart());
		m_cbvDescriptorSize = d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		for (int n = 0; n < DX::c_frameCount; n++)
		{
			D3D12_CONSTANT_BUFFER_VIEW_DESC desc;
			desc.BufferLocation = cbvGpuAddress;
			desc.SizeInBytes = c_alignedConstantBufferSize;
			d3dDevice->CreateConstantBufferView(&desc, cbvCpuHandle);

			cbvGpuAddress += desc.SizeInBytes;
			cbvCpuHandle.Offset(m_cbvDescriptorSize);
		}

		/*
		// Map the constant buffers.
		CD3DX12_RANGE readRange(0, 0);		// We do not intend to read from this resource on the CPU.
		DX::ThrowIfFailed(m_constantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&m_mappedConstantBuffer)));
		ZeroMemory(m_mappedConstantBuffer, DX::c_frameCount * c_alignedConstantBufferSize);
		// We don't unmap this until the app closes. Keeping things mapped for the lifetime of the resource is okay.

		// Close the command list and execute it to begin the vertex/index buffer copy into the GPU's default heap.
		DX::ThrowIfFailed(m_commandList->Close());
		ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
		m_deviceResources->GetCommandQueue()->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

		// Create vertex/index buffer views.
		m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
		m_vertexBufferView.StrideInBytes = sizeof(VertexPositionColor);
		m_vertexBufferView.SizeInBytes = sizeof(cubeVertices);

		m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
		m_indexBufferView.SizeInBytes = sizeof(cubeIndices);
		m_indexBufferView.Format = DXGI_FORMAT_R16_UINT;
		*/

		this->_waitForGpu();
		// initial calls
		this->_deviceClear(true);
		this->presentFrame();
		this->setOrthoProjection(gvec2((float)window->getWidth(), (float)window->getHeight()));
	}

	void DirectX12_RenderSystem::_deviceReset()
	{
		DirectX_RenderSystem::_deviceReset();
		// possible Microsoft bug, required for SwapChainPanel to update its layout 
		//reinterpret_cast<IUnknown*>(WinUWP::App->Overlay)->QueryInterface(IID_PPV_ARGS(&this->swapChainNative));
		//this->swapChainNative->SetSwapChain(this->swapChain.Get());
	}

	void DirectX12_RenderSystem::_deviceSetupCaps()
	{
		// depends on FEATURE_LEVEL, while 9.3 supports 4096, 9.2 and 9.1 support only 2048 so using 2048 is considered safe
		this->caps.maxTextureSize = D3D_FL9_1_REQ_TEXTURE1D_U_DIMENSION;
		this->caps.npotTexturesLimited = true;
		this->caps.npotTextures = false; // because of usage of feature level 9_3
	}

	void DirectX12_RenderSystem::_deviceSetup()
	{
		// not used
	}

	// Wait for pending GPU work to complete.
	void DirectX12_RenderSystem::_waitForGpu()
	{
		HRESULT hr = this->commandQueue->Signal(this->fence.Get(), this->fenceValues[this->currentFrame]);
		if (FAILED(hr))
		{
			throw Exception("Could not Signal command queue!");
		}
		hr = this->fence->SetEventOnCompletion(this->fenceValues[this->currentFrame], this->fenceEvent);
		if (FAILED(hr))
		{
			throw Exception("Could not Signal command queue!");
		}
		WaitForSingleObjectEx(this->fenceEvent, INFINITE, FALSE);
		++this->fenceValues[this->currentFrame];
	}



	void DirectX12_RenderSystem::_configureDevice()
	{
		this->_waitForGpu();
		for_iter (i, 0, FRAME_COUNT)
		{
			this->renderTargets[i] = nullptr;
			this->fenceValues[i] = this->fenceValues[this->currentFrame];
		}
		// swap chain
		float dpiRatio = WinUWP::getDpiRatio(this->dpi);
		this->outputSize.Width = hmax(hround(this->logicalSize.Width * dpiRatio), 1);
		this->outputSize.Height = hmax(hround(this->logicalSize.Width * dpiRatio), 1);
		DXGI_MODE_ROTATION displayRotation = this->_getDxgiRotation();
		if (displayRotation != DXGI_MODE_ROTATION_ROTATE90 && displayRotation != DXGI_MODE_ROTATION_ROTATE270)
		{
			this->d3dRenderTargetSize.Width = this->outputSize.Width;
			this->d3dRenderTargetSize.Height = this->outputSize.Height;
		}
		else
		{
			this->d3dRenderTargetSize.Width = this->outputSize.Height;
			this->d3dRenderTargetSize.Height = this->outputSize.Width;
		}
		if (this->swapChain != nullptr)
		{
			this->_resizeSwapChain(april::window->getWidth(), april::window->getHeight());
		}
		else
		{
			this->_createSwapChain(april::window->getWidth(), april::window->getHeight());
		}
		// other
		D3D12_DESCRIPTOR_RANGE range;
		range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
		range.NumDescriptors = 1;
		range.BaseShaderRegister = 0;
		range.RegisterSpace = 0;
		range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
		D3D12_ROOT_PARAMETER parameter;
		parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		parameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
		parameter.DescriptorTable.NumDescriptorRanges = 1;
		parameter.DescriptorTable.pDescriptorRanges = &range;
		D3D12_ROOT_SIGNATURE_DESC rootSignatureDescription;
		rootSignatureDescription.NumParameters = 1;
		rootSignatureDescription.pParameters = &parameter;
		rootSignatureDescription.NumStaticSamplers = 0;
		rootSignatureDescription.pStaticSamplers = nullptr;
		rootSignatureDescription.Flags = 
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | // Only the input assembler stage needs access to the constant buffer.
			D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;
		ComPtr<ID3DBlob> pSignature;
		ComPtr<ID3DBlob> pError;
		HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDescription, D3D_ROOT_SIGNATURE_VERSION_1, pSignature.GetAddressOf(), pError.GetAddressOf());
		if (FAILED(hr))
		{
			throw Exception("Unable to serialize root signature!");
		}
		hr = this->d3dDevice->CreateRootSignature(0, pSignature->GetBufferPointer(), pSignature->GetBufferSize(), IID_PPV_ARGS(&this->rootSignature));
		if (FAILED(hr))
		{
			throw Exception("Unable to create root signature!");
		}
		/*


		ComPtr<ID3D12Texture2D> _backBuffer;
		HRESULT hr = this->swapChain->GetBuffer(0, IID_PPV_ARGS(&_backBuffer));
		D3D12_RASTERIZER_DESC rasterDesc;
		rasterDesc.AntialiasedLineEnable = false;
		rasterDesc.CullMode = D3D12_CULL_NONE;
		rasterDesc.DepthBias = 0;
		rasterDesc.DepthBiasClamp = 0.0f;
		rasterDesc.DepthClipEnable = true;
		rasterDesc.FillMode = D3D12_FILL_SOLID;
		rasterDesc.FrontCounterClockwise = false;
		rasterDesc.MultisampleEnable = false;
		rasterDesc.ScissorEnable = false;
		rasterDesc.SlopeScaledDepthBias = 0.0f;
		hr = this->d3dDevice->CreateRasterizerState(&rasterDesc, &this->rasterState);
		if (FAILED(hr))
		{
			throw Exception("Unable to create raster state!");
		}
		this->d3dDeviceContext->RSSetState(this->rasterState.Get());
		D3D12_TEXTURE2D_DESC backBufferDesc = { 0 };
		_backBuffer->GetDesc(&backBufferDesc);
		SystemInfo info = april::getSystemInfo();
		this->setViewport(grect(0.0f, 0.0f, (float)backBufferDesc.Width, (float)backBufferDesc.Height)); // just to be on the safe side and prevent floating point errors
																										 // blend modes
		D3D12_BLEND_DESC blendDesc = { 0 };
		blendDesc.AlphaToCoverageEnable = false;
		blendDesc.IndependentBlendEnable = false;
		blendDesc.RenderTarget[0].BlendEnable = true;
		blendDesc.RenderTarget[0].RenderTargetWriteMask = (D3D12_COLOR_WRITE_ENABLE_RED | D3D12_COLOR_WRITE_ENABLE_GREEN | D3D12_COLOR_WRITE_ENABLE_BLUE);
		// alpha
		blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
		blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
		blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
		this->d3dDevice->CreateBlendState(&blendDesc, &this->blendStateAlpha);
		// add
		blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
		blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
		blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
		this->d3dDevice->CreateBlendState(&blendDesc, &this->blendStateAdd);
		// subtract
		blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
		blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
		blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_REV_SUBTRACT;
		blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
		this->d3dDevice->CreateBlendState(&blendDesc, &this->blendStateSubtract);
		// overwrite
		blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
		blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
		blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
		this->d3dDevice->CreateBlendState(&blendDesc, &this->blendStateOverwrite);
		// texture samplers
		D3D12_SAMPLER_DESC samplerDesc;
		memset(&samplerDesc, 0, sizeof(samplerDesc));
		samplerDesc.MaxAnisotropy = 0;
		samplerDesc.MipLODBias = 0.0f;
		samplerDesc.MinLOD = 0;
		samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
		samplerDesc.ComparisonFunc = D3D12_COMPARISON_NEVER;
		samplerDesc.BorderColor[0] = 0.0f;
		samplerDesc.BorderColor[1] = 0.0f;
		samplerDesc.BorderColor[2] = 0.0f;
		samplerDesc.BorderColor[3] = 0.0f;
		// linear + wrap
		samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_WRAP;
		this->d3dDevice->CreateSamplerState(&samplerDesc, &this->samplerLinearWrap);
		// linear + clamp
		samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_CLAMP;
		this->d3dDevice->CreateSamplerState(&samplerDesc, &this->samplerLinearClamp);
		// nearest neighbor + wrap
		samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
		samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_WRAP;
		this->d3dDevice->CreateSamplerState(&samplerDesc, &this->samplerNearestWrap);
		// nearest neighbor + clamp
		samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
		samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_CLAMP;
		this->d3dDevice->CreateSamplerState(&samplerDesc, &this->samplerNearestClamp);
		*/
		// other
		this->_deviceClear(true);
		this->presentFrame();
	}

	void DirectX12_RenderSystem::_createSwapChain(int width, int height)
	{
		DXGI_SWAP_CHAIN_DESC1 swapChainDescription = {};
		swapChainDescription.Width = width;
		swapChainDescription.Height = height;
		swapChainDescription.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		swapChainDescription.Stereo = false;
		swapChainDescription.SampleDesc.Count = 1;
		swapChainDescription.SampleDesc.Quality = 0;
		swapChainDescription.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDescription.BufferCount = FRAME_COUNT;
		swapChainDescription.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // UWP apps MUST use _FLIP_
		swapChainDescription.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
		swapChainDescription.Scaling = DXGI_SCALING_STRETCH;
		swapChainDescription.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
		ComPtr<IDXGISwapChain1> swapChain;
		HRESULT hr = this->dxgiFactory->CreateSwapChainForCoreWindow(this->commandQueue.Get(), reinterpret_cast<IUnknown*>(this->coreWindow.Get()), &swapChainDescription, nullptr, &swapChain);
		if (FAILED(hr))
		{
			throw Exception("Unable to create swap chain!");
		}
		hr = swapChain.As(&this->swapChain);
		if (FAILED(hr))
		{
			throw Exception("Unable to cast swap chain to non-COM object!");
		}
		this->_configureSwapChain(width, height);
	}

	void DirectX12_RenderSystem::_resizeSwapChain(int width, int height)
	{
		// If the swap chain already exists, resize it.
		HRESULT hr = this->swapChain->ResizeBuffers(FRAME_COUNT, width, height, DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);
		if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
		{
			// If the device was removed for any reason, a new device and swap chain will need to be created.
			//m_deviceRemoved = true;

			// Do not continue execution of this method. DeviceResources will be destroyed and re-created.
			return;
		}
		if (FAILED(hr))
		{
			throw Exception("Unable to resize swap chain buffers!");
		}
		this->_configureSwapChain(width, height);
	}

	void DirectX12_RenderSystem::_configureSwapChain(int width, int height)
	{
		/*
		// so... we have to apply an inverted scale to the swap chain?
		//DXGI_MATRIX_3X2_F inverseScale = { 0 };
		//inverseScale._11 = 1.0f / WinUWP::App->Overlay->CompositionScaleX;
		//inverseScale._22 = 1.0f / WinUWP::App->Overlay->CompositionScaleY;
		//this->swapChain->SetMatrixTransform(&inverseScale);
		// get the back buffer
		ComPtr<ID3D12Texture2D> _backBuffer;
		HRESULT hr = this->swapChain->GetBuffer(0, IID_PPV_ARGS(&_backBuffer));
		if (FAILED(hr))
		{
			throw Exception("Unable to get swap chain back buffer!");
		}
		// Create a descriptor for the RenderTargetView.
		CD3D12_RENDER_TARGET_VIEW_DESC renderTargetViewDesc(D3D12_RTV_DIMENSION_TEXTURE2DARRAY, DXGI_FORMAT_B8G8R8A8_UNORM, 0, 0, 1);
		hr = this->d3dDevice->CreateRenderTargetView(_backBuffer.Get(), &renderTargetViewDesc, &this->renderTargetView);
		if (FAILED(hr))
		{
			throw Exception("Unable to create render target view!");
		}
		// has to use GetAddressOf(), because the parameter is a pointer to an array of render target views
		this->d3dDeviceContext->OMSetRenderTargets(1, this->renderTargetView.GetAddressOf(), NULL);
		*/

		HRESULT hr = this->swapChain->SetRotation(this->_getDxgiRotation());
		if (FAILED(hr))
		{
			throw Exception("Unable to set rotation on swap chain!");
		}

		this->currentFrame = this->swapChain->GetCurrentBackBufferIndex();
		D3D12_CPU_DESCRIPTOR_HANDLE rtvDescriptor = this->rtvHeap->GetCPUDescriptorHandleForHeapStart();
		for_iter (i, 0, FRAME_COUNT)
		{
			hr = this->swapChain->GetBuffer(i, IID_PPV_ARGS(&this->renderTargets[i]));
			if (FAILED(hr))
			{
				throw Exception(hsprintf("Unable to get buffer %d from swap chain!", i));
			}
			this->d3dDevice->CreateRenderTargetView(this->renderTargets[i].Get(), nullptr, rtvDescriptor);
			rtvDescriptor.ptr += this->rtvDescriptorSize;
		}
		if (this->options.depthBuffer)
		{
			D3D12_HEAP_PROPERTIES depthHeapProperties = {};
			depthHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
			depthHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
			depthHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
			depthHeapProperties.CreationNodeMask = 1;
			depthHeapProperties.VisibleNodeMask = 1;
			D3D12_RESOURCE_DESC depthResourceDescription = {};
			depthResourceDescription.Format = DXGI_FORMAT_D16_UNORM;
			depthResourceDescription.Alignment = 0;
			depthResourceDescription.Width = width;
			depthResourceDescription.Height = height;
			depthResourceDescription.DepthOrArraySize = 1;
			depthResourceDescription.MipLevels = 1;
			depthResourceDescription.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
			depthResourceDescription.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
			depthResourceDescription.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
			depthResourceDescription.SampleDesc.Count = 1;
			depthResourceDescription.SampleDesc.Quality = 0;
			D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
			float depth = 1.0f;
			depthOptimizedClearValue.Format = DXGI_FORMAT_D16_UNORM;
			// using memcpy to preserve NAN values
			memcpy(&depthOptimizedClearValue.DepthStencil.Depth, &depth, sizeof(depth));
			depthOptimizedClearValue.DepthStencil.Stencil = 0;
			hr = this->d3dDevice->CreateCommittedResource(&depthHeapProperties, D3D12_HEAP_FLAG_NONE, &depthResourceDescription, D3D12_RESOURCE_STATE_DEPTH_WRITE,
				&depthOptimizedClearValue, IID_PPV_ARGS(&this->depthStencil));
			if (FAILED(hr))
			{
				throw Exception("Unable to create depth buffer!");
			}
			D3D12_DEPTH_STENCIL_VIEW_DESC dsvDescription = {};
			dsvDescription.Format = DXGI_FORMAT_D16_UNORM;
			dsvDescription.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
			dsvDescription.Flags = D3D12_DSV_FLAG_NONE;
			this->d3dDevice->CreateDepthStencilView(this->depthStencil.Get(), &dsvDescription, this->dsvHeap->GetCPUDescriptorHandleForHeapStart());
		}
		//this->screenViewport = { 0.0f, 0.0f, this->d3dRenderTargetSize.Width, this->d3dRenderTargetSize.Height, 0.0f, 1.0f };
	}

	int DirectX12_RenderSystem::getVRam() const
	{
		if (this->d3dDevice == nullptr)
		{
			return 0;
		}
		ComPtr<IDXGIDevice2> dxgiDevice;
		HRESULT hr = this->d3dDevice.As(&dxgiDevice);
		if (FAILED(hr))
		{
			hlog::error(logTag, "Unable to retrieve DXGI device!");
			return 0;
		}
		ComPtr<IDXGIAdapter> dxgiAdapter;
		hr = dxgiDevice->GetAdapter(&dxgiAdapter);
		if (FAILED(hr))
		{
			hlog::error(logTag, "Unable to get adapter from DXGI device!");
			return 0;
		}
		DXGI_ADAPTER_DESC desc;
		hr = dxgiAdapter->GetDesc(&desc);
		if (FAILED(hr))
		{
			hlog::error(logTag, "Unable to get description from DXGI adapter!");
			return 0;
		}
		return (desc.DedicatedVideoMemory / (1024 * 1024));
	}

	Texture* DirectX12_RenderSystem::_deviceCreateTexture(bool fromResource)
	{
		return new DirectX12_Texture(fromResource);
	}

	PixelShader* DirectX12_RenderSystem::_deviceCreatePixelShader()
	{
		return new DirectX12_PixelShader();
	}

	VertexShader* DirectX12_RenderSystem::_deviceCreateVertexShader()
	{
		return new DirectX12_VertexShader();
	}

	void DirectX12_RenderSystem::_deviceChangeResolution(int w, int h, bool fullscreen)
	{
		if (this->swapChain != nullptr)
		{
			this->_resizeSwapChain(april::window->getWidth(), april::window->getHeight());
		}
		else
		{
			this->_createSwapChain(april::window->getWidth(), april::window->getHeight());
		}
	}

	void DirectX12_RenderSystem::_setDeviceViewport(cgrect rect)
	{
		grect viewport = rect;
		// this is needed on WinRT because of a graphics driver bug on Windows RT and on WinP8 because of a completely different graphics driver bug on Windows Phone 8
		gvec2 resolution = april::getSystemInfo().displayResolution;
		int w = april::window->getWidth();
		int h = april::window->getHeight();
		if (viewport.x < 0.0f)
		{
			viewport.w += viewport.x;
			viewport.x = 0.0f;
		}
		if (viewport.y < 0.0f)
		{
			viewport.h += viewport.y;
			viewport.y = 0.0f;
		}
		viewport.w = hclamp(viewport.w, 0.0f, hmax(w - viewport.x, 0.0f));
		viewport.h = hclamp(viewport.h, 0.0f, hmax(h - viewport.y, 0.0f));
		if (viewport.w > 0.0f && viewport.h > 0.0f)
		{
			viewport.x = hclamp(viewport.x, 0.0f, (float)w);
			viewport.y = hclamp(viewport.y, 0.0f, (float)h);
		}
		else
		{
			viewport.set((float)w, (float)h, 0.0f, 0.0f);
		}
		// setting the system viewport
		D3D12_VIEWPORT dx12Viewport;
		dx12Viewport.MinDepth = D3D12_MIN_DEPTH;
		dx12Viewport.MaxDepth = D3D12_MAX_DEPTH;
		// these double-casts are to ensure consistent behavior among rendering systems
		dx12Viewport.TopLeftX = (float)(int)viewport.x;
		dx12Viewport.TopLeftY = (float)(int)viewport.y;
		dx12Viewport.Width = (float)(int)viewport.w;
		dx12Viewport.Height = (float)(int)viewport.h;
		// TODOuwp
		//this->d3dDeviceContext->RSSetViewports(1, &dx12Viewport);
	}

	void DirectX12_RenderSystem::_setDeviceModelviewMatrix(const gmat4& matrix)
	{
		this->deviceState_constantBufferChanged = true;
	}

	void DirectX12_RenderSystem::_setDeviceProjectionMatrix(const gmat4& matrix)
	{
		this->deviceState_constantBufferChanged = true;
	}

	void DirectX12_RenderSystem::_setDeviceDepthBuffer(bool enabled, bool writeEnabled)
	{
		hlog::error(logTag, "Not implemented!");
	}

	void DirectX12_RenderSystem::_setDeviceRenderMode(bool useTexture, bool useColor)
	{
		// not used
	}

	void DirectX12_RenderSystem::_setDeviceTexture(Texture* texture)
	{
		/*
		if (texture != NULL)
		{
			this->d3dDeviceContext->PSSetShaderResources(0, 1, ((DirectX12_Texture*)texture)->d3dView.GetAddressOf());
			Texture::Filter filter = texture->getFilter();
			Texture::AddressMode addressMode = texture->getAddressMode();
			ComPtr<ID3D12SamplerState> sampler = nullptr;
			if (filter == Texture::Filter::Linear && addressMode == Texture::AddressMode::Wrap)
			{
				sampler = this->samplerLinearWrap;
			}
			else if (filter == Texture::Filter::Linear && addressMode == Texture::AddressMode::Clamp)
			{
				sampler = this->samplerLinearClamp;
			}
			else if (filter == Texture::Filter::Nearest && addressMode == Texture::AddressMode::Wrap)
			{
				sampler = this->samplerNearestWrap;
			}
			else if (filter == Texture::Filter::Nearest && addressMode == Texture::AddressMode::Clamp)
			{
				sampler = this->samplerNearestClamp;
			}
			if (this->deviceState_sampler != sampler)
			{
				this->d3dDeviceContext->PSSetSamplers(0, 1, sampler.GetAddressOf());
				this->deviceState_sampler = sampler;
			}
		}
		else
		{
			this->d3dDeviceContext->PSSetShaderResources(0, 0, NULL);
		}
		*/
	}

	void DirectX12_RenderSystem::_setDeviceTextureFilter(const Texture::Filter& textureFilter)
	{
		// not used
	}

	void DirectX12_RenderSystem::_setDeviceTextureAddressMode(const Texture::AddressMode& textureAddressMode)
	{
		// not used
	}

	void DirectX12_RenderSystem::_setDeviceBlendMode(const BlendMode& blendMode)
	{
		/*
		static const float blendFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		if (blendMode == BlendMode::Alpha)
		{
			this->d3dDeviceContext->OMSetBlendState(this->blendStateAlpha.Get(), blendFactor, 0xFFFFFFFF);
		}
		else if (blendMode == BlendMode::Add)
		{
			this->d3dDeviceContext->OMSetBlendState(this->blendStateAdd.Get(), blendFactor, 0xFFFFFFFF);
		}
		else if (blendMode == BlendMode::Subtract)
		{
			this->d3dDeviceContext->OMSetBlendState(this->blendStateSubtract.Get(), blendFactor, 0xFFFFFFFF);
		}
		else if (blendMode == BlendMode::Overwrite)
		{
			this->d3dDeviceContext->OMSetBlendState(this->blendStateOverwrite.Get(), blendFactor, 0xFFFFFFFF);
		}
		else
		{
			hlog::error(logTag, "Trying to set unsupported blend mode!");
		}
		*/
	}

	void DirectX12_RenderSystem::_setDeviceColorMode(const ColorMode& colorMode, float colorModeFactor, bool useTexture, bool useColor, const Color& systemColor)
	{
		this->deviceState_constantBufferChanged = true;
	}

	void DirectX12_RenderSystem::_updateDeviceState(bool forceUpdate)
	{
		DirectX_RenderSystem::_updateDeviceState(forceUpdate);
		this->_updateShader(forceUpdate);
	}

	void DirectX12_RenderSystem::_updateShader(bool forceUpdate)
	{
		/*
		// select shader
		ShaderComposition* shader = NULL;
		if (this->deviceState->colorMode == ColorMode::Multiply)
		{
			shader = _SELECT_SHADER(this->deviceState->useTexture, this->deviceState->useColor, Multiply);
		}
		else if (this->deviceState->colorMode == ColorMode::AlphaMap)
		{
			shader = _SELECT_SHADER(this->deviceState->useTexture, this->deviceState->useColor, AlphaMap);
		}
		else if (this->deviceState->colorMode == ColorMode::Lerp)
		{
			shader = _SELECT_SHADER(this->deviceState->useTexture, this->deviceState->useColor, Lerp);
		}
		// change shaders
		bool inputLayoutChanged = false;
		bool vertexShaderChanged = false;
		bool pixelShaderChanged = false;
		if (this->deviceState_shader != shader)
		{
			if (this->deviceState_shader == NULL)
			{
				inputLayoutChanged = true;
				vertexShaderChanged = true;
				pixelShaderChanged = true;
			}
			else
			{
				inputLayoutChanged = (this->deviceState_shader->inputLayout != shader->inputLayout);
				vertexShaderChanged = (this->deviceState_shader->vertexShader != shader->vertexShader);
				pixelShaderChanged = (this->deviceState_shader->pixelShader != shader->pixelShader);
			}
			this->deviceState_shader = shader;
		}
		if (inputLayoutChanged)
		{
			this->d3dDeviceContext->IASetInputLayout(shader->inputLayout.Get());
		}
		if (vertexShaderChanged)
		{
			this->d3dDeviceContext->VSSetShader(shader->vertexShader->dx11Shader.Get(), NULL, 0);
		}
		if (pixelShaderChanged)
		{
			this->d3dDeviceContext->PSSetShader(shader->pixelShader->dx11Shader.Get(), NULL, 0);
		}
		*/
		// change other data
		if (this->deviceState_constantBufferChanged)
		{
			this->constantBufferData.matrix = (this->deviceState->projectionMatrix * this->deviceState->modelviewMatrix).transposed();
			this->constantBufferData.systemColor.set(this->deviceState->systemColor.r_f(), this->deviceState->systemColor.g_f(),
				this->deviceState->systemColor.b_f(), this->deviceState->systemColor.a_f());
			this->constantBufferData.lerpAlpha.set(this->deviceState->colorModeFactor, this->deviceState->colorModeFactor,
				this->deviceState->colorModeFactor, this->deviceState->colorModeFactor);


			//UINT8* destination = this->mappedConstantBuffer + (this->currentFrame * ALIGNED_CONSTANT_BUFFER_SIZE);
			//memcpy(destination, &m_constantBufferData, sizeof(m_constantBufferData));



			D3D12_RANGE readRange = {};
			readRange.Begin = 0;
			readRange.End = 0;
			unsigned char* mappedConstantBuffer = NULL;
			this->constantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&mappedConstantBuffer));
			

			//this->d3dDeviceContext->Map(this->constantBuffer.Get(), 0, D3D12_MAP_WRITE_DISCARD, 0, &this->mappedSubResource);
			memcpy(mappedConstantBuffer, &this->constantBufferData, sizeof(ConstantBuffer));
			this->constantBuffer->Unmap(0, nullptr);
			//this->d3dDeviceContext->Unmap(this->constantBuffer.Get(), 0);
			this->deviceState_constantBufferChanged = false;
		}
	}

	void DirectX12_RenderSystem::_deviceClear(bool depth)
	{
		static const float clearColor[4] = {0.0f, 0.0f, 0.0f, 0.0f};
		D3D12_CPU_DESCRIPTOR_HANDLE handle = this->rtvHeap->GetCPUDescriptorHandleForHeapStart();
		handle.ptr += this->currentFrame * this->rtvDescriptorSize;
		this->commandList->ClearRenderTargetView(handle, clearColor, 0, nullptr);
	}
	
	void DirectX12_RenderSystem::_deviceClear(const Color& color, bool depth)
	{
		static float clearColor[4] = {0.0f, 0.0f, 0.0f, 0.0f};
		clearColor[0] = color.b_f();
		clearColor[1] = color.g_f();
		clearColor[2] = color.r_f();
		clearColor[3] = color.a_f();
		D3D12_CPU_DESCRIPTOR_HANDLE handle = this->rtvHeap->GetCPUDescriptorHandleForHeapStart();
		handle.ptr += this->currentFrame * this->rtvDescriptorSize;
		this->commandList->ClearRenderTargetView(handle, clearColor, 0, nullptr);
	}

	void DirectX12_RenderSystem::_deviceClearDepth()
	{
		static const float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		this->commandList->ClearDepthStencilView(this->dsvHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	}

	void DirectX12_RenderSystem::_deviceRender(const RenderOperation& renderOperation, const PlainVertex* vertices, int count)
	{
		this->_setDX11VertexBuffer(renderOperation, vertices, count, sizeof(PlainVertex));
		this->d3dDeviceContext->Draw(count, 0);
	}

	void DirectX12_RenderSystem::_deviceRender(const RenderOperation& renderOperation, const TexturedVertex* vertices, int count)
	{
		this->_setDX11VertexBuffer(renderOperation, vertices, count, sizeof(TexturedVertex));
		this->d3dDeviceContext->Draw(count, 0);
	}

	void DirectX12_RenderSystem::_deviceRender(const RenderOperation& renderOperation, const ColoredVertex* vertices, int count)
	{
		this->_setDX11VertexBuffer(renderOperation, vertices, count, sizeof(ColoredVertex));
		this->d3dDeviceContext->Draw(count, 0);
	}

	void DirectX12_RenderSystem::_deviceRender(const RenderOperation& renderOperation, const ColoredTexturedVertex* vertices, int count)
	{
		this->_setDX11VertexBuffer(renderOperation, vertices, count, sizeof(ColoredTexturedVertex));
		this->d3dDeviceContext->Draw(count, 0);
	}

	void DirectX12_RenderSystem::_setDX11VertexBuffer(const RenderOperation& renderOperation, const void* data, int count, unsigned int vertexSize)
	{
		if (this->deviceState_renderOperation != renderOperation)
		{
			this->d3dDeviceContext->IASetPrimitiveTopology(_dx11RenderOperations[renderOperation.value]);
			this->deviceState_renderOperation = renderOperation;
		}
		unsigned int size = (unsigned int)(vertexSize * count);
		if (size > this->vertexBufferDesc.ByteWidth)
		{
			this->vertexBuffer = nullptr;
			this->vertexBufferData.pSysMem = data;
			this->vertexBufferDesc.ByteWidth = size;
			this->vertexBufferDesc.StructureByteStride = vertexSize;
			this->d3dDevice->CreateBuffer(&this->vertexBufferDesc, &this->vertexBufferData, &this->vertexBuffer);
		}
		else
		{
			this->d3dDeviceContext->Map(this->vertexBuffer.Get(), 0, D3D12_MAP_WRITE_DISCARD, 0, &this->mappedSubResource);
			memcpy(this->mappedSubResource.pData, data, size);
			this->d3dDeviceContext->Unmap(this->vertexBuffer.Get(), 0);
		}
		static unsigned int offset = 0;
		this->d3dDeviceContext->IASetVertexBuffers(0, 1, this->vertexBuffer.GetAddressOf(), &vertexSize, &offset);
	}

	Image::Format DirectX12_RenderSystem::getNativeTextureFormat(Image::Format format) const
	{
		if (format == Image::Format::RGBA || format == Image::Format::ARGB || format == Image::Format::BGRA || format == Image::Format::ABGR)
		{
			return Image::Format::BGRA;
		}
		if (format == Image::Format::RGBX || format == Image::Format::XRGB || format == Image::Format::BGRX ||
			format == Image::Format::XBGR || format == Image::Format::RGB || format == Image::Format::BGR)
		{
			return Image::Format::BGRX;
		}
		if (format == Image::Format::Alpha || format == Image::Format::Greyscale || format == Image::Format::Palette)
		{
			return format;
		}
		return Image::Format::Invalid;
	}
	
	unsigned int DirectX12_RenderSystem::getNativeColorUInt(const april::Color& color) const
	{
		return ((color.a << 24) | (color.b << 16) | (color.g << 8) | color.r);
	}

	Image* DirectX12_RenderSystem::takeScreenshot(Image::Format format)
	{
		// TODOa - if possible
		hlog::warn(logTag, "DirectX12_RenderSystem::takeScreenshot() not implemented!");
		return NULL;
	}
	
	void DirectX12_RenderSystem::presentFrame()
	{
		HRESULT hr = this->swapChain->Present(1, 0);

		// If the device was removed either by a disconnection or a driver upgrade, we 
		// must recreate all device resources.
		if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
		{
			//m_deviceRemoved = true;
			return;
		}
		if (FAILED(hr))
		{
			throw Exception("Unable to present swap chain!");
		}
		// Schedule a Signal command in the queue.
		const UINT64 currentFenceValue = this->fenceValues[this->currentFrame];
		hr = this->commandQueue->Signal(this->fence.Get(), currentFenceValue);
		if (FAILED(hr))
		{
			throw Exception("Unable to signal command queue!");
		}
		this->currentFrame = this->swapChain->GetCurrentBackBufferIndex();
		if (this->fence->GetCompletedValue() < this->fenceValues[this->currentFrame])
		{
			hr = this->fence->SetEventOnCompletion(this->fenceValues[this->currentFrame], this->fenceEvent);
			if (FAILED(hr))
			{
				throw Exception("Unable to set even on completion!");
			}
			WaitForSingleObjectEx(this->fenceEvent, INFINITE, FALSE);
		}
		this->fenceValues[this->currentFrame] = currentFenceValue + 1;
	}

	void DirectX12_RenderSystem::updateDeviceReset()
	{
		// TODOuwp
		/*
		if (this->deviceRemoved)
		{
			// do stuff
		}
		*/
	}

	void DirectX12_RenderSystem::updateWindowSize(bool reconfigureIfChanged)
	{
		this->coreWindow = CoreWindow::GetForCurrentThread();
		DisplayInformation^ displayInformation = DisplayInformation::GetForCurrentView();
		this->nativeOrientation = displayInformation->NativeOrientation;
		if (reconfigureIfChanged)
		{
			bool changed = false;
			Size newLogicalSize(this->coreWindow->Bounds.Width, this->coreWindow->Bounds.Height);
			if (this->logicalSize != newLogicalSize)
			{
				this->logicalSize != newLogicalSize;
				changed = true;
			}
			if (this->currentOrientation != displayInformation->CurrentOrientation)
			{
				this->currentOrientation = displayInformation->CurrentOrientation;
				changed = true;
			}
			if (this->dpi != displayInformation->LogicalDpi)
			{
				this->dpi = displayInformation->LogicalDpi;
				changed = true;
			}
			if (changed)
			{
				this->_configureDevice();
			}
		}
		else
		{
			this->logicalSize = Windows::Foundation::Size(this->coreWindow->Bounds.Width, this->coreWindow->Bounds.Height);
			this->currentOrientation = displayInformation->CurrentOrientation;
			this->dpi = displayInformation->LogicalDpi;
		}
	}

	DXGI_MODE_ROTATION DirectX12_RenderSystem::_getDxgiRotation() const
	{
		switch (this->nativeOrientation)
		{
		case DisplayOrientations::Landscape:
			switch (this->currentOrientation)
			{
			case DisplayOrientations::Landscape:		return DXGI_MODE_ROTATION_IDENTITY;
			case DisplayOrientations::Portrait:			return DXGI_MODE_ROTATION_ROTATE270;
			case DisplayOrientations::LandscapeFlipped:	return DXGI_MODE_ROTATION_ROTATE180;
			case DisplayOrientations::PortraitFlipped:	return DXGI_MODE_ROTATION_ROTATE90;
			}
			break;
		case DisplayOrientations::Portrait:
			switch (this->currentOrientation)
			{
			case DisplayOrientations::Landscape:		return DXGI_MODE_ROTATION_ROTATE90;
			case DisplayOrientations::Portrait:			return DXGI_MODE_ROTATION_IDENTITY;
			case DisplayOrientations::LandscapeFlipped:	return DXGI_MODE_ROTATION_ROTATE270;
			case DisplayOrientations::PortraitFlipped:	return DXGI_MODE_ROTATION_ROTATE180;
			}
			break;
		}
		return DXGI_MODE_ROTATION_UNSPECIFIED;
	}

	void DirectX12_RenderSystem::trim()
	{
		// TODOuwp - is this still needed?
		/*
		ComPtr<IDXGIDevice3> dxgiDevice;
		HRESULT hr = this->d3dDevice.As(&dxgiDevice);
		if (FAILED(hr))
		{
			throw Exception("Unable to retrieve DXGI device!");
		}
		dxgiDevice->Trim();
		*/
	}

	Texture* DirectX12_RenderSystem::getRenderTarget()
	{
		// TODOa - implement
		return NULL;// this->renderTarget;
	}

	void DirectX12_RenderSystem::setRenderTarget(Texture* source)
	{
		// TODOa - implement (this code is experimental)
		/*
		DirectX12_Texture* texture = (DirectX12_Texture*)source;
		if (texture == NULL)
		{
			// has to use GetAddressOf(), because the parameter is a pointer to an array of render target views
			this->d3dDeviceContext->OMSetRenderTargets(1, this->renderTargetView.GetAddressOf(), NULL);
		}
		else if (texture->d3dRenderTargetView != nullptr)
		{
			// has to use GetAddressOf(), because the parameter is a pointer to an array of render target views
			this->d3dDeviceContext->OMSetRenderTargets(1, texture->d3dRenderTargetView.GetAddressOf(), NULL);
		}
		else
		{
			hlog::error(logTag, "Texture not created as rendertarget: " + texture->_getInternalName());
			return;
		}
		this->renderTarget = texture;
		*/
		this->deviceState_constantBufferChanged = true;
	}

	void DirectX12_RenderSystem::setPixelShader(PixelShader* pixelShader)
	{
		// TODOa
		//this->activePixelShader = (DirectX12_PixelShader*)pixelShader;
	}

	void DirectX12_RenderSystem::setVertexShader(VertexShader* vertexShader)
	{
		// TODOa
		//this->activeVertexShader = (DirectX12_VertexShader*)vertexShader;
	}

}

#endif
