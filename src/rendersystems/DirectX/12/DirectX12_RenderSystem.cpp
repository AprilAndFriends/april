/// @file
/// @version 5.2
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
#define VERTEX_BUFFER_SIZE 262144 // 256kb per vertex buffer for data is enough to handle most render data that is used
#define CBV_SRV_UAV_HEAP_SIZE 2
#define SAMPLER_COUNT (Texture::Filter::getCount() * Texture::AddressMode::getCount())

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
				char message[1024] = { 0 };
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
		D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST,	// RenderOperation::TriangleList
		D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP,	// RenderOperation::TriangleStrip
		D3D_PRIMITIVE_TOPOLOGY_LINELIST,		// RenderOperation::ListList
		D3D_PRIMITIVE_TOPOLOGY_LINESTRIP,		// RenderOperation::LineStrip
		D3D_PRIMITIVE_TOPOLOGY_POINTLIST,		// RenderOperation::PointList
		D3D_PRIMITIVE_TOPOLOGY_UNDEFINED,		// triangle fans are deprecated in DX12
	};

	DirectX12_RenderSystem::DirectX12_RenderSystem() : DirectX_RenderSystem()
	{
		this->name = april::RenderSystemType::DirectX12.getName();
		this->vertexBufferIndex = 0;
		this->commandListIndex = 0;
		this->commandListSize = 1;
	}

	DirectX12_RenderSystem::~DirectX12_RenderSystem()
	{
		this->destroy();
	}

	int DirectX12_RenderSystem::_getBackbufferCount() const
	{
		return (this->options.tripleBuffering ? 3 : 2);
	}

	void DirectX12_RenderSystem::_deviceInit()
	{
		this->d3dDevice = nullptr;
		this->swapChain = nullptr;
		for_iter (i, 0, MAX_VERTEX_BUFFERS)
		{
			this->vertexBuffers[i] = nullptr;
		}
		for_iter (i, 0, MAX_COMMAND_LISTS)
		{
			this->constantBuffers[i] = nullptr;
		}
		this->vertexShaderPlain = NULL;
		this->vertexShaderTextured = NULL;
		this->vertexShaderColored = NULL;
		this->vertexShaderColoredTextured = NULL;
		this->pixelShaderMultiply = NULL;
		this->pixelShaderAlphaMap = NULL;
		this->pixelShaderLerp = NULL;
		this->pixelShaderDesaturate = NULL;
		this->pixelShaderSepia = NULL;
		this->pixelShaderTexturedMultiply = NULL;
		this->pixelShaderTexturedAlphaMap = NULL;
		this->pixelShaderTexturedLerp = NULL;
		this->pixelShaderTexturedDesaturate = NULL;
		this->pixelShaderTexturedSepia = NULL;
		this->deviceState_constantBufferChanged = true;
		this->deviceViewport.MinDepth = D3D12_MIN_DEPTH;
		this->deviceViewport.MaxDepth = D3D12_MAX_DEPTH;
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
		_HL_TRY_DELETE(this->pixelShaderDesaturate);
		_HL_TRY_DELETE(this->pixelShaderSepia);
		_HL_TRY_DELETE(this->pixelShaderTexturedMultiply);
		_HL_TRY_DELETE(this->pixelShaderTexturedAlphaMap);
		_HL_TRY_DELETE(this->pixelShaderTexturedLerp);
		_HL_TRY_DELETE(this->pixelShaderTexturedDesaturate);
		_HL_TRY_DELETE(this->pixelShaderTexturedSepia);
		this->setViewport(grecti(0, 0, april::getSystemInfo().displayResolution));
		return true;
	}

	void DirectX12_RenderSystem::_deviceAssignWindow(Window* window)
	{
		this->_createD3dDevice();
		this->_createHeapDescriptors();
		// commands
		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		queueDesc.NodeMask = 0;
		_TRY_UNSAFE(this->d3dDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&this->commandQueue)), "Unable to create command queue!");
		int backbufferCount = this->_getBackbufferCount();
		for_iter (j, 0, MAX_COMMAND_LISTS)
		{
			for_iter (i, 0, backbufferCount)
			{
				_TRY_UNSAFE(this->d3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&this->commandAllocators[j][i])), hsprintf("Unable to create command allocator %d!", i));
			}
		}
		// create synchronization objects
		_TRY_UNSAFE(this->d3dDevice->CreateFence(this->fenceLimits[this->currentFrame], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&this->fence)), "Unable to create fence");
		++this->fenceLimits[this->currentFrame];
		++this->fenceValues[this->currentFrame];
		this->fenceEvent = CreateEventEx(nullptr, FALSE, FALSE, EVENT_ALL_ACCESS);
		// configure device
		this->coreWindow = CoreWindow::GetForCurrentThread();
		DisplayInformation^ displayInformation = DisplayInformation::GetForCurrentView();
		this->nativeOrientation = displayInformation->NativeOrientation;
		this->logicalSize = Windows::Foundation::Size((float)window->getWidth(), (float)window->getHeight());
		this->currentOrientation = displayInformation->CurrentOrientation;
		this->dpi = displayInformation->LogicalDpi;
		this->_setupSwapChain();
		this->_createRootSignatures();
		this->deviceState_rootSignature = this->rootSignatures[0];
		this->_createShaders();
		this->_createPipeline();
		this->deviceState_pipelineState = this->pipelineStates[0][0][0][0][0];
		for_iter (i, 0, MAX_COMMAND_LISTS)
		{
			_TRY_UNSAFE(this->d3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, this->commandAllocators[i][this->currentFrame].Get(),
				this->deviceState_pipelineState.Get(), IID_PPV_ARGS(&this->commandList[i])), "Unable to create command allocators state!");
		}
		// vertex bufffers
		this->uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		this->uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
		this->uploadHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		this->uploadHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		this->uploadHeapProperties.CreationNodeMask = 1;
		this->uploadHeapProperties.VisibleNodeMask = 1;
		this->vertexBufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		this->vertexBufferDesc.Alignment = 0;
		this->vertexBufferDesc.Width = VERTEX_BUFFER_SIZE;
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
				D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&this->vertexBufferUploads[i])), "Unable to create vertex buffer upload heap!");
		}
		// constant buffer
		this->constantBufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		this->constantBufferDesc.Alignment = 0;
		this->constantBufferDesc.Width = backbufferCount * ALIGNED_CONSTANT_BUFFER_SIZE;
		this->constantBufferDesc.Height = 1;
		this->constantBufferDesc.DepthOrArraySize = 1;
		this->constantBufferDesc.MipLevels = 1;
		this->constantBufferDesc.Format = DXGI_FORMAT_UNKNOWN;
		this->constantBufferDesc.SampleDesc.Count = 1;
		this->constantBufferDesc.SampleDesc.Quality = 0;
		this->constantBufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		this->constantBufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
		for_iter (j, 0, MAX_COMMAND_LISTS)
		{
			_TRY_UNSAFE(this->d3dDevice->CreateCommittedResource(&this->uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &this->constantBufferDesc,
				D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&this->constantBuffers[j])), "Unable to create constant buffer!");
			// constant buffer views
			D3D12_GPU_VIRTUAL_ADDRESS cbvGpuAddress = this->constantBuffers[j]->GetGPUVirtualAddress();
			D3D12_CPU_DESCRIPTOR_HANDLE cbvCpuHandle = this->cbvSrvUavHeaps[j]->GetCPUDescriptorHandleForHeapStart();
			D3D12_CONSTANT_BUFFER_VIEW_DESC desc;
			desc.SizeInBytes = ALIGNED_CONSTANT_BUFFER_SIZE;
			for_iter (i, 0, backbufferCount)
			{
				desc.BufferLocation = cbvGpuAddress;
				this->d3dDevice->CreateConstantBufferView(&desc, cbvCpuHandle);
				cbvGpuAddress += desc.SizeInBytes;
				cbvCpuHandle.ptr += this->cbvSrvUavDescSize;
			}
			D3D12_RANGE readRange = {};
			readRange.Begin = 0;
			readRange.End = 0;
			this->constantBuffers[j]->Map(0, &readRange, reinterpret_cast<void**>(&this->mappedConstantBuffers[j]));
		}
		// finish and execute these commands
		harray<ID3D12CommandList*> commandLists;
		for_iter (i, 0, MAX_COMMAND_LISTS)
		{
			_TRY_UNSAFE(this->commandList[i]->Close(), "Unable to close command list!");
			commandLists += this->commandList[i].Get();
		}
		this->commandQueue->ExecuteCommandLists(commandLists.size(), (ID3D12CommandList* const*)commandLists);
		this->_waitForGpu();
		// initial calls
		grecti viewport(0, 0, window->getSize());
		gvec2f windowSizeFloat((float)viewport.w, (float)viewport.h);
		_TRY_UNSAFE(this->commandAllocators[this->commandListIndex][this->currentFrame]->Reset(), hsprintf("Unable to reset command allocator %d!", this->currentFrame));
		_TRY_UNSAFE(this->commandList[this->commandListIndex]->Reset(this->commandAllocators[this->commandListIndex][this->currentFrame].Get(), this->deviceState_pipelineState.Get()), "Unable to reset command list!");
		this->commandList[this->commandListIndex]->Close();
		ID3D12CommandList* ppCommandLists[] = { this->commandList[this->commandListIndex].Get() };
		this->commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
		_TRY_UNSAFE(this->swapChain->Present(1, 0), "Unable to present initial swap chain!");
		this->deviceState_rootSignature = this->rootSignatures[0];
		this->waitForAllCommands();
		this->prepareNewCommands();
		D3D12_RESOURCE_BARRIER renderTargetResourceBarrier = {};
		renderTargetResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		renderTargetResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		renderTargetResourceBarrier.Transition.pResource = this->renderTargets[this->currentFrame].Get();
		renderTargetResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
		renderTargetResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		renderTargetResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		this->commandList[this->commandListIndex]->ResourceBarrier(1, &renderTargetResourceBarrier);
		this->setOrthoProjection(windowSizeFloat);
		this->setViewport(viewport);
	}

	void DirectX12_RenderSystem::_createD3dDevice()
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
			ComPtr<IDXGIFactory5> dxgiFactory5 = nullptr;
			if (SUCCEEDED(this->dxgiFactory.As(&dxgiFactory5)) && dxgiFactory5 != nullptr)
			{
				// must use BOOL here due to how the internal implementation of CheckFeatureSupport() works
				BOOL allowTearing = FALSE;
				if (SUCCEEDED(dxgiFactory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing))))
				{
					if (allowTearing == FALSE)
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
		D3D_FEATURE_LEVEL availableFeatureLevels[] = { D3D_FEATURE_LEVEL_12_1, D3D_FEATURE_LEVEL_12_0, D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0 };
		D3D_FEATURE_LEVEL featureLevel = availableFeatureLevels[0];
		for_iter (i, 0, _countof(availableFeatureLevels))
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
		HRESULT hr = S_OK;
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
		// TODOuwp - is this even needed?
		/*
		if (!this->options.depthBuffer)
		{
			D3D12_FEATURE_DATA_D3D12_OPTIONS2 d3dOptions = {};
			if (SUCCEEDED(this->d3dDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS2, &d3dOptions, sizeof(d3dOptions))))
			{
				if (!d3dOptions.DepthBoundsTestSupported)
				{
					hlog::warn(logTag, "Cannot enable Depth-Buffer, D3D device says DepthBoundsTestSupported is not supported!");
					this->options.depthBuffer = true;
				}
			}
			else
			{
				hlog::warn(logTag, "Cannot enable Depth-Buffer, D3D device is unable to check for support!");
				this->options.depthBuffer = true;
			}
		}
		*/
#ifdef _DEBUG
		ComPtr<ID3D12InfoQueue> pInfoQueue;
		if (SUCCEEDED(this->d3dDevice.As(&pInfoQueue)))
		{
			pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
			pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
			pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);
		}
