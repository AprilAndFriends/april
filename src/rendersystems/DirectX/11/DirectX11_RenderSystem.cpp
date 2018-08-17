/// @file
/// @version 5.2
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifdef _DIRECTX11
#include <comdef.h>
#include <d3d11_4.h>
#include <dxgi1_5.h>
#include <stdio.h>
#ifdef _DEBUG
#include <dxgidebug.h>
#endif

#include <gtypes/Rectangle.h>
#include <gtypes/Vector2.h>
#include <hltypes/hexception.h>
#include <hltypes/hlog.h>
#include <hltypes/hplatform.h>
#include <hltypes/hresource.h>
#include <hltypes/hthread.h>

#include "april.h"
#include "DirectX11_PixelShader.h"
#include "DirectX11_RenderSystem.h"
#include "DirectX11_Texture.h"
#include "DirectX11_VertexShader.h"
#include "Image.h"
#include "Keys.h"
#include "Platform.h"
#include "RenderState.h"
#include "Timer.h"
#include "UWP.h"
#include "UWP_Window.h"

#define SHADER_PATH "april/"
#define VERTEX_BUFFER_COUNT 65536
#define BACKBUFFER_COUNT 2

#define __EXPAND(x) x

#define LOAD_SHADER(name, type, file) \
	if (name == NULL) \
	{ \
		name = (DirectX11_ ## type ## Shader*)this->create ## type ## ShaderFromResource(SHADER_PATH #type "Shader_" #file ".cso"); \
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

	static ColoredTexturedVertex static_ctv[VERTEX_BUFFER_COUNT];

	D3D11_PRIMITIVE_TOPOLOGY DirectX11_RenderSystem::_dx11RenderOperations[] =
	{
		D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST,	// RenderOperation::TriangleList
		D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP,	// RenderOperation::TriangleStrip
		D3D_PRIMITIVE_TOPOLOGY_LINELIST,		// RenderOperation::ListList
		D3D_PRIMITIVE_TOPOLOGY_LINESTRIP,		// RenderOperation::LineStrip
		D3D_PRIMITIVE_TOPOLOGY_POINTLIST,		// RenderOperation::PointList
		D3D_PRIMITIVE_TOPOLOGY_UNDEFINED,		// triangle fans are deprecated in DX11
	};

	DirectX11_RenderSystem::ShaderComposition::ShaderComposition(ComPtr<ID3D11InputLayout> inputLayout,
		DirectX11_VertexShader* vertexShader, DirectX11_PixelShader* pixelShader)
	{
		this->inputLayout = inputLayout;
		this->vertexShader = vertexShader;
		this->pixelShader = pixelShader;
	}

	DirectX11_RenderSystem::ShaderComposition::~ShaderComposition()
	{
	}

	DirectX11_RenderSystem::DirectX11_RenderSystem() : DirectX_RenderSystem(), deviceState_constantBufferChanged(true),
		deviceState_shader(NULL), deviceState_sampler(nullptr), deviceState_renderOperation(RenderOperation::PointList)
	{
		this->name = april::RenderSystemType::DirectX11.getName();
	}

	DirectX11_RenderSystem::~DirectX11_RenderSystem()
	{
		this->destroy();
	}

	void DirectX11_RenderSystem::_deviceInit()
	{
		this->dxgiFactory = nullptr;
		this->d3dDevice = nullptr;
		this->d3dDeviceContext = nullptr;
		this->swapChain = nullptr;
		this->rasterState = nullptr;
		this->renderTarget = nullptr;
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

	bool DirectX11_RenderSystem::_deviceCreate(Options options)
	{
		hlog::write(logTag, "april::getSystemInfo() in DirectX11_RenderSystem::_deviceCreate()");
		this->setViewport(grecti(0, 0, april::getSystemInfo().displayResolution));
		return true;
	}

	bool DirectX11_RenderSystem::_deviceDestroy()
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
		hlog::write(logTag, "april::getSystemInfo() in DirectX11_RenderSystem::_deviceDestroy()");
		this->setViewport(grecti(0, 0, april::getSystemInfo().displayResolution));
		return true;
	}

	void DirectX11_RenderSystem::_deviceAssignWindow(Window* window)
	{
		unsigned int creationFlags = 0;
		unsigned int dxgiFactoryFlags = 0;
		creationFlags |= D3D11_CREATE_DEVICE_PREVENT_ALTERING_LAYER_SETTINGS_FROM_REGISTRY;
#ifndef _DEBUG
		if (this->options.debugInfo)
#endif
		{
			if (SUCCEEDED(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_NULL, 0, D3D11_CREATE_DEVICE_DEBUG, nullptr, 0, D3D11_SDK_VERSION, nullptr, nullptr, nullptr)))
			{
				creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
				dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
			}
			else
			{
				hlog::warn(logTag, "D3D debug device is not available.");
			}
		}
#ifdef _DEBUG
		ComPtr<IDXGIInfoQueue> dxgiInfoQueue;
		if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(dxgiInfoQueue.GetAddressOf()))))
		{
			dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_WARNING, true);
			dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, true);
			dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, true);
		}
