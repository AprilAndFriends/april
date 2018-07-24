/// @file
/// @version 5.1
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifdef _DIRECTX12
#include <comdef.h>
#include <d3d12.h>
#include <dxgi1_5.h>
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
#include "UWP.h"
#include "UWP_Window.h"

#define SHADER_PATH "april/"
#define VERTEX_BUFFER_COUNT 32768
#define MAX_D3D_FEATURE_LEVELS 4
#define CBV_SRV_UAV_HEAP_SIZE 2
#define SAMPLER_COUNT (Texture::Filter::getValues().size() * Texture::AddressMode::getValues().size())

#define __EXPAND(x) x

#define LOAD_SHADER(name, type, file) \
	if (name == NULL) \
	{ \
		name = (DirectX12_ ## type ## Shader*)this->create ## type ## ShaderFromResource(SHADER_PATH #type "Shader_" #file ".cso"); \
	}

using namespace Microsoft::WRL;
using namespace Windows::Graphics::Display;

namespace april
{
	static inline void _TRY_UNSAFE(HRESULT hr, chstr errorMessage)
	{
		if (FAILED(hr))
		{
			hstr systemError;
			try
			{
				char message[1024];
				FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, NULL, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), message, 1023, NULL);
				systemError = hstr(message).replaced("\r\n", "\n").trimmedRight('\n');
			}
			catch (...)
			{
			}
			throw Exception(hsprintf("%s - SYSTEM ERROR: '%s' - HRESULT: 0x%08X", errorMessage.cStr(), systemError.cStr(), hr));
		}
	}

	D3D_PRIMITIVE_TOPOLOGY DirectX12_RenderSystem::_dx12RenderOperations[] =
	{
		D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST,	// ROP_TRIANGLE_LIST
		D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP,	// ROP_TRIANGLE_STRIP
		D3D_PRIMITIVE_TOPOLOGY_LINELIST,		// ROP_LINE_LIST
		D3D_PRIMITIVE_TOPOLOGY_LINESTRIP,		// ROP_LINE_STRIP
		D3D_PRIMITIVE_TOPOLOGY_POINTLIST,		// ROP_POINT_LIST
		D3D_PRIMITIVE_TOPOLOGY_UNDEFINED,		// triangle fans are deprecated in DX12
	};

	DirectX12_RenderSystem::DirectX12_RenderSystem() : DirectX_RenderSystem(), deviceState_constantBufferChanged(true)
	{
		this->name = april::RenderSystemType::DirectX12.getName();
		this->vertexBufferIndex = 0;
	}

	DirectX12_RenderSystem::~DirectX12_RenderSystem()
	{
		this->destroy();
	}

	void DirectX12_RenderSystem::_deviceInit()
	{
		this->d3dDevice = nullptr;
		this->swapChain = nullptr;
		/*
		this->samplerLinearWrap = nullptr;
		this->samplerLinearClamp = nullptr;
		this->samplerNearestWrap = nullptr;
		this->samplerNearestClamp = nullptr;
		*/
		for_iter (i, 0, MAX_VERTEX_BUFFERS)
		{
			this->vertexBuffers[MAX_VERTEX_BUFFERS] = nullptr;
		}
		this->constantBuffer = nullptr;
		this->vertexShaderPlain = NULL;
		this->vertexShaderTextured = NULL;
		this->vertexShaderColored = NULL;
		this->vertexShaderColoredTextured = NULL;
		this->pixelShaderMultiply = NULL;
		this->pixelShaderAlphaMap = NULL;
		this->pixelShaderLerp = NULL;
		this->pixelShaderTexturedMultiply = NULL;
		this->pixelShaderTexturedAlphaMap = NULL;
		this->pixelShaderTexturedLerp = NULL;
		this->deviceState_constantBufferChanged = true;
	}

	bool DirectX12_RenderSystem::_deviceCreate(Options options)
	{
		return true;
	}

	bool DirectX12_RenderSystem::_deviceDestroy()
	{
		_HL_TRY_DELETE(this->vertexShaderPlain);
		_HL_TRY_DELETE(this->vertexShaderTextured);
		_HL_TRY_DELETE(this->vertexShaderColored);
		_HL_TRY_DELETE(this->vertexShaderColoredTextured);
		_HL_TRY_DELETE(this->pixelShaderMultiply);
		_HL_TRY_DELETE(this->pixelShaderAlphaMap);
		_HL_TRY_DELETE(this->pixelShaderLerp);
		_HL_TRY_DELETE(this->pixelShaderTexturedMultiply);
		_HL_TRY_DELETE(this->pixelShaderTexturedAlphaMap);
		_HL_TRY_DELETE(this->pixelShaderTexturedLerp);
		this->setViewport(grecti(0, 0, april::getSystemInfo().displayResolution));
		return true;
	}

	void DirectX12_RenderSystem::_deviceAssignWindow(Window* window)
	{
		unsigned int dxgiFactoryFlags = 0;
#ifndef _DEBUG
		if (this->options.debugInfo)
#endif
		{
			ComPtr<ID3D12Debug> debugController;
			if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
			{
				debugController->EnableDebugLayer();
			}
			dxgiFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
		}
		_TRY_UNSAFE(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&this->dxgiFactory)), "Unable to create DXGI factory!");
		if (!this->options.vSync)
		{
			// graphical debugging tools aren't available in 1.4 (at the time of writing) so they are upcast to 1.5 
			ComPtr<IDXGIFactory5> dxgiFactory5;
			if (SUCCEEDED(this->dxgiFactory.As(&dxgiFactory5)))
			{
				bool allowTearing = false;
				if (SUCCEEDED(dxgiFactory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing))))
				{
					if (allowTearing)
					{
						dxgiFactoryFlags |= DXGI_FEATURE_PRESENT_ALLOW_TEARING;
						ComPtr<IDXGIFactory4> newDxgiFactory;
						if (SUCCEEDED(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&newDxgiFactory))))
						{
							this->dxgiFactory = newDxgiFactory;
						}
						else
						{
							hlog::warn(logTag, "Cannot disable V-Sync, could not create new DXGI factory!");
							this->options.vSync = true;
						}
					}
					else
					{
						hlog::warn(logTag, "Cannot disable V-Sync, DXGI factory 5 says it's not supported!");
						this->options.vSync = true;
					}
				}
				else
				{
					hlog::warn(logTag, "Cannot disable V-Sync, DXGI factory 5 is unable to check for support!");
					this->options.vSync = true;
				}
			}
			else
			{
				hlog::warn(logTag, "Cannot disable V-Sync, DXGI factory 5 is not available!");
				this->options.vSync = true;
			}
		}
		// get DX12-capable hardware adapter
		ComPtr<IDXGIAdapter1> adapter = nullptr;
		DXGI_ADAPTER_DESC1 adapterDesc;
		UINT adapterIndex = 0;
		D3D_FEATURE_LEVEL availableFeatureLevels[MAX_D3D_FEATURE_LEVELS] = { D3D_FEATURE_LEVEL_12_1, D3D_FEATURE_LEVEL_12_0, D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0 };
		D3D_FEATURE_LEVEL featureLevel = availableFeatureLevels[0];
		for_iter (i, 0, MAX_D3D_FEATURE_LEVELS)
		{
			adapterIndex = 0;
			featureLevel = availableFeatureLevels[i];
			while (this->dxgiFactory->EnumAdapters1(adapterIndex, &adapter) != DXGI_ERROR_NOT_FOUND)
			{
				adapter->GetDesc1(&adapterDesc);
				if ((adapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0 && SUCCEEDED(D3D12CreateDevice(adapter.Get(), featureLevel, _uuidof(ID3D12Device), nullptr)))
				{
					break;
				}
				adapter = nullptr;
				++adapterIndex;
			}
			if (adapter != nullptr)
			{
				break;
			}
		}
		HRESULT hr;
		if (adapter != nullptr)
		{
			hr = D3D12CreateDevice(adapter.Get(), featureLevel, IID_PPV_ARGS(&this->d3dDevice));
			if (FAILED(hr))
			{
				hlog::write(logTag, "Hardware device not available. Falling back to WARP device.");
				adapter = nullptr;
			}
		}
		else
		{
			hlog::write(logTag, "Unable to find hardware adapter. Falling back to WARP device.");
		}
		// no valid adapter, use WARP
		if (adapter == nullptr)
		{
			ComPtr<IDXGIAdapter> warpAdapter = nullptr;
			_TRY_UNSAFE(this->dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)), "Unable to create DX12 device! WARP device not available!");
			hr = D3D12CreateDevice(warpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&this->d3dDevice));
		}
		_TRY_UNSAFE(hr, "Unable to create DX12 device!");