#endif
	}

	void DirectX12_RenderSystem::_createHeapDescriptors()
	{
		int backbufferCount = this->_getBackbufferCount();
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
		rtvHeapDesc.NumDescriptors = backbufferCount;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		_TRY_UNSAFE(this->d3dDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&this->rtvHeap)), "Unable to create RTV heap!");
		this->rtvHeap->SetName(L"RTV Heap");
		this->rtvDescSize = this->d3dDevice->GetDescriptorHandleIncrementSize(rtvHeapDesc.Type);
		D3D12_DESCRIPTOR_HEAP_DESC cbvSrvUavHeapDesc = {};
		cbvSrvUavHeapDesc.NumDescriptors = backbufferCount * CBV_SRV_UAV_HEAP_SIZE;
		cbvSrvUavHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		cbvSrvUavHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		D3D12_DESCRIPTOR_HEAP_DESC samplerHeapDesc = {};
		samplerHeapDesc.NumDescriptors = SAMPLER_COUNT;
		samplerHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
		samplerHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		for_iter (i, 0, MAX_COMMAND_LISTS)
		{
			_TRY_UNSAFE(this->d3dDevice->CreateDescriptorHeap(&cbvSrvUavHeapDesc, IID_PPV_ARGS(&this->cbvSrvUavHeaps[i])), "Unable to create CBV heap!");
			this->cbvSrvUavHeaps[i]->SetName(hstr("CBV Heap " + hstr(i)).wStr().c_str());
			_TRY_UNSAFE(this->d3dDevice->CreateDescriptorHeap(&samplerHeapDesc, IID_PPV_ARGS(&this->samplerHeaps[i])), "Unable to create sampler heap!");
			this->samplerHeaps[i]->SetName(hstr("Sampler Heap " + hstr(i)).wStr().c_str());
		}
		this->cbvSrvUavDescSize = this->d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		this->samplerDescSize = this->d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
		// depth stencil view
		D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
		dsvHeapDesc.NumDescriptors = 1;
		dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		_TRY_UNSAFE(this->d3dDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&this->dsvHeap)), "Unable to create DSV heap!");
		this->dsvHeap->SetName(L"DSV Heap");
		this->dsvDescSize = this->d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	}

	void DirectX12_RenderSystem::_setupSwapChain()
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
		swapChainDesc.BufferCount = this->_getBackbufferCount();
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
		this->executeCurrentCommand();
		this->waitForAllCommands();
		this->_waitForGpu();
		int backbufferCount = this->_getBackbufferCount();
		for_iter (i, 0, backbufferCount)
		{
			this->renderTargets[i] = nullptr;
			this->fenceValues[i] = this->fenceLimits[i] = this->fenceLimits[this->currentFrame];
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
		D3D12_RESOURCE_BARRIER renderTargetResourceBarrier = {};
		renderTargetResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		renderTargetResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		renderTargetResourceBarrier.Transition.pResource = this->renderTargets[this->currentFrame].Get();
		renderTargetResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
		renderTargetResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		renderTargetResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		this->commandList[this->commandListIndex]->ResourceBarrier(1, &renderTargetResourceBarrier);
	}

	void DirectX12_RenderSystem::_configureSwapChain(int width, int height)
	{
		_TRY_UNSAFE(this->swapChain->SetRotation(this->_getDxgiRotation()), "Unable to set rotation on swap chain!");
		this->currentFrame = this->swapChain->GetCurrentBackBufferIndex();
		D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = this->rtvHeap->GetCPUDescriptorHandleForHeapStart();
		int backbufferCount = this->_getBackbufferCount();
		for_iter (i, 0, backbufferCount)
		{
			_TRY_UNSAFE(this->swapChain->GetBuffer(i, IID_PPV_ARGS(&this->renderTargets[i])), hsprintf("Unable to get buffer %d from swap chain!", i));
			this->d3dDevice->CreateRenderTargetView(this->renderTargets[i].Get(), nullptr, cpuHandle);
			this->renderTargets[i]->SetName(("Render Target " + hstr(i)).wStr().c_str());
			cpuHandle.ptr += this->rtvDescSize;
		}
		if (this->options.depthBuffer)
		{
			CD3DX12_HEAP_PROPERTIES depthHeapProperties(D3D12_HEAP_TYPE_DEFAULT);
			CD3DX12_RESOURCE_DESC depthResourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, width, height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
			D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
			depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
			depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
			depthOptimizedClearValue.DepthStencil.Stencil = 0;
			_TRY_UNSAFE(this->d3dDevice->CreateCommittedResource(&depthHeapProperties, D3D12_HEAP_FLAG_NONE, &depthResourceDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE,
				&depthOptimizedClearValue, IID_PPV_ARGS(&this->depthStencil)), "Unable to create depth buffer!");
			D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
			dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
			dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
			dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
			this->d3dDevice->CreateDepthStencilView(this->depthStencil.Get(), &dsvDesc, this->dsvHeap->GetCPUDescriptorHandleForHeapStart());
		}
		this->setViewport(grecti(0, 0, width, height));
	}

	void DirectX12_RenderSystem::_createRootSignatures()
	{
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

	void DirectX12_RenderSystem::_createShaders()
	{
		// default shaders
		LOAD_SHADER(this->vertexShaderPlain, Vertex, Plain);
		LOAD_SHADER(this->vertexShaderTextured, Vertex, Textured);
		LOAD_SHADER(this->vertexShaderColored, Vertex, Colored);
		LOAD_SHADER(this->vertexShaderColoredTextured, Vertex, ColoredTextured);
		LOAD_SHADER(this->pixelShaderMultiply, Pixel, Multiply);
		LOAD_SHADER(this->pixelShaderAlphaMap, Pixel, AlphaMap);
		LOAD_SHADER(this->pixelShaderLerp, Pixel, Lerp);
		LOAD_SHADER(this->pixelShaderDesaturate, Pixel, Desaturate);
		LOAD_SHADER(this->pixelShaderSepia, Pixel, Sepia);
		LOAD_SHADER(this->pixelShaderTexturedMultiply, Pixel, TexturedMultiply);
		LOAD_SHADER(this->pixelShaderTexturedAlphaMap, Pixel, TexturedAlphaMap);
		LOAD_SHADER(this->pixelShaderTexturedLerp, Pixel, TexturedLerp);
		LOAD_SHADER(this->pixelShaderTexturedDesaturate, Pixel, TexturedDesaturate);
		LOAD_SHADER(this->pixelShaderTexturedSepia, Pixel, TexturedSepia);
		// texture samplers
		D3D12_SAMPLER_DESC samplerDesc;
		memset(&samplerDesc, 0, sizeof(samplerDesc));
		samplerDesc.MaxAnisotropy = 0;
		samplerDesc.MipLODBias = 0.0f;
		samplerDesc.MinLOD = 0.0f;
		samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
		samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		samplerDesc.BorderColor[0] = 0.0f;
		samplerDesc.BorderColor[1] = 0.0f;
		samplerDesc.BorderColor[2] = 0.0f;
		samplerDesc.BorderColor[3] = 0.0f;
		for_iter (i, 0, MAX_COMMAND_LISTS)
		{
			D3D12_CPU_DESCRIPTOR_HANDLE samplerCpuHandle = this->samplerHeaps[i]->GetCPUDescriptorHandleForHeapStart();
			int filterSize = Texture::Filter::getCount();
			int adressModeSize = Texture::AddressMode::getCount();
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
		}
	}

	void DirectX12_RenderSystem::_createPipeline()
	{
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
		this->pixelShaders += this->pixelShaderDesaturate;
		this->pixelShaders += this->pixelShaderSepia;
		this->pixelShaders += this->pixelShaderTexturedMultiply;
		this->pixelShaders += this->pixelShaderTexturedAlphaMap;
		this->pixelShaders += this->pixelShaderTexturedLerp;
		this->pixelShaders += this->pixelShaderTexturedDesaturate;
		this->pixelShaders += this->pixelShaderTexturedSepia;
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
		state.DSVFormat = DXGI_FORMAT_D32_FLOAT;
		state.SampleMask = UINT_MAX;
		state.NumRenderTargets = 1;
		state.SampleDesc.Count = 1;
		state.SampleDesc.Quality = 0;
		state.BlendState.AlphaToCoverageEnable = false;
		state.BlendState.IndependentBlendEnable = false;
		state.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
		state.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL; // using the slower less-equal so multiple overlays can be rendered properly
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
					for_iter (l, 0, D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT)
					{
						state.BlendState.RenderTarget[l] = this->blendStateRenderTargets[k];
					}
					for_iter (l, 0, this->primitiveTopologyTypes.size())
					{
						state.PrimitiveTopologyType = this->primitiveTopologyTypes[l];
						state.DepthStencilState.DepthEnable = false;
						_TRY_UNSAFE(this->d3dDevice->CreateGraphicsPipelineState(&state, IID_PPV_ARGS(&this->pipelineStates[i][j][k][l][0])), "Unable to create graphics pipeline state!");
						state.DepthStencilState.DepthEnable = true;
						_TRY_UNSAFE(this->d3dDevice->CreateGraphicsPipelineState(&state, IID_PPV_ARGS(&this->pipelineStates[i][j][k][l][1])), "Unable to create graphics pipeline state!");
					}
				}
			}
		}
	}

	void DirectX12_RenderSystem::_deviceReset()
	{
		DirectX_RenderSystem::_deviceReset();
		// TODOuwp - implement this
		// possible Microsoft bug, required for SwapChainPanel to update its layout 
		//reinterpret_cast<IUnknown*>(UWP::App->Overlay)->QueryInterface(IID_PPV_ARGS(&this->swapChainNative));
		//this->swapChainNative->SetSwapChain(this->swapChain.Get());
	}

	void DirectX12_RenderSystem::_deviceSetupCaps()
	{
		this->caps.maxTextureSize = D3D_FL9_3_REQ_TEXTURE1D_U_DIMENSION;
		this->caps.npotTexturesLimited = true;
		this->caps.npotTextures = true;
	}

	void DirectX12_RenderSystem::_deviceSetup()
	{
		// not used
	}

	// Wait for pending GPU work to complete.
	void DirectX12_RenderSystem::_waitForGpu()
	{
		HRESULT hr = this->commandQueue->Signal(this->fence.Get(), this->fenceLimits[this->currentFrame]);
		if (FAILED(hr))
		{
			throw Exception("Could not Signal command queue!");
		}
		hr = this->fence->SetEventOnCompletion(this->fenceLimits[this->currentFrame], this->fenceEvent);
		if (FAILED(hr))
		{
			throw Exception("Could not Signal command queue!");
		}
		WaitForSingleObjectEx(this->fenceEvent, INFINITE, FALSE);
		++this->fenceLimits[this->currentFrame];
		this->fenceValues[this->currentFrame] = this->fenceLimits[this->currentFrame];
		this->commandListIndex = 0;
		this->commandListSize = 1;
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
			this->_setupSwapChain();
		}
	}

	void DirectX12_RenderSystem::_setDeviceViewport(cgrecti rect)
	{
		this->deviceViewport.TopLeftX = (float)rect.x;
		this->deviceViewport.TopLeftY = (float)rect.y;
		this->deviceViewport.Width = (float)rect.w;
		this->deviceViewport.Height = (float)rect.h;
		this->deviceScissorRect.left = (LONG)rect.x;
		this->deviceScissorRect.top = (LONG)rect.y;
		this->deviceScissorRect.right = (LONG)rect.right();
		this->deviceScissorRect.bottom = (LONG)rect.bottom();
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
		/*
		if (enabled || writeEnabled)
		{
			hlog::error(logTag, "_setDeviceDepthBuffer() is not implemented in: " + this->name);
		}
		*/
		// not used
	}

	void DirectX12_RenderSystem::_setDeviceRenderMode(bool useTexture, bool useColor)
	{
		// not used
	}

	void DirectX12_RenderSystem::_setDeviceTexture(Texture* texture)
	{
		// not used
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
		this->commandList[this->commandListIndex]->ClearRenderTargetView(cpuHandle, clearColor, 0, nullptr);
		if (depth)
		{
			this->_deviceClearDepth();
		}
	}
	
	void DirectX12_RenderSystem::_deviceClear(const Color& color, bool depth)
	{
		static float clearColor[4] = {0.0f, 0.0f, 0.0f, 0.0f};
		clearColor[0] = color.b_f();
		clearColor[1] = color.g_f();
		clearColor[2] = color.r_f();
		clearColor[3] = color.a_f();
		CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle(this->rtvHeap->GetCPUDescriptorHandleForHeapStart(), this->currentFrame, this->rtvDescSize);
		this->commandList[this->commandListIndex]->ClearRenderTargetView(cpuHandle, clearColor, 0, nullptr);
		if (depth)
		{
			this->_deviceClearDepth();
		}
	}

	void DirectX12_RenderSystem::_deviceClearDepth()
	{
		if (this->options.depthBuffer)
		{
			this->commandList[this->commandListIndex]->ClearDepthStencilView(this->dsvHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
		}
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
		this->deviceState_pipelineState = this->pipelineStates[i][j][k][l][m];
		this->deviceState_rootSignature = this->rootSignatures[r];
		this->executeCurrentCommand();
		this->commandListIndex = (this->commandListIndex + 1) % MAX_COMMAND_LISTS;
		if (this->commandListSize >= MAX_COMMAND_LISTS)
		{
			this->waitForLastCommand();
		}
		else
		{
			++this->commandListSize;
		}
		this->prepareNewCommands();
		if (this->deviceState_constantBufferChanged)
		{
			this->constantBufferData.matrix = (this->deviceState->projectionMatrix * this->deviceState->modelviewMatrix).transposed();
			this->constantBufferData.systemColor.set(this->deviceState->systemColor.r_f(), this->deviceState->systemColor.g_f(),
				this->deviceState->systemColor.b_f(), this->deviceState->systemColor.a_f());
			this->constantBufferData.lerpAlpha.set(this->deviceState->colorModeFactor, this->deviceState->colorModeFactor,
				this->deviceState->colorModeFactor, this->deviceState->colorModeFactor);
			this->deviceState_constantBufferChanged = false;
		}
		unsigned char* mappedConstantBuffer = this->mappedConstantBuffers[this->commandListIndex] + (this->currentFrame * ALIGNED_CONSTANT_BUFFER_SIZE);
		memcpy(mappedConstantBuffer, &this->constantBufferData, sizeof(ConstantBuffer));
	}

	void DirectX12_RenderSystem::executeCurrentCommand()
	{
		PIXEndEvent(this->commandList[this->commandListIndex].Get());
		this->commandList[this->commandListIndex]->Close();
		ID3D12CommandList* ppCommandLists[] = { this->commandList[this->commandListIndex].Get() };
		this->commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
		// signal queue to make sure it continues processing lists
		const UINT64 currentFenceValue = this->fenceLimits[this->currentFrame];
		_TRY_UNSAFE(this->commandQueue->Signal(this->fence.Get(), currentFenceValue), "Unable to signal command queue!");
		this->currentFrame = this->swapChain->GetCurrentBackBufferIndex();
		this->fenceLimits[this->currentFrame] = currentFenceValue + 1;
	}

	void DirectX12_RenderSystem::waitForAllCommands()
	{
		const UINT64 currentFenceValue = this->fenceLimits[this->currentFrame];
		_TRY_UNSAFE(this->commandQueue->Signal(this->fence.Get(), currentFenceValue), "Unable to signal command queue!");
		this->currentFrame = this->swapChain->GetCurrentBackBufferIndex();
		if (this->fence->GetCompletedValue() < this->fenceLimits[this->currentFrame])
		{
			_TRY_UNSAFE(this->fence->SetEventOnCompletion(this->fenceLimits[this->currentFrame], this->fenceEvent), "Unable to set event on completion!");
			WaitForSingleObjectEx(this->fenceEvent, INFINITE, FALSE);
		}
		this->fenceValues[this->currentFrame] = this->fenceLimits[this->currentFrame] = currentFenceValue + 1;
		this->vertexBufferIndex = 0;
		this->commandListIndex = 0;
		this->commandListSize = 1;
	}

	void DirectX12_RenderSystem::waitForLastCommand()
	{
		const UINT64 currentFenceValue = this->fenceValues[this->currentFrame];
		_TRY_UNSAFE(this->commandQueue->Signal(this->fence.Get(), currentFenceValue), "Unable to signal command queue!");
		this->currentFrame = this->swapChain->GetCurrentBackBufferIndex();
		if (this->fence->GetCompletedValue() < this->fenceValues[this->currentFrame])
		{
			_TRY_UNSAFE(this->fence->SetEventOnCompletion(this->fenceValues[this->currentFrame], this->fenceEvent), "Unable to set event on completion!");
			WaitForSingleObjectEx(this->fenceEvent, INFINITE, FALSE);
		}
		this->fenceValues[this->currentFrame] = currentFenceValue + 1;
	}

	void DirectX12_RenderSystem::prepareNewCommands()
	{
		_TRY_UNSAFE(this->commandAllocators[this->commandListIndex][this->currentFrame]->Reset(), hsprintf("Unable to reset command allocator %d!", this->currentFrame));
		_TRY_UNSAFE(this->commandList[this->commandListIndex]->Reset(this->commandAllocators[this->commandListIndex][this->currentFrame].Get(), this->deviceState_pipelineState.Get()), "Unable to reset command list!");
		PIXBeginEvent(this->commandList[this->commandListIndex].Get(), 0, L"");
		this->commandList[this->commandListIndex]->SetGraphicsRootSignature(this->deviceState_rootSignature.Get());
		CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle;
		if (this->deviceState->useTexture && this->deviceState->texture != NULL)
		{
			ID3D12DescriptorHeap* heaps[] = { this->cbvSrvUavHeaps[this->commandListIndex].Get(), this->samplerHeaps[this->commandListIndex].Get() };
			this->commandList[this->commandListIndex]->SetDescriptorHeaps(_countof(heaps), heaps);
			gpuHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(this->cbvSrvUavHeaps[this->commandListIndex]->GetGPUDescriptorHandleForHeapStart(), this->currentFrame, this->cbvSrvUavDescSize);
			this->commandList[this->commandListIndex]->SetGraphicsRootDescriptorTable(0, gpuHandle);
			// texture
			april::DirectX12_Texture* texture = (april::DirectX12_Texture*)this->deviceState->texture;
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.Format = texture->dxgiFormat;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MipLevels = 1;
			int heapIndex = this->_getBackbufferCount() + this->currentFrame;
			CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle(this->cbvSrvUavHeaps[this->commandListIndex]->GetCPUDescriptorHandleForHeapStart(), heapIndex, this->cbvSrvUavDescSize);
			this->d3dDevice->CreateShaderResourceView(texture->d3dTexture.Get(), &srvDesc, cpuHandle);
			gpuHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(this->cbvSrvUavHeaps[this->commandListIndex]->GetGPUDescriptorHandleForHeapStart(), heapIndex, this->cbvSrvUavDescSize);
			this->commandList[this->commandListIndex]->SetGraphicsRootDescriptorTable(1, gpuHandle);
			// sampler
			int adressModeSize = Texture::AddressMode::getCount();
			gpuHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(this->samplerHeaps[this->commandListIndex]->GetGPUDescriptorHandleForHeapStart(), texture->getFilter().value * adressModeSize + texture->getAddressMode().value, this->samplerDescSize);
			this->commandList[this->commandListIndex]->SetGraphicsRootDescriptorTable(2, gpuHandle);
		}
		else
		{
			ID3D12DescriptorHeap* heaps[] = { this->cbvSrvUavHeaps[this->commandListIndex].Get() };
			this->commandList[this->commandListIndex]->SetDescriptorHeaps(_countof(heaps), heaps);
			gpuHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(this->cbvSrvUavHeaps[this->commandListIndex]->GetGPUDescriptorHandleForHeapStart(), this->currentFrame, this->cbvSrvUavDescSize);
			this->commandList[this->commandListIndex]->SetGraphicsRootDescriptorTable(0, gpuHandle);
		}
	}

	void DirectX12_RenderSystem::_renderDX12VertexBuffer(const RenderOperation& renderOperation, const void* data, int count, unsigned int vertexSize)
	{
		// This kind of approach to render chunks of vertices is due to the current implementation that
		// doesn't use vertex buffers by default to handle data.
		static int size = 0;
		static int byteSize = 0;
		size = count;
		D3D12_SUBRESOURCE_DATA vertexData = {};
		D3D12_RESOURCE_BARRIER vertexBufferResourceBarrier = {};
		vertexBufferResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		vertexBufferResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		vertexBufferResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
		vertexBufferResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
		vertexBufferResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		D3D12_CPU_DESCRIPTOR_HANDLE renderTargetView = this->rtvHeap->GetCPUDescriptorHandleForHeapStart();
		renderTargetView.ptr += this->currentFrame * this->rtvDescSize;
		D3D12_PRIMITIVE_TOPOLOGY primitiveTopology = _dx12RenderOperations[renderOperation.value];
		unsigned char* vertices = (unsigned char*)data;
		D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView;
		for_iter_step (i, 0, count, size)
		{
			size = this->_limitVertices(renderOperation, hmin(count - i, VERTEX_BUFFER_SIZE / (int)vertexSize));
			byteSize = size * vertexSize;
			this->_updatePipelineState(renderOperation);
			vertexData.pData = vertices;
			vertexData.RowPitch = byteSize;
			vertexData.SlicePitch = vertexData.RowPitch;
			UpdateSubresources(this->commandList[this->commandListIndex].Get(), this->vertexBuffers[this->vertexBufferIndex].Get(), this->vertexBufferUploads[this->vertexBufferIndex].Get(), 0, 0, 1, &vertexData);
			vertexBufferResourceBarrier.Transition.pResource = this->vertexBuffers[this->vertexBufferIndex].Get();
			this->commandList[this->commandListIndex]->ResourceBarrier(1, &vertexBufferResourceBarrier);
			// viewport, scissor rect, render target
			this->commandList[this->commandListIndex]->RSSetViewports(1, &this->deviceViewport);
			this->commandList[this->commandListIndex]->RSSetScissorRects(1, &this->deviceScissorRect);
			if (this->deviceState->depthBuffer)
			{
				depthStencilView = this->dsvHeap->GetCPUDescriptorHandleForHeapStart();
				this->commandList[this->commandListIndex]->OMSetRenderTargets(1, &renderTargetView, false, &depthStencilView);
			}
			else
			{
				this->commandList[this->commandListIndex]->OMSetRenderTargets(1, &renderTargetView, false, NULL);
			}
			this->commandList[this->commandListIndex]->IASetPrimitiveTopology(primitiveTopology);
			this->vertexBufferViews[this->vertexBufferIndex].BufferLocation = this->vertexBuffers[this->vertexBufferIndex]->GetGPUVirtualAddress();
			this->vertexBufferViews[this->vertexBufferIndex].StrideInBytes = vertexSize;
			this->vertexBufferViews[this->vertexBufferIndex].SizeInBytes = byteSize;
			this->commandList[this->commandListIndex]->IASetVertexBuffers(0, 1, &this->vertexBufferViews[this->vertexBufferIndex]);
			this->commandList[this->commandListIndex]->DrawInstanced(size, 1, 0, 0);
			this->vertexBufferIndex = (this->vertexBufferIndex + 1) % MAX_VERTEX_BUFFERS;
			vertices += byteSize;
		}
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
		// TODOuwp - if possible
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
		this->commandList[this->commandListIndex]->ResourceBarrier(1, &presentResourceBarrier);
		this->executeCurrentCommand();
		this->waitForAllCommands();
		HRESULT hr = this->swapChain->Present((this->options.vSync ? 1 : 0), 0);
		if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
		{
			// TODOuwp - handle this properly
			//m_deviceRemoved = true;
			//this->updateDeviceReset();
			return;
		}
		_TRY_UNSAFE(hr, "Unable to present swap chain!");
		this->deviceState_rootSignature = (this->deviceState->useTexture && this->deviceState->texture != NULL ? this->rootSignatures[1] : this->rootSignatures[0]);
		this->waitForAllCommands();
		this->prepareNewCommands();
		D3D12_RESOURCE_BARRIER renderTargetResourceBarrier = {};
		renderTargetResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		renderTargetResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		renderTargetResourceBarrier.Transition.pResource = this->renderTargets[this->currentFrame].Get();
		renderTargetResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
		renderTargetResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		renderTargetResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		this->commandList[this->commandListIndex]->ResourceBarrier(1, &renderTargetResourceBarrier);
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
		// TODOuwp - implement
		return NULL;// this->renderTarget;
	}

	void DirectX12_RenderSystem::setRenderTarget(Texture* source)
	{
		// TODOuwp - implement (this code is experimental)
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
	}

	void DirectX12_RenderSystem::setPixelShader(PixelShader* pixelShader)
	{
		// TODOuwp
		//this->activePixelShader = (DirectX12_PixelShader*)pixelShader;
	}

	void DirectX12_RenderSystem::setVertexShader(VertexShader* vertexShader)
	{
		// TODOuwp
		//this->activeVertexShader = (DirectX12_VertexShader*)vertexShader;
	}

}

#endif