#endif
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
		D3D_FEATURE_LEVEL availableFeatureLevels[] =
		{
			D3D_FEATURE_LEVEL_12_1,
			D3D_FEATURE_LEVEL_12_0,
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_10_0,
			D3D_FEATURE_LEVEL_9_3,
			D3D_FEATURE_LEVEL_9_2,
			D3D_FEATURE_LEVEL_9_1
		};
		ComPtr<IDXGIAdapter1> adapter = nullptr;
		this->_getAdapter(adapter.GetAddressOf());
		HRESULT hr = E_FAIL;
		ComPtr<ID3D11Device> device;
		ComPtr<ID3D11DeviceContext> context;
		D3D_FEATURE_LEVEL createdFeatureLevel;
		if (adapter != nullptr)
		{
			hr = D3D11CreateDevice(adapter.Get(), D3D_DRIVER_TYPE_UNKNOWN, 0, creationFlags, availableFeatureLevels, _countof(availableFeatureLevels),
				D3D11_SDK_VERSION, device.GetAddressOf(), &createdFeatureLevel, context.GetAddressOf());
			if (FAILED(hr))
			{
				hlog::write(logTag, "Hardware device not available. Falling back to software device.");
				adapter = nullptr;
			}
		}
		else
		{
			hlog::write(logTag, "Unable to find hardware adapter. Falling back to software device.");
		}
		if (adapter == nullptr)
		{
			this->_getAdapter(adapter.GetAddressOf(), false);
			if (adapter != nullptr)
			{
				hr = D3D11CreateDevice(adapter.Get(), D3D_DRIVER_TYPE_UNKNOWN, 0, creationFlags, availableFeatureLevels, _countof(availableFeatureLevels),
					D3D11_SDK_VERSION, device.GetAddressOf(), &createdFeatureLevel, context.GetAddressOf());
				if (FAILED(hr))
				{
					hlog::write(logTag, "Software device not available. Falling back to WARP device.");
					adapter = nullptr;
				}
			}
			else
			{
				hlog::write(logTag, "Unable to find software adapter. Falling back to WARP device.");
			}
		}
		// no valid adapter, use WARP
		if (adapter == nullptr)
		{
			ComPtr<IDXGIAdapter> warpAdapter = nullptr;
			_TRY_UNSAFE(this->dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)), "Unable to create DX12 device! WARP device not available!");
			hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_WARP, 0, creationFlags, availableFeatureLevels, _countof(availableFeatureLevels),
				D3D11_SDK_VERSION, device.GetAddressOf(), &createdFeatureLevel, context.GetAddressOf());
		}
		_TRY_UNSAFE(hr, "Unable to create DX11 device!");
#ifdef _DEBUG
		ComPtr<ID3D11Debug> d3dDebug;
		if (SUCCEEDED(device.As(&d3dDebug)))
		{
			ComPtr<ID3D11InfoQueue> d3dInfoQueue;
			if (SUCCEEDED(d3dDebug.As(&d3dInfoQueue)))
			{
				d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
				d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
				D3D11_MESSAGE_ID hide[] = { D3D11_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS, };
				D3D11_INFO_QUEUE_FILTER filter = {};
				filter.DenyList.NumIDs = _countof(hide);
				filter.DenyList.pIDList = hide;
				d3dInfoQueue->AddStorageFilterEntries(&filter);
			}
		}