#ifdef _DEBUG
		ComPtr<ID3D12InfoQueue> pInfoQueue;
		if (SUCCEEDED(this->d3dDevice.As(&pInfoQueue)))
		{
			pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
			pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
			pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);
		}
#endif
		// resource descriptors
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
		rtvHeapDesc.NumDescriptors = BACKBUFFER_COUNT;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		_TRY_UNSAFE(this->d3dDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&this->rtvHeap)), "Unable to create RTV heap!");
		this->rtvHeap->SetName(L"RTV Heap");
		this->rtvDescSize = this->d3dDevice->GetDescriptorHandleIncrementSize(rtvHeapDesc.Type);
		D3D12_DESCRIPTOR_HEAP_DESC cbvSrvUavHeapDesc = {};
		cbvSrvUavHeapDesc.NumDescriptors = BACKBUFFER_COUNT * CBV_SRV_UAV_HEAP_SIZE;
		cbvSrvUavHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		cbvSrvUavHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		_TRY_UNSAFE(this->d3dDevice->CreateDescriptorHeap(&cbvSrvUavHeapDesc, IID_PPV_ARGS(&this->cbvSrvUavHeap)), "Unable to create CBV heap!");
		this->cbvSrvUavHeap->SetName(L"CBV Heap");
		this->cbvSrvUavDescSize = this->d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		D3D12_DESCRIPTOR_HEAP_DESC samplerHeapDesc = {};
		samplerHeapDesc.NumDescriptors = SAMPLER_COUNT;
		samplerHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
		samplerHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		_TRY_UNSAFE(this->d3dDevice->CreateDescriptorHeap(&samplerHeapDesc, IID_PPV_ARGS(&this->samplerHeap)), "Unable to create sampler heap!");
		this->samplerHeap->SetName(L"Sampler Heap");
		this->samplerDescSize = this->d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
		// TODOuwp - maybe could be removed / disabled as depth buffers aren't supported widely in april
		D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
		dsvHeapDesc.NumDescriptors = 1;
		dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		// TODOuwp - maybe could be removed / disabled as depth buffers aren't supported widely in april
		_TRY_UNSAFE(this->d3dDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&this->dsvHeap)), "Unable to create DSV heap!");
		this->dsvHeap->SetName(L"DSV Heap");
		this->dsvDescSize = this->d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		// commands
		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		queueDesc.NodeMask = 0;
		_TRY_UNSAFE(this->d3dDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&this->commandQueue)), "Unable to create command queue!");
		for_iter (i, 0, BACKBUFFER_COUNT)
		{
			_TRY_UNSAFE(this->d3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&this->commandAllocators[i])), hsprintf("Unable to create command allocator %d!", i));
		}
		// create synchronization objects
		_TRY_UNSAFE(this->d3dDevice->CreateFence(this->fenceValues[this->currentFrame], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&this->fence)), "Unable to create fence");
		++this->fenceValues[this->currentFrame];
		this->fenceEvent = CreateEventEx(nullptr, FALSE, FALSE, EVENT_ALL_ACCESS);
		// configure device
		this->coreWindow = CoreWindow::GetForCurrentThread();
		DisplayInformation^ displayInformation = DisplayInformation::GetForCurrentView();
		this->nativeOrientation = displayInformation->NativeOrientation;
		this->logicalSize = Windows::Foundation::Size((float)window->getWidth(), (float)window->getHeight());
		this->currentOrientation = displayInformation->CurrentOrientation;
		this->dpi = displayInformation->LogicalDpi;
		this->_configureDevice();
		// default shaders
		// TODOuwp - missing desaturate and sepia shaders
		LOAD_SHADER(this->vertexShaderPlain, Vertex, Plain);
		LOAD_SHADER(this->vertexShaderTextured, Vertex, Textured);
		LOAD_SHADER(this->vertexShaderColored, Vertex, Colored);
		LOAD_SHADER(this->vertexShaderColoredTextured, Vertex, ColoredTextured);
		LOAD_SHADER(this->pixelShaderMultiply, Pixel, Multiply);
		LOAD_SHADER(this->pixelShaderAlphaMap, Pixel, AlphaMap);
		LOAD_SHADER(this->pixelShaderLerp, Pixel, Lerp);
		LOAD_SHADER(this->pixelShaderTexturedMultiply, Pixel, TexturedMultiply);
		LOAD_SHADER(this->pixelShaderTexturedAlphaMap, Pixel, TexturedAlphaMap);
		LOAD_SHADER(this->pixelShaderTexturedLerp, Pixel, TexturedLerp);
		// input layouts for default shaders
		static const D3D12_INPUT_ELEMENT_DESC inputLayoutDescPlain[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		};
		static const D3D12_INPUT_ELEMENT_DESC inputLayoutDescColored[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		};
		static const D3D12_INPUT_ELEMENT_DESC inputLayoutDescTextured[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
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
		// indexed data
		// TODOuwp - missing desaturate and sepia shaders
		this->inputLayoutDescs.clear();
		this->inputLayoutDescs += { inputLayoutDescPlain, _countof(inputLayoutDescPlain) };
		this->inputLayoutDescs += { inputLayoutDescColored, _countof(inputLayoutDescColored) };
		this->inputLayoutDescs += { inputLayoutDescTextured, _countof(inputLayoutDescTextured) };
		this->inputLayoutDescs += { inputLayoutDescColoredTextured, _countof(inputLayoutDescColoredTextured) };
		this->vertexShaders.clear();
		this->vertexShaders += this->vertexShaderPlain;
		this->vertexShaders += this->vertexShaderColored;
		this->vertexShaders += this->vertexShaderTextured;
		this->vertexShaders += this->vertexShaderColoredTextured;
		this->pixelShaders.clear();
		this->pixelShaders += this->pixelShaderMultiply;
		this->pixelShaders += this->pixelShaderAlphaMap;
		this->pixelShaders += this->pixelShaderLerp;
		this->pixelShaders += this->pixelShaderTexturedMultiply;
		this->pixelShaders += this->pixelShaderTexturedAlphaMap;
		this->pixelShaders += this->pixelShaderTexturedLerp;
		this->blendStateRenderTargets.clear();
		this->blendStateRenderTargets += renderTargetAlpha;
		this->blendStateRenderTargets += renderTargetAdd;
		this->blendStateRenderTargets += renderTargetSubtract;
		this->blendStateRenderTargets += renderTargetOverwrite;
		this->primitiveTopologyTypes.clear();
		this->primitiveTopologyTypes += D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		this->primitiveTopologyTypes += D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
		this->primitiveTopologyTypes += D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
		// pipeline states
		D3D12_GRAPHICS_PIPELINE_STATE_DESC state = {};
		state.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
		state.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
		state.RasterizerState.FrontCounterClockwise = false;
		state.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
		state.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
		state.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
		state.RasterizerState.DepthClipEnable = true;
		state.RasterizerState.MultisampleEnable = false;
		state.RasterizerState.AntialiasedLineEnable = false;
		state.RasterizerState.ForcedSampleCount = 0;
		state.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
		state.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		for_iter (i, 1, D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT)
		{
			state.RTVFormats[i] = DXGI_FORMAT_UNKNOWN;
		}
		state.DSVFormat = DXGI_FORMAT_D16_UNORM;
		state.SampleMask = UINT_MAX;
		state.NumRenderTargets = 1;
		state.SampleDesc.Count = 1;
		state.SampleDesc.Quality = 0;
		state.BlendState.AlphaToCoverageEnable = false;
		state.BlendState.IndependentBlendEnable = false;
		state.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
		state.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
		state.DepthStencilState.StencilEnable = false;
		state.DepthStencilState.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
		state.DepthStencilState.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
		state.DepthStencilState.FrontFace = defaultStencilOperation;
		state.DepthStencilState.BackFace = defaultStencilOperation;
		// dynamic properties
		int pixelIndex = -1;
		// using half the shader size, because half the vertex shaders need to use one group of pixel shaders while the others need the other group
		int pixelShaderSize = this->pixelShaders.size() / 2;
		for_iter (i, 0, this->inputLayoutDescs.size())
		{
			state.InputLayout = this->inputLayoutDescs[i];
			state.VS.pShaderBytecode = (unsigned char*)this->vertexShaders[i]->shaderData;
			state.VS.BytecodeLength = (SIZE_T)this->vertexShaders[i]->shaderData.size();
			for_iter (j, 0, pixelShaderSize)
			{
				pixelIndex = j + (i / (this->inputLayoutDescs.size() / 2)) * pixelShaderSize;
				state.pRootSignature = this->rootSignatures[pixelIndex / pixelShaderSize].Get();
				state.PS.pShaderBytecode = (unsigned char*)this->pixelShaders[pixelIndex]->shaderData;
				state.PS.BytecodeLength = (SIZE_T)this->pixelShaders[pixelIndex]->shaderData.size();
				for_iter (k, 0, this->blendStateRenderTargets.size())
				{
					for_iter (m, 0, D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT)
					{
						state.BlendState.RenderTarget[m] = this->blendStateRenderTargets[k];
					}
					for_iter (m, 0, this->primitiveTopologyTypes.size())
					{
						state.PrimitiveTopologyType = this->primitiveTopologyTypes[m];
						state.DepthStencilState.DepthEnable = false;
						_TRY_UNSAFE(this->d3dDevice->CreateGraphicsPipelineState(&state, IID_PPV_ARGS(&this->pipelineStates[i][j][k][m][0])), "Unable to create graphics pipeline state!");
						// TODOuwp - maybe could be removed / disabled as depth buffers aren't supported widely in april
						state.DepthStencilState.DepthEnable = true;
						_TRY_UNSAFE(this->d3dDevice->CreateGraphicsPipelineState(&state, IID_PPV_ARGS(&this->pipelineStates[i][j][k][m][1])), "Unable to create graphics pipeline state!");
					}
				}
			}
		}
		this->deviceState_pipelineState = this->pipelineStates[0][0][0][0][0];
		this->deviceState_rootSignature = this->rootSignatures[0];
		_TRY_UNSAFE(this->d3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, this->commandAllocators[this->currentFrame].Get(),
			this->deviceState_pipelineState.Get(), IID_PPV_ARGS(&this->commandList)), "Unable to create command allocators state!");
		this->uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
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
		this->vertexBufferDesc.Width = VERTEX_BUFFER_COUNT;
		this->vertexBufferDesc.Height = 1;
		this->vertexBufferDesc.DepthOrArraySize = 1;
		this->vertexBufferDesc.MipLevels = 1;
		this->vertexBufferDesc.Format = DXGI_FORMAT_UNKNOWN;
		this->vertexBufferDesc.SampleDesc.Count = 1;
		this->vertexBufferDesc.SampleDesc.Quality = 0;
		this->vertexBufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		this->vertexBufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;


		CD3DX12_HEAP_PROPERTIES defaultHeapProperties(D3D12_HEAP_TYPE_DEFAULT);

		for_iter (i, 0, MAX_VERTEX_BUFFERS)
		{
			_TRY_UNSAFE(d3dDevice->CreateCommittedResource(&defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &this->vertexBufferDesc,
				D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&this->vertexBuffers[i])), "Unable to create vertex buffer!");
		}

		for_iter (i, 0, MAX_VERTEX_BUFFERS)
		{
			_TRY_UNSAFE(this->d3dDevice->CreateCommittedResource(&this->uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &this->vertexBufferDesc,
				D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&this->vertexBufferUploads[i])), "Unable to create vertex buffer upload!");
		}
		// constant buffer
		this->constantBufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		this->constantBufferDesc.Alignment = 0;
		this->constantBufferDesc.Width = BACKBUFFER_COUNT * ALIGNED_CONSTANT_BUFFER_SIZE;
		this->constantBufferDesc.Height = 1;
		this->constantBufferDesc.DepthOrArraySize = 1;
		this->constantBufferDesc.MipLevels = 1;
		this->constantBufferDesc.Format = DXGI_FORMAT_UNKNOWN;
		this->constantBufferDesc.SampleDesc.Count = 1;
		this->constantBufferDesc.SampleDesc.Quality = 0;
		this->constantBufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		this->constantBufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
		_TRY_UNSAFE(this->d3dDevice->CreateCommittedResource(&this->uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &this->constantBufferDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&this->constantBuffer)), "Unable to create constant buffer!");
		// constant buffer views
		D3D12_GPU_VIRTUAL_ADDRESS cbvGpuAddress = this->constantBuffer->GetGPUVirtualAddress();
		D3D12_CPU_DESCRIPTOR_HANDLE cbvCpuHandle = this->cbvSrvUavHeap->GetCPUDescriptorHandleForHeapStart();
		D3D12_CONSTANT_BUFFER_VIEW_DESC desc;
		desc.SizeInBytes = ALIGNED_CONSTANT_BUFFER_SIZE;
		for_iter (i, 0, BACKBUFFER_COUNT)
		{
			desc.BufferLocation = cbvGpuAddress;
			this->d3dDevice->CreateConstantBufferView(&desc, cbvCpuHandle);
			cbvGpuAddress += desc.SizeInBytes;
			cbvCpuHandle.ptr += this->cbvSrvUavDescSize;
		}
		D3D12_RANGE readRange = {};
		readRange.Begin = 0;
		readRange.End = 0;
		this->constantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&this->mappedConstantBuffer));
		// texture samplers
		D3D12_SAMPLER_DESC samplerDesc;
		memset(&samplerDesc, 0, sizeof(samplerDesc));
		samplerDesc.MaxAnisotropy = 0;
		samplerDesc.MipLODBias = 0.0f;
		samplerDesc.MinLOD = 0;
		samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
		samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		samplerDesc.BorderColor[0] = 0.0f;
		samplerDesc.BorderColor[1] = 0.0f;
		samplerDesc.BorderColor[2] = 0.0f;
		samplerDesc.BorderColor[3] = 0.0f;
		D3D12_CPU_DESCRIPTOR_HANDLE samplerCpuHandle = this->samplerHeap->GetCPUDescriptorHandleForHeapStart();
		int filterSize = Texture::Filter::getValues().size();
		int adressModeSize = Texture::AddressMode::getValues().size();
		for_iter (i, 0, filterSize)
		{
			samplerDesc.Filter = (i == 0 ? D3D12_FILTER_MIN_MAG_MIP_POINT : D3D12_FILTER_MIN_MAG_MIP_LINEAR);
			for_iter (j, 0, adressModeSize)
			{
				if (j == 0)
				{
					samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
					samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
					samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
				}
				else
				{
					samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
					samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
					samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
				}
				this->d3dDevice->CreateSampler(&samplerDesc, samplerCpuHandle);
				samplerCpuHandle.ptr += this->samplerDescSize;
			}
		}
		// finish and execute these commands
		_TRY_UNSAFE(this->commandList->Close(), "Unable to close command list!");
		ID3D12CommandList* ppCommandLists[] = { this->commandList.Get() };
		this->commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
		this->_waitForGpu();
		// initial calls
		_TRY_UNSAFE(this->commandAllocators[this->currentFrame]->Reset(), hsprintf("Unable to reset command allocator %d!", this->currentFrame));
		_TRY_UNSAFE(this->commandList->Reset(this->commandAllocators[this->currentFrame].Get(), this->deviceState_pipelineState.Get()), "Unable to reset command list!");
		PIXBeginEvent(this->commandList.Get(), 0, L"");
		grecti viewport(0, 0, window->getSize());
		gvec2f windowSizeFloat((float)viewport.w, (float)viewport.h);
		//this->_deviceClear(true);
		this->executeCurrentCommands();
		_TRY_UNSAFE(this->swapChain->Present(1, 0), "Unable to present initial swap chain!");
		this->deviceState_rootSignature = this->rootSignatures[0];
		this->waitForCommands();
		this->prepareNewCommands();
		D3D12_RESOURCE_BARRIER renderTargetResourceBarrier = {};
		renderTargetResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		renderTargetResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		renderTargetResourceBarrier.Transition.pResource = this->renderTargets[this->currentFrame].Get();
		renderTargetResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
		renderTargetResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		renderTargetResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		this->commandList->ResourceBarrier(1, &renderTargetResourceBarrier);
		this->setOrthoProjection(windowSizeFloat);
		this->setViewport(viewport);
	}

	void DirectX12_RenderSystem::_deviceReset()
	{
		DirectX_RenderSystem::_deviceReset();
		// possible Microsoft bug, required for SwapChainPanel to update its layout 
		//reinterpret_cast<IUnknown*>(UWP::App->Overlay)->QueryInterface(IID_PPV_ARGS(&this->swapChainNative));
		//this->swapChainNative->SetSwapChain(this->swapChain.Get());
	}

	void DirectX12_RenderSystem::_deviceSetupCaps()
	{
		this->caps.maxTextureSize = D3D_FL9_3_REQ_TEXTURE1D_U_DIMENSION;
		this->caps.npotTexturesLimited = true;
		// TODOuwp - this might actually work on DX12 by default, check
		this->caps.npotTextures = false;
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
		// swap chain
		float dpiRatio = UWP::getDpiRatio(this->dpi);
		Size outputSize((float)hmax(hround(this->logicalSize.Width * dpiRatio), 1), (float)hmax(hround(this->logicalSize.Height * dpiRatio), 1));
		DXGI_MODE_ROTATION displayRotation = this->_getDxgiRotation();
		if (displayRotation == DXGI_MODE_ROTATION_ROTATE90 || displayRotation == DXGI_MODE_ROTATION_ROTATE270)
		{
			hswap(outputSize.Width, outputSize.Height);
		}
		if (this->swapChain != nullptr)
		{
			this->_resizeSwapChain((int)outputSize.Width, (int)outputSize.Height);
		}
		else
		{
			this->_createSwapChain((int)outputSize.Width, (int)outputSize.Height);
		}
		// TODOuwp - is this the right place for this code? do root signatures need to be created on size change?
		// root signature without texture
		CD3DX12_DESCRIPTOR_RANGE ranges[3];
		ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
		CD3DX12_ROOT_PARAMETER parameters[3];
		parameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_VERTEX);
		CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Init(1, parameters, 0, NULL, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS);
		ComPtr<ID3DBlob> pSignature;
		ComPtr<ID3DBlob> pError;
		_TRY_UNSAFE(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, pSignature.GetAddressOf(), pError.GetAddressOf()), "Unable to serialize root signature!");
		_TRY_UNSAFE(this->d3dDevice->CreateRootSignature(0, pSignature->GetBufferPointer(), pSignature->GetBufferSize(), IID_PPV_ARGS(&this->rootSignatures[0])), "Unable to create root signature!");
		this->rootSignatures[0]->SetName(L"Root Signature 0");
		// root signature with texture
		ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
		ranges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0);
		parameters[1].InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_PIXEL);
		parameters[2].InitAsDescriptorTable(1, &ranges[2], D3D12_SHADER_VISIBILITY_PIXEL);
		rootSignatureDesc.Init(3, parameters, 0, NULL, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS);
		_TRY_UNSAFE(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, pSignature.GetAddressOf(), pError.GetAddressOf()), "Unable to serialize root signature!");
		_TRY_UNSAFE(this->d3dDevice->CreateRootSignature(0, pSignature->GetBufferPointer(), pSignature->GetBufferSize(), IID_PPV_ARGS(&this->rootSignatures[1])), "Unable to create root signature!");
		this->rootSignatures[1]->SetName(L"Root Signature 1");
	}

	void DirectX12_RenderSystem::_createSwapChain(int width, int height)
	{
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
		swapChainDesc.Width = width;
		swapChainDesc.Height = height;
		swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.Stereo = false;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = BACKBUFFER_COUNT;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // UWP apps MUST use a "_FLIP_" variant
		swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
		swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
		swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
		if (!this->options.vSync)
		{
			swapChainDesc.Flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
		}
		ComPtr<IDXGISwapChain1> swapChain;
		_TRY_UNSAFE(this->dxgiFactory->CreateSwapChainForCoreWindow(this->commandQueue.Get(), reinterpret_cast<IUnknown*>(this->coreWindow.Get()),
			&swapChainDesc, nullptr, &swapChain), "Unable to create swap chain!");
		_TRY_UNSAFE(swapChain.As(&this->swapChain), "Unable to cast swap chain to non-COM object!");
		DXGI_RGBA black = { 0.0f, 0.0f, 0.0f, 1.0f };
		this->swapChain->SetBackgroundColor(&black);
		this->_configureSwapChain(width, height);
	}

	void DirectX12_RenderSystem::_resizeSwapChain(int width, int height)
	{
		this->executeCurrentCommands();
		this->waitForCommands();
		this->_waitForGpu();
		for_iter (i, 0, BACKBUFFER_COUNT)
		{
			this->renderTargets[i] = nullptr;
			this->fenceValues[i] = this->fenceValues[this->currentFrame];
		}
		DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
		this->swapChain->GetDesc(&swapChainDesc);
		HRESULT hr = this->swapChain->ResizeBuffers(0, width, height, swapChainDesc.BufferDesc.Format, swapChainDesc.Flags);
		if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
		{
			// TODOuwp - this needs to be tested and fixed or implemented
			// If the device was removed for any reason, a new device and swap chain will need to be created.
			//m_deviceRemoved = true;

			// Do not continue execution of this method. DeviceResources will be destroyed and re-created.
			return;
		}
		_TRY_UNSAFE(hr, "Unable to resize swap chain buffers!");
		this->_configureSwapChain(width, height);
		this->prepareNewCommands();
	}

	void DirectX12_RenderSystem::_configureSwapChain(int width, int height)
	{
		_TRY_UNSAFE(this->swapChain->SetRotation(this->_getDxgiRotation()), "Unable to set rotation on swap chain!");
		this->currentFrame = this->swapChain->GetCurrentBackBufferIndex();
		D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = this->rtvHeap->GetCPUDescriptorHandleForHeapStart();
		for_iter (i, 0, BACKBUFFER_COUNT)
		{
			_TRY_UNSAFE(this->swapChain->GetBuffer(i, IID_PPV_ARGS(&this->renderTargets[i])), hsprintf("Unable to get buffer %d from swap chain!", i));
			this->d3dDevice->CreateRenderTargetView(this->renderTargets[i].Get(), nullptr, cpuHandle);
			this->renderTargets[i]->SetName(("Render Target " + hstr(i)).wStr().c_str());
			cpuHandle.ptr += this->rtvDescSize;
		}
		// TODOuwp - maybe could be removed / disabled as depth buffers aren't supported widely in april
		if (this->options.depthBuffer)
		{
			D3D12_HEAP_PROPERTIES depthHeapProperties = {};
			depthHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
			depthHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
			depthHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
			depthHeapProperties.CreationNodeMask = 1;
			depthHeapProperties.VisibleNodeMask = 1;
			D3D12_RESOURCE_DESC depthResourceDesc = {};
			depthResourceDesc.Format = DXGI_FORMAT_D16_UNORM;
			depthResourceDesc.Alignment = 0;
			depthResourceDesc.Width = width;
			depthResourceDesc.Height = height;
			depthResourceDesc.DepthOrArraySize = 1;
			depthResourceDesc.MipLevels = 1;
			depthResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
			depthResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
			depthResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
			depthResourceDesc.SampleDesc.Count = 1;
			depthResourceDesc.SampleDesc.Quality = 0;
			D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
			float depth = 1.0f;
			depthOptimizedClearValue.Format = DXGI_FORMAT_D16_UNORM;
			// using memcpy to preserve NAN values
			memcpy(&depthOptimizedClearValue.DepthStencil.Depth, &depth, sizeof(depth));
			depthOptimizedClearValue.DepthStencil.Stencil = 0;
			_TRY_UNSAFE(this->d3dDevice->CreateCommittedResource(&depthHeapProperties, D3D12_HEAP_FLAG_NONE, &depthResourceDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE,
				&depthOptimizedClearValue, IID_PPV_ARGS(&this->depthStencil)), "Unable to create depth buffer!");
			D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
			dsvDesc.Format = DXGI_FORMAT_D16_UNORM;
			dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
			dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
			this->d3dDevice->CreateDepthStencilView(this->depthStencil.Get(), &dsvDesc, this->dsvHeap->GetCPUDescriptorHandleForHeapStart());
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
		this->coreWindow = CoreWindow::GetForCurrentThread();
		DisplayInformation^ displayInformation = DisplayInformation::GetForCurrentView();
		this->nativeOrientation = displayInformation->NativeOrientation;
		bool changed = false;
		Size newLogicalSize(this->coreWindow->Bounds.Width, this->coreWindow->Bounds.Height);
		if (this->logicalSize != newLogicalSize)
		{
			this->logicalSize = newLogicalSize;
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

	void DirectX12_RenderSystem::_setDeviceViewport(cgrecti rect)
	{
		// not used
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
		if (enabled || writeEnabled)
		{
			hlog::error(logTag, "_setDeviceDepthBuffer() is not implemented in: " + this->name);
		}
	}

	void DirectX12_RenderSystem::_setDeviceRenderMode(bool useTexture, bool useColor)
	{
		// not used
	}

	void DirectX12_RenderSystem::_setDeviceTexture(Texture* texture)
	{
		// not really the constant buffer, but the texture update
		this->deviceState_constantBufferChanged = true;
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
		// not used
	}

	void DirectX12_RenderSystem::_setDeviceColorMode(const ColorMode& colorMode, float colorModeFactor, bool useTexture, bool useColor, const Color& systemColor)
	{
		this->deviceState_constantBufferChanged = true;
	}

	void DirectX12_RenderSystem::_deviceClear(bool depth)
	{
		static const float clearColor[4] = {0.0f, 0.0f, 0.0f, 0.0f};
		CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle(this->rtvHeap->GetCPUDescriptorHandleForHeapStart(), this->currentFrame, this->rtvDescSize);
		this->commandList->ClearRenderTargetView(cpuHandle, clearColor, 0, nullptr);
	}
	
	void DirectX12_RenderSystem::_deviceClear(const Color& color, bool depth)
	{
		static float clearColor[4] = {0.0f, 0.0f, 0.0f, 0.0f};
		clearColor[0] = color.b_f();
		clearColor[1] = color.g_f();
		clearColor[2] = color.r_f();
		clearColor[3] = color.a_f();
		CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle(this->rtvHeap->GetCPUDescriptorHandleForHeapStart(), this->currentFrame, this->rtvDescSize);
		this->commandList->ClearRenderTargetView(cpuHandle, clearColor, 0, nullptr);
	}

	void DirectX12_RenderSystem::_deviceClearDepth()
	{
		static const float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		// TODOuwp - maybe could be removed / disabled as depth buffers aren't supported widely in april
		this->commandList->ClearDepthStencilView(this->dsvHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	}

	void DirectX12_RenderSystem::_deviceRender(const RenderOperation& renderOperation, const PlainVertex* vertices, int count)
	{
		this->_renderDX12VertexBuffer(renderOperation, vertices, count, sizeof(PlainVertex));
	}

	void DirectX12_RenderSystem::_deviceRender(const RenderOperation& renderOperation, const TexturedVertex* vertices, int count)
	{
		this->_renderDX12VertexBuffer(renderOperation, vertices, count, sizeof(TexturedVertex));
	}

	void DirectX12_RenderSystem::_deviceRender(const RenderOperation& renderOperation, const ColoredVertex* vertices, int count)
	{
		this->_renderDX12VertexBuffer(renderOperation, vertices, count, sizeof(ColoredVertex));
	}

	void DirectX12_RenderSystem::_deviceRender(const RenderOperation& renderOperation, const ColoredTexturedVertex* vertices, int count)
	{
		this->_renderDX12VertexBuffer(renderOperation, vertices, count, sizeof(ColoredTexturedVertex));
	}

	void DirectX12_RenderSystem::_updatePipelineState(const RenderOperation& renderOperation)
	{
		int r = 0;
		int i = 0;
		int j = this->deviceState->colorMode.value;
		int k = this->deviceState->blendMode.value;
		int l = 0;
		int m = 0;
		if (this->deviceState->useTexture)
		{
			i += 2;
			++r;
		}
		if (this->deviceState->useColor)
		{
			++i;
		}
		if (renderOperation.isLine())
		{
			++l;
		}
		else if (renderOperation.isPoint())
		{
			l += 2;
		}
		if (this->deviceState->depthBuffer)
		{
			++m;
		}
		bool changed = false;
		if (this->deviceState_constantBufferChanged || this->deviceState_pipelineState != this->pipelineStates[i][j][k][l][m])
		{
			changed = true;
		}
		if (!changed)
		{
			return;
		}
		this->deviceState_pipelineState = this->pipelineStates[i][j][k][l][m];
		this->deviceState_rootSignature = this->rootSignatures[r];
		//this->deviceState_rootSignature = this->rootSignatures[0];
		this->executeCurrentCommands();
		this->waitForCommands();
		this->prepareNewCommands();
		if (this->deviceState_constantBufferChanged)
		{
			this->constantBufferData.matrix = (this->deviceState->projectionMatrix * this->deviceState->modelviewMatrix).transposed();
			this->constantBufferData.systemColor.set(this->deviceState->systemColor.r_f(), this->deviceState->systemColor.g_f(),
				this->deviceState->systemColor.b_f(), this->deviceState->systemColor.a_f());
			this->constantBufferData.lerpAlpha.set(this->deviceState->colorModeFactor, this->deviceState->colorModeFactor,
				this->deviceState->colorModeFactor, this->deviceState->colorModeFactor);
			unsigned char* mappedConstantBuffer = this->mappedConstantBuffer + (this->currentFrame * ALIGNED_CONSTANT_BUFFER_SIZE);
			memcpy(mappedConstantBuffer, &this->constantBufferData, sizeof(ConstantBuffer));
			this->deviceState_constantBufferChanged = false;
		}
	}

	void DirectX12_RenderSystem::executeCurrentCommands()
	{
		PIXEndEvent(this->commandList.Get());
		this->commandList->Close();
		ID3D12CommandList* ppCommandLists[] = { this->commandList.Get() };
		this->commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
	}

	void DirectX12_RenderSystem::waitForCommands()
	{
		const UINT64 currentFenceValue = this->fenceValues[this->currentFrame];
		_TRY_UNSAFE(this->commandQueue->Signal(this->fence.Get(), currentFenceValue), "Unable to signal command queue!");
		this->currentFrame = this->swapChain->GetCurrentBackBufferIndex();
		if (this->fence->GetCompletedValue() < this->fenceValues[this->currentFrame])
		{
			_TRY_UNSAFE(this->fence->SetEventOnCompletion(this->fenceValues[this->currentFrame], this->fenceEvent), "Unable to set even on completion!");
			WaitForSingleObjectEx(this->fenceEvent, INFINITE, FALSE);
		}
		this->fenceValues[this->currentFrame] = currentFenceValue + 1;
		this->vertexBufferIndex = 0;
	}

	void DirectX12_RenderSystem::prepareNewCommands()
	{
		_TRY_UNSAFE(this->commandAllocators[this->currentFrame]->Reset(), hsprintf("Unable to reset command allocator %d!", this->currentFrame));
		_TRY_UNSAFE(this->commandList->Reset(this->commandAllocators[this->currentFrame].Get(), this->deviceState_pipelineState.Get()), "Unable to reset command list!");
		PIXBeginEvent(this->commandList.Get(), 0, L"");
		this->commandList->SetGraphicsRootSignature(this->deviceState_rootSignature.Get());
		CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle;
		if (this->deviceState->useTexture && this->deviceState->texture != NULL)
		{
			ID3D12DescriptorHeap* heaps[] = { this->cbvSrvUavHeap.Get(), this->samplerHeap.Get() };
			this->commandList->SetDescriptorHeaps(_countof(heaps), heaps);
			gpuHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(this->cbvSrvUavHeap->GetGPUDescriptorHandleForHeapStart(), this->currentFrame, this->cbvSrvUavDescSize);
			this->commandList->SetGraphicsRootDescriptorTable(0, gpuHandle);
			// texture
			april::DirectX12_Texture* texture = (april::DirectX12_Texture*)this->deviceState->texture;
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.Format = texture->dxgiFormat;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MipLevels = 1;
			int heapIndex = BACKBUFFER_COUNT + this->currentFrame;
			CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle(this->cbvSrvUavHeap->GetCPUDescriptorHandleForHeapStart(), heapIndex, this->cbvSrvUavDescSize);
			this->d3dDevice->CreateShaderResourceView(texture->d3dTexture.Get(), &srvDesc, cpuHandle);
			gpuHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(this->cbvSrvUavHeap->GetGPUDescriptorHandleForHeapStart(), heapIndex, this->cbvSrvUavDescSize);
			this->commandList->SetGraphicsRootDescriptorTable(1, gpuHandle);
			// sampler
			int adressModeSize = Texture::AddressMode::getValues().size();
			gpuHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(this->samplerHeap->GetGPUDescriptorHandleForHeapStart(), texture->getFilter().value * adressModeSize + texture->getAddressMode().value, this->samplerDescSize);
			this->commandList->SetGraphicsRootDescriptorTable(2, gpuHandle);
		}
		else
		{
			ID3D12DescriptorHeap* heaps[] = { this->cbvSrvUavHeap.Get() };
			this->commandList->SetDescriptorHeaps(_countof(heaps), heaps);
			gpuHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(this->cbvSrvUavHeap->GetGPUDescriptorHandleForHeapStart(), this->currentFrame, this->cbvSrvUavDescSize);
			this->commandList->SetGraphicsRootDescriptorTable(0, gpuHandle);
		}
	}

	void DirectX12_RenderSystem::_renderDX12VertexBuffer(const RenderOperation& renderOperation, const void* data, int count, unsigned int vertexSize)
	{
		this->_updatePipelineState(renderOperation);
		D3D12_SUBRESOURCE_DATA vertexData = {};
		vertexData.pData = data;
		vertexData.RowPitch = (count * vertexSize);
		vertexData.SlicePitch = vertexData.RowPitch;
		UpdateSubresources(this->commandList.Get(), this->vertexBuffers[this->vertexBufferIndex].Get(), this->vertexBufferUploads[this->vertexBufferIndex].Get(), 0, 0, 1, &vertexData);
		
		D3D12_RESOURCE_BARRIER vertexBufferResourceBarrier = {};
		vertexBufferResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		vertexBufferResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        vertexBufferResourceBarrier.Transition.pResource = this->vertexBuffers[this->vertexBufferIndex].Get();
        vertexBufferResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
        vertexBufferResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
        vertexBufferResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		this->commandList->ResourceBarrier(1, &vertexBufferResourceBarrier);

		D3D12_CPU_DESCRIPTOR_HANDLE renderTargetView = this->rtvHeap->GetCPUDescriptorHandleForHeapStart();
		renderTargetView.ptr += this->currentFrame * this->rtvDescSize;
		grecti viewport = this->getViewport();
		// TODOuwp - this used to be needed on WinRT because of a graphics driver bug on Windows RT and on WinP8 because of a completely different graphics driver bug on Windows Phone 8
		// maybe it will be needed for UWP as well
		/*
		gvec2i resolution = april::getSystemInfo().displayResolution;
		int w = april::window->getWidth();
		int h = april::window->getHeight();
		if (viewport.x < 0)
		{
			viewport.w += viewport.x;
			viewport.x = 0;
		}
		if (viewport.y < 0)
		{
			viewport.h += viewport.y;
			viewport.y = 0;
		}
		viewport.w = hclamp(viewport.w, 0, hmax(w - viewport.x, 0));
		viewport.h = hclamp(viewport.h, 0, hmax(h - viewport.y, 0));
		if (viewport.w > 0 && viewport.h > 0)
		{
			viewport.x = hclamp(viewport.x, 0, w);
			viewport.y = hclamp(viewport.y, 0, h);
		}
		else
		{
			viewport.set(w, h, 0, 0);
		}
		*/
		// setting the system viewport
		D3D12_VIEWPORT dx12Viewport;
		dx12Viewport.MinDepth = D3D12_MIN_DEPTH;
		dx12Viewport.MaxDepth = D3D12_MAX_DEPTH;
		// these double-casts are to ensure consistent behavior among rendering systems
		dx12Viewport.TopLeftX = (float)viewport.x;
		dx12Viewport.TopLeftY = (float)viewport.y;
		dx12Viewport.Width = (float)viewport.w;
		dx12Viewport.Height = (float)viewport.h;
		this->commandList->RSSetViewports(1, &dx12Viewport);
		D3D12_RECT scissorRect = { (LONG)viewport.x, (LONG)viewport.y, (LONG)viewport.w, (LONG)viewport.h };
		this->commandList->RSSetScissorRects(1, &scissorRect);

		// TODOuwp - maybe could be removed / disabled as depth buffers aren't supported widely in april
		/*
		D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView = this->dsvHeap->GetCPUDescriptorHandleForHeapStart();
		this->commandList->OMSetRenderTargets(1, &renderTargetView, false, &depthStencilView);
		*/
		this->commandList->OMSetRenderTargets(1, &renderTargetView, false, NULL);
		
		this->commandList->IASetPrimitiveTopology(_dx12RenderOperations[renderOperation.value]);
		this->vertexBufferViews[this->vertexBufferIndex].BufferLocation = this->vertexBuffers[this->vertexBufferIndex]->GetGPUVirtualAddress();
		this->vertexBufferViews[this->vertexBufferIndex].StrideInBytes = vertexSize;
		this->vertexBufferViews[this->vertexBufferIndex].SizeInBytes = count * vertexSize;

		this->commandList->IASetVertexBuffers(0, 1, &this->vertexBufferViews[this->vertexBufferIndex]);
		this->commandList->DrawInstanced(count, 1, 0, 0);

		this->vertexBufferIndex = (this->vertexBufferIndex + 1) % MAX_VERTEX_BUFFERS;
	}

	Image::Format DirectX12_RenderSystem::getNativeTextureFormat(Image::Format format) const
	{
		if (format == Image::Format::RGBA || format == Image::Format::ARGB || format == Image::Format::BGRA || format == Image::Format::ABGR)
		{
			return Image::Format::RGBA;
		}
		if (format == Image::Format::RGBX || format == Image::Format::XRGB || format == Image::Format::BGRX ||
			format == Image::Format::XBGR || format == Image::Format::RGB || format == Image::Format::BGR)
		{
			return Image::Format::RGBX;
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
	
	void DirectX12_RenderSystem::_devicePresentFrame(bool systemEnabled)
	{
		RenderSystem::_devicePresentFrame(systemEnabled);
		D3D12_RESOURCE_BARRIER presentResourceBarrier = {};
		presentResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		presentResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		presentResourceBarrier.Transition.pResource = this->renderTargets[this->currentFrame].Get();
		presentResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		presentResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
		presentResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		this->commandList->ResourceBarrier(1, &presentResourceBarrier);
		this->executeCurrentCommands();
		// TODOuwp - to support disabled vsync properly, this might have to be this->swapChain->Present(0, 0)
		HRESULT hr = this->swapChain->Present(1, 0);
		if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
		{
			// TODOuwp - handle this properly
			//m_deviceRemoved = true;
			//this->updateDeviceReset();
			return;
		}
		_TRY_UNSAFE(hr, "Unable to present swap chain!");
		this->deviceState_rootSignature = (!this->deviceState->useTexture ? this->rootSignatures[0] : this->rootSignatures[1]);
		this->waitForCommands();
		this->prepareNewCommands();
		D3D12_RESOURCE_BARRIER renderTargetResourceBarrier = {};
		renderTargetResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		renderTargetResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		renderTargetResourceBarrier.Transition.pResource = this->renderTargets[this->currentFrame].Get();
		renderTargetResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
		renderTargetResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		renderTargetResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		this->commandList->ResourceBarrier(1, &renderTargetResourceBarrier);
	}

	void DirectX12_RenderSystem::_deviceRepeatLastFrame()
	{
		//DirectX_RenderSystem::_deviceRepeatLastFrame();
	}

	void DirectX12_RenderSystem::_deviceCopyRenderTargetData(Texture* source, Texture* destination)
	{

	}

	void DirectX12_RenderSystem::updateDeviceReset()
	{
		// TODOuwp - remove this or use it in
		/*
		if (this->deviceRemoved)
		{
			// do stuff
		}
		*/
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