#endif
		_TRY_UNSAFE(device.As(&this->d3dDevice), "Could not cast D3D device!");
		_TRY_UNSAFE(context.As(&this->d3dDeviceContext), "Could not cast D3D device context!");
		// device config
		this->_configureDevice();
		// initial vertex buffer data
		this->vertexBufferData.pSysMem = NULL;
		this->vertexBufferData.SysMemPitch = 0;
		this->vertexBufferData.SysMemSlicePitch = 0;
		this->vertexBufferDesc.ByteWidth = 0;
		this->vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		this->vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		this->vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		this->vertexBufferDesc.MiscFlags = 0;
		this->vertexBufferDesc.StructureByteStride = 0;
		// initial constant buffer
		D3D11_SUBRESOURCE_DATA constantSubresourceData = { 0 };
		constantSubresourceData.pSysMem = &this->constantBufferData;
		constantSubresourceData.SysMemPitch = 0;
		constantSubresourceData.SysMemSlicePitch = 0;
		D3D11_BUFFER_DESC constantBufferDesc = {0};
		constantBufferDesc.ByteWidth = sizeof(this->constantBufferData);
		constantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		constantBufferDesc.MiscFlags = 0;
		constantBufferDesc.StructureByteStride = 0;
		_TRY_UNSAFE(this->d3dDevice->CreateBuffer(&constantBufferDesc, &constantSubresourceData, &this->constantBuffer), "Unable to create constant buffer!");
		this->d3dDeviceContext->VSSetConstantBuffers(0, 1, this->constantBuffer.GetAddressOf());
		// initial calls
		this->setOrthoProjection(gvec2f((float)window->getWidth(), (float)window->getHeight()));
		this->_deviceClear(true);
		this->_devicePresentFrame(true);
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
		static const D3D11_INPUT_ELEMENT_DESC inputLayoutDescPlain[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		static const D3D11_INPUT_ELEMENT_DESC inputLayoutDescTextured[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		static const D3D11_INPUT_ELEMENT_DESC inputLayoutDescColored[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		static const D3D11_INPUT_ELEMENT_DESC inputLayoutDescColoredTextured[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
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
	}

	void DirectX11_RenderSystem::_deviceReset()
	{
		DirectX_RenderSystem::_deviceReset();
		// TODOuwp - is this still needed?
		/*
		// possible Microsoft bug, required for SwapChainPanel to update its layout 
		reinterpret_cast<IUnknown*>(UWP::app->Overlay)->QueryInterface(IID_PPV_ARGS(&this->swapChainNative));
		this->swapChainNative->SetSwapChain(this->swapChain.Get());
		*/
	}

	void DirectX11_RenderSystem::_deviceSetupCaps()
	{
		// depends on FEATURE_LEVEL, while 9.3 supports 4096, 9.2 and 9.1 support only 2048 so using 2048 is considered safe
		this->caps.maxTextureSize = D3D_FL9_1_REQ_TEXTURE1D_U_DIMENSION;
		this->caps.npotTexturesLimited = true;
		this->caps.npotTextures = false; // because of usage of feature level 9_3
	}

	void DirectX11_RenderSystem::_deviceSetup()
	{
		// not used
	}

	void DirectX11_RenderSystem::_getAdapter(IDXGIAdapter1** adapter, bool hardware)
	{
		*adapter = NULL;
		ComPtr<IDXGIAdapter1> currentAdapter;
		UINT currentAdapterIndex = 0;
		DXGI_ADAPTER_DESC1 currentAdapterDesc;
#if defined(__dxgi1_6_h__) && defined(NTDDI_WIN10_RS4) // TODOuwp - this has actually not been implemented yet
		ComPtr<IDXGIFactory6> factory6;
		HRESULT hr = this->dxgiFactory.As(&factory6);
		if (SUCCEEDED(hr))
		{
			while (DXGI_ERROR_NOT_FOUND != factory6->EnumAdapterByGpuPreference(currentAdapterIndex, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(currentAdapter.ReleaseAndGetAddressOf())))
			{
				currentAdapter->GetDesc1(&currentAdapterDesc);
				if (((currentAdapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0) == hardware)
				{
					break;
				}
				currentAdapter = nullptr;
				++currentAdapterIndex;
			}
		}
		else
#endif
		{
			while (DXGI_ERROR_NOT_FOUND != this->dxgiFactory->EnumAdapters1(currentAdapterIndex, currentAdapter.ReleaseAndGetAddressOf()))
			{
				currentAdapter->GetDesc1(&currentAdapterDesc);
				if (((currentAdapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0) == hardware)
				{
					break;
				}
				currentAdapter = nullptr;
				++currentAdapterIndex;
			}
		}
		if (currentAdapter != nullptr)
		{
			*adapter = currentAdapter.Detach();
		}
	}

	void DirectX11_RenderSystem::_createSwapChain(int width, int height)
	{
		// Once the swap chain desc is configured, it must be
		// created on the same adapter as the existing D3D Device.
		ComPtr<IDXGIDevice3> dxgiDevice;
		_TRY_UNSAFE(this->d3dDevice.As(&dxgiDevice), "Unable to retrieve DXGI device!");
		// TODOuwp - this might need to be removed for vsync=disabled to work
		_TRY_UNSAFE(dxgiDevice->SetMaximumFrameLatency(1), "Unable to set MaximumFrameLatency!");
		ComPtr<IDXGIAdapter> dxgiAdapter;
		_TRY_UNSAFE(dxgiDevice->GetAdapter(&dxgiAdapter), "Unable to get adapter from DXGI device!");
		ComPtr<IDXGIFactory3> dxgiFactory;
		_TRY_UNSAFE(dxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory)), "Unable to get parent factory from DXGI adapter!");
		SystemInfo info = april::getSystemInfo();
		int w = info.displayResolution.x;
		int h = info.displayResolution.y;
		if (w != width || h != height)
		{
			hlog::warnf(logTag, "On UWP the window resolution (%d,%d) should match the display resolution (%d,%d) in order to avoid problems.", width, height, w, h);
		}
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { };
		swapChainDesc.Width = width;
		swapChainDesc.Height = height;
		swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = BACKBUFFER_COUNT; // TODOuwp - should respect tripleBuffering option
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.Scaling = DXGI_SCALING_ASPECT_RATIO_STRETCH;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL; // TODOuwp - test this
		//swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
		swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH; // TODOuwp - test this
		if (this->options.vSync)
		{
			swapChainDesc.Flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
		}
		ComPtr<IDXGISwapChain1> _swapChain;
		_TRY_UNSAFE(this->dxgiFactory->CreateSwapChainForCoreWindow(this->d3dDevice.Get(), (IUnknown*)april::window->getBackendId(), &swapChainDesc,
			nullptr, _swapChain.GetAddressOf()), "Unable to create swap chain!");
		_TRY_UNSAFE(_swapChain.As(&this->swapChain), "Could not cast swap chain!");
		this->_configureSwapChain();
		this->updateOrientation();
	}

	void DirectX11_RenderSystem::_resizeSwapChain(int width, int height)
	{
		UINT flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH; // TODOuwp - test this
		if (this->options.vSync)
		{
			flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
		}
		_TRY_UNSAFE(this->swapChain->ResizeBuffers(BACKBUFFER_COUNT, width, height, DXGI_FORMAT_B8G8R8A8_UNORM, flags), "Unable to resize swap chain buffers!");
		this->_configureSwapChain();
		this->updateOrientation();
	}

	void DirectX11_RenderSystem::_configureSwapChain()
	{
		// so... we have to apply an inverted scale to the swap chain?
		DXGI_MATRIX_3X2_F inverseScale = { 0 };
		// TODOuwp - implement this
		/*
		inverseScale._11 = 1.0f / WinRT::App->Overlay->CompositionScaleX;
		inverseScale._22 = 1.0f / WinRT::App->Overlay->CompositionScaleY;
		*/
		//this->swapChain->SetMatrixTransform(&inverseScale);
		// get the back buffer
		//ComPtr<ID3D11Texture2D> _backBuffer;
		_TRY_UNSAFE(this->swapChain->GetBuffer(0, IID_PPV_ARGS(&this->renderTarget)), "Unable to get swap chain back buffer!");
		// Create a descriptor for the RenderTargetView.
		CD3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc(D3D11_RTV_DIMENSION_TEXTURE2DARRAY, DXGI_FORMAT_B8G8R8A8_UNORM, 0, 0, 1);
		_TRY_UNSAFE(this->d3dDevice->CreateRenderTargetView(this->renderTarget.Get(), &renderTargetViewDesc, &this->renderTargetView), "Unable to create render target view!");
		// has to use GetAddressOf(), because the parameter is a pointer to an array of render target views
		this->d3dDeviceContext->OMSetRenderTargets(1, this->renderTargetView.GetAddressOf(), NULL);
	}

	void DirectX11_RenderSystem::_configureDevice()
	{
		this->d3dDeviceContext->OMSetRenderTargets(0, NULL, NULL);
		this->renderTargetView = nullptr;
		this->renderTarget = nullptr;
		// TODOuwp - implement
		//this->d3dDepthStencilView = nullptr;
		//this->depthStencil = nullptr;
		this->d3dDeviceContext->Flush();
		if (this->swapChain != nullptr)
		{
			this->_resizeSwapChain(april::window->getWidth(), april::window->getHeight());
		}
		else
		{
			this->_createSwapChain(april::window->getWidth(), april::window->getHeight());
		}
		D3D11_RASTERIZER_DESC rasterDesc;
		rasterDesc.AntialiasedLineEnable = false;
		rasterDesc.CullMode = D3D11_CULL_NONE;
		rasterDesc.DepthBias = 0;
		rasterDesc.DepthBiasClamp = 0.0f;
		rasterDesc.DepthClipEnable = true;
		rasterDesc.FillMode = D3D11_FILL_SOLID;
		rasterDesc.FrontCounterClockwise = false;
		rasterDesc.MultisampleEnable = false;
		rasterDesc.ScissorEnable = false;
		rasterDesc.SlopeScaledDepthBias = 0.0f;
		_TRY_UNSAFE(this->d3dDevice->CreateRasterizerState(&rasterDesc, &this->rasterState), "Unable to create raster state!");
		this->d3dDeviceContext->RSSetState(this->rasterState.Get());
		D3D11_TEXTURE2D_DESC backBufferDesc = { };
		this->renderTarget->GetDesc(&backBufferDesc);
		this->setViewport(grecti(0, 0, backBufferDesc.Width, backBufferDesc.Height)); // just to be on the safe side and prevent floating point errors
		// blend modes
		D3D11_BLEND_DESC blendDesc = { };
		blendDesc.AlphaToCoverageEnable = false;
		blendDesc.IndependentBlendEnable = false;
		blendDesc.RenderTarget[0].BlendEnable = true;
		blendDesc.RenderTarget[0].RenderTargetWriteMask = (D3D11_COLOR_WRITE_ENABLE_RED | D3D11_COLOR_WRITE_ENABLE_GREEN | D3D11_COLOR_WRITE_ENABLE_BLUE);
		// alpha
		blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
		blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		this->d3dDevice->CreateBlendState(&blendDesc, &this->blendStateAlpha);
		// add
		blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
		blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
		this->d3dDevice->CreateBlendState(&blendDesc, &this->blendStateAdd);
		// subtract
		blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
		blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_REV_SUBTRACT;
		blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
		this->d3dDevice->CreateBlendState(&blendDesc, &this->blendStateSubtract);
		// overwrite
		blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
		this->d3dDevice->CreateBlendState(&blendDesc, &this->blendStateOverwrite);
		// texture samplers
		D3D11_SAMPLER_DESC samplerDesc;
		memset(&samplerDesc, 0, sizeof(samplerDesc));
		samplerDesc.MaxAnisotropy = 0;
		samplerDesc.MipLODBias = 0.0f;
		samplerDesc.MinLOD = 0;
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		samplerDesc.BorderColor[0] = 0.0f;
		samplerDesc.BorderColor[1] = 0.0f;
		samplerDesc.BorderColor[2] = 0.0f;
		samplerDesc.BorderColor[3] = 0.0f;
		// linear + wrap
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		this->d3dDevice->CreateSamplerState(&samplerDesc, &this->samplerLinearWrap);
		// linear + clamp
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		this->d3dDevice->CreateSamplerState(&samplerDesc, &this->samplerLinearClamp);
		// nearest neighbor + wrap
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		this->d3dDevice->CreateSamplerState(&samplerDesc, &this->samplerNearestWrap);
		// nearest neighbor + clamp
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		this->d3dDevice->CreateSamplerState(&samplerDesc, &this->samplerNearestClamp);
		// other
		this->_deviceClear(true);
		this->_devicePresentFrame(true);
	}

	int DirectX11_RenderSystem::getVRam() const
	{
		if (this->d3dDevice == nullptr)
		{
			return 0;
		}
		HRESULT hr;
		ComPtr<IDXGIDevice2> dxgiDevice;
		hr = this->d3dDevice.As(&dxgiDevice);
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

	Texture* DirectX11_RenderSystem::_deviceCreateTexture(bool fromResource)
	{
		return new DirectX11_Texture(fromResource);
	}

	PixelShader* DirectX11_RenderSystem::_deviceCreatePixelShader()
	{
		return new DirectX11_PixelShader();
	}

	VertexShader* DirectX11_RenderSystem::_deviceCreateVertexShader()
	{
		return new DirectX11_VertexShader();
	}

	void DirectX11_RenderSystem::_deviceChangeResolution(int w, int h, bool fullscreen)
	{
		this->d3dDeviceContext->OMSetRenderTargets(0, NULL, NULL);
		this->renderTargetView = nullptr;
		this->renderTarget = nullptr;
		// TODOuwp - implement
		//this->d3dDepthStencilView = nullptr;
		//this->depthStencil = nullptr;
		this->d3dDeviceContext->Flush();
		if (this->swapChain != nullptr)
		{
			this->_resizeSwapChain(april::window->getWidth(), april::window->getHeight());
		}
		else
		{
			this->_createSwapChain(april::window->getWidth(), april::window->getHeight());
		}
	}

	void DirectX11_RenderSystem::_setDeviceViewport(cgrecti rect)
	{
		grecti viewport = rect;
		// this is needed on WinRT because of a graphics driver bug on Windows RT and on WinP8 because of a completely different graphics driver bug on Windows Phone 8
		gvec2i resolution = april::getSystemInfo().displayResolution;
		// TODOuwp - is this still needed in DX11?
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
		// setting the system viewport
		D3D11_VIEWPORT dx11Viewport;
		dx11Viewport.MinDepth = D3D11_MIN_DEPTH;
		dx11Viewport.MaxDepth = D3D11_MAX_DEPTH;
		dx11Viewport.TopLeftX = (float)viewport.x;
		dx11Viewport.TopLeftY = (float)viewport.y;
		dx11Viewport.Width = (float)viewport.w;
		dx11Viewport.Height = (float)viewport.h;
		this->d3dDeviceContext->RSSetViewports(1, &dx11Viewport);
	}

	void DirectX11_RenderSystem::_setDeviceModelviewMatrix(const gmat4& matrix)
	{
		this->deviceState_constantBufferChanged = true;
	}

	void DirectX11_RenderSystem::_setDeviceProjectionMatrix(const gmat4& matrix)
	{
		this->deviceState_constantBufferChanged = true;
	}

	void DirectX11_RenderSystem::_setDeviceDepthBuffer(bool enabled, bool writeEnabled)
	{
		//hlog::error(logTag, "Not implemented!");
	}

	void DirectX11_RenderSystem::_setDeviceRenderMode(bool useTexture, bool useColor)
	{
		// not used
	}

	void DirectX11_RenderSystem::_setDeviceTexture(Texture* texture)
	{
		if (texture != NULL)
		{
			this->d3dDeviceContext->PSSetShaderResources(0, 1, ((DirectX11_Texture*)texture)->d3dView.GetAddressOf());
			Texture::Filter filter = texture->getFilter();
			Texture::AddressMode addressMode = texture->getAddressMode();
			ComPtr<ID3D11SamplerState> sampler = nullptr;
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
	}

	void DirectX11_RenderSystem::_setDeviceTextureFilter(const Texture::Filter& textureFilter)
	{
		// not used
	}

	void DirectX11_RenderSystem::_setDeviceTextureAddressMode(const Texture::AddressMode& textureAddressMode)
	{
		// not used
	}

	void DirectX11_RenderSystem::_setDeviceBlendMode(const BlendMode& blendMode)
	{
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
	}

	void DirectX11_RenderSystem::_setDeviceColorMode(const ColorMode& colorMode, float colorModeFactor, bool useTexture, bool useColor, const Color& systemColor)
	{
		this->deviceState_constantBufferChanged = true;
	}

	void DirectX11_RenderSystem::_updateDeviceState(RenderState* state, bool forceUpdate)
	{
		DirectX_RenderSystem::_updateDeviceState(state, forceUpdate);
		this->_updateShader(forceUpdate);
	}

	void DirectX11_RenderSystem::_updateShader(bool forceUpdate)
	{
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
		if (this->deviceState_shader != NULL)
		{
			if (inputLayoutChanged)
			{
				this->d3dDeviceContext->IASetInputLayout(this->deviceState_shader->inputLayout.Get());
			}
			if (vertexShaderChanged)
			{
				this->d3dDeviceContext->VSSetShader(this->deviceState_shader->vertexShader->dx11Shader.Get(), NULL, 0);
			}
			if (pixelShaderChanged)
			{
				this->d3dDeviceContext->PSSetShader(this->deviceState_shader->pixelShader->dx11Shader.Get(), NULL, 0);
			}
		}
		// change other data
		if (this->deviceState_constantBufferChanged)
		{
			this->constantBufferData.matrix = (this->deviceState->projectionMatrix * this->deviceState->modelviewMatrix).transposed();
			this->constantBufferData.systemColor.set(this->deviceState->systemColor.r_f(), this->deviceState->systemColor.g_f(),
				this->deviceState->systemColor.b_f(), this->deviceState->systemColor.a_f());
			this->constantBufferData.lerpAlpha.set(this->deviceState->colorModeFactor, this->deviceState->colorModeFactor,
				this->deviceState->colorModeFactor, this->deviceState->colorModeFactor);
			this->d3dDeviceContext->Map(this->constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &this->mappedSubResource);
			memcpy(this->mappedSubResource.pData, &this->constantBufferData, sizeof(ConstantBuffer));
			this->d3dDeviceContext->Unmap(this->constantBuffer.Get(), 0);
			this->deviceState_constantBufferChanged = false;
		}
	}

	void DirectX11_RenderSystem::_deviceClear(bool depth)
	{
		static const float clearColor[4] = {0.0f, 0.0f, 0.0f, 0.0f};
		// TODOuwp - does this work?
		this->d3dDeviceContext->ClearRenderTargetView(this->renderTargetView.Get(), clearColor);
	}
	
	void DirectX11_RenderSystem::_deviceClear(const Color& color, bool depth)
	{
		static float clearColor[4] = {0.0f, 0.0f, 0.0f, 0.0f};
		clearColor[0] = color.b_f();
		clearColor[1] = color.g_f();
		clearColor[2] = color.r_f();
		clearColor[3] = color.a_f();
		// TODOuwp - does this work?
		this->d3dDeviceContext->ClearRenderTargetView(this->renderTargetView.Get(), clearColor);
	}

	void DirectX11_RenderSystem::_deviceClearDepth()
	{
		// TODOuwp - implement
		//static const float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		//this->d3dDeviceContext->ClearRenderTargetView(this->renderTargetView.Get(), clearColor);
	}

	void DirectX11_RenderSystem::_deviceRender(const RenderOperation& renderOperation, const PlainVertex* vertices, int count)
	{
		this->_setDX11VertexBuffer(renderOperation, vertices, count, sizeof(PlainVertex));
		this->d3dDeviceContext->Draw(count, 0);
	}

	void DirectX11_RenderSystem::_deviceRender(const RenderOperation& renderOperation, const TexturedVertex* vertices, int count)
	{
		this->_setDX11VertexBuffer(renderOperation, vertices, count, sizeof(TexturedVertex));
		this->d3dDeviceContext->Draw(count, 0);
	}

	void DirectX11_RenderSystem::_deviceRender(const RenderOperation& renderOperation, const ColoredVertex* vertices, int count)
	{
		this->_setDX11VertexBuffer(renderOperation, vertices, count, sizeof(ColoredVertex));
		this->d3dDeviceContext->Draw(count, 0);
	}

	void DirectX11_RenderSystem::_deviceRender(const RenderOperation& renderOperation, const ColoredTexturedVertex* vertices, int count)
	{
		this->_setDX11VertexBuffer(renderOperation, vertices, count, sizeof(ColoredTexturedVertex));
		this->d3dDeviceContext->Draw(count, 0);
	}

	void DirectX11_RenderSystem::_setDX11VertexBuffer(const RenderOperation& renderOperation, const void* data, int count, unsigned int vertexSize)
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
			this->d3dDeviceContext->Map(this->vertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &this->mappedSubResource);
			memcpy(this->mappedSubResource.pData, data, size);
			this->d3dDeviceContext->Unmap(this->vertexBuffer.Get(), 0);
		}
		static unsigned int offset = 0;
		this->d3dDeviceContext->IASetVertexBuffers(0, 1, this->vertexBuffer.GetAddressOf(), &vertexSize, &offset);
	}

	Image::Format DirectX11_RenderSystem::getNativeTextureFormat(Image::Format format) const
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
	
	unsigned int DirectX11_RenderSystem::getNativeColorUInt(const april::Color& color) const
	{
		return ((color.a << 24) | (color.b << 16) | (color.g << 8) | color.r);
	}

	Image* DirectX11_RenderSystem::takeScreenshot(Image::Format format)
	{
		// TODOa - if possible
		hlog::warn(logTag, "DirectX11_RenderSystem::takeScreenshot() not implemented!");
		return NULL;
	}
	
	void DirectX11_RenderSystem::_devicePresentFrame(bool systemEnabled)
	{
		RenderSystem::_devicePresentFrame(systemEnabled);
		// TODOuwp - for vsync support, is this correct?
		this->options.vSync ? this->swapChain->Present(1, 0) : this->swapChain->Present(0, DXGI_PRESENT_ALLOW_TEARING);
		// has to use GetAddressOf(), because the parameter is a pointer to an array of render target views
		this->d3dDeviceContext->OMSetRenderTargets(1, this->renderTargetView.GetAddressOf(), NULL);
	}

	void DirectX11_RenderSystem::updateOrientation()
	{
		// TODOuwp - this doesn't seem to do anything, remove?
		DisplayOrientations orientation = DisplayInformation::GetForCurrentView()->CurrentOrientation;
		DXGI_MODE_ROTATION rotation = DXGI_MODE_ROTATION_UNSPECIFIED;
		if (orientation == DisplayOrientations::Landscape)
		{
			rotation = DXGI_MODE_ROTATION_ROTATE90;
		}
		else if (orientation == DisplayOrientations::Portrait)
		{
			rotation = DXGI_MODE_ROTATION_IDENTITY;
		}
		else if (orientation == DisplayOrientations::LandscapeFlipped)
		{
			rotation = DXGI_MODE_ROTATION_ROTATE270;
		}
		else if (orientation == DisplayOrientations::PortraitFlipped)
		{
			rotation = DXGI_MODE_ROTATION_ROTATE180;
		}
		else
		{
			hlog::error(logTag, "Undefined screen orienation, using default landscape!");
			rotation = DXGI_MODE_ROTATION_ROTATE90;
		}
	}

	void DirectX11_RenderSystem::trim()
	{
		// TODOuwp - is this still needed in UWP?
		ComPtr<IDXGIDevice3> dxgiDevice;
		_TRY_UNSAFE(this->d3dDevice.As(&dxgiDevice), "Unable to retrieve DXGI device!");
		dxgiDevice->Trim();
	}

	Texture* DirectX11_RenderSystem::getRenderTarget()
	{
		// TODOa - implement
		return NULL;// this->renderTarget;
	}

	void DirectX11_RenderSystem::setRenderTarget(Texture* source)
	{
		// TODOa - implement (this code is experimental)
		/*
		DirectX11_Texture* texture = (DirectX11_Texture*)source;
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

	void DirectX11_RenderSystem::setPixelShader(PixelShader* pixelShader)
	{
		// TODOa
		//this->activePixelShader = (DirectX11_PixelShader*)pixelShader;
	}

	void DirectX11_RenderSystem::setVertexShader(VertexShader* vertexShader)
	{
		// TODOa
		//this->activeVertexShader = (DirectX11_VertexShader*)vertexShader;
	}

}

#endif
