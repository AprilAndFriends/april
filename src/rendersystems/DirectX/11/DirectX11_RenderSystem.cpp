/// @file
/// @version 4.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifdef _DIRECTX11
#include <d3d11_2.h>
#include <stdio.h>

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
#include "WinRT.h"
#include "WinRT_Window.h"

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
	static ColoredTexturedVertex static_ctv[VERTEX_BUFFER_COUNT];

	D3D11_PRIMITIVE_TOPOLOGY DirectX11_RenderSystem::_dx11RenderOperations[] =
	{
		D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED,
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST,	// ROP_TRIANGLE_LIST
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP,	// ROP_TRIANGLE_STRIP
		D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED,		// triangle fans are deprecated in DX11
		D3D11_PRIMITIVE_TOPOLOGY_LINELIST,		// ROP_LINE_LIST
		D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP,		// ROP_LINE_STRIP
		D3D11_PRIMITIVE_TOPOLOGY_POINTLIST,		// ROP_POINT_LIST
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
		deviceState_shader(NULL), deviceState_sampler(nullptr), deviceState_renderOperation(RO_UNDEFINED)
	{
		this->name = APRIL_RS_DIRECTX11;
	}

	DirectX11_RenderSystem::~DirectX11_RenderSystem()
	{
		this->destroy();
	}

	void DirectX11_RenderSystem::_deviceInit()
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
		this->deviceState_renderOperation = RO_UNDEFINED;
	}

	bool DirectX11_RenderSystem::_deviceCreate(Options options)
	{
		this->setViewport(grect(0.0f, 0.0f, april::getSystemInfo().displayResolution));
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
		this->setViewport(grect(0.0f, 0.0f, april::getSystemInfo().displayResolution));
		return true;
	}

	void DirectX11_RenderSystem::_deviceAssignWindow(Window* window)
	{
		unsigned int creationFlags = 0;
		creationFlags |= D3D11_CREATE_DEVICE_PREVENT_ALTERING_LAYER_SETTINGS_FROM_REGISTRY;
		D3D_FEATURE_LEVEL featureLevels[] =
		{
			// Intel HD GPUs have driver problems with 11.x feature levels so they have been disabled
			//D3D_FEATURE_LEVEL_11_1,
			//D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_10_0,
			D3D_FEATURE_LEVEL_9_3,
			D3D_FEATURE_LEVEL_9_2,
			D3D_FEATURE_LEVEL_9_1
		};
		ComPtr<ID3D11Device> _d3dDevice;
		ComPtr<ID3D11DeviceContext> _d3dDeviceContext;
		HRESULT hr;
		hr = D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, creationFlags, featureLevels,
			ARRAYSIZE(featureLevels), D3D11_SDK_VERSION, &_d3dDevice, NULL, &_d3dDeviceContext);
		if (FAILED(hr))
		{
			hlog::write(logTag, "Hardware device not available. Falling back to WARP device.");
			// if hardware device is not available, try a WARP device as a fallback instead
			hr = D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_WARP, NULL, creationFlags, featureLevels,
				ARRAYSIZE(featureLevels), D3D11_SDK_VERSION, &_d3dDevice, NULL, &_d3dDeviceContext);
			if (FAILED(hr))
			{
				throw Exception("Unable to create DX11 device!");
			}
		}
		hr = _d3dDevice.As(&this->d3dDevice);
		if (FAILED(hr))
		{
			throw Exception("Unable to retrieve Direct3D 11.1 device interface!");
		}
		hr = _d3dDeviceContext.As(&this->d3dDeviceContext);
		if (FAILED(hr))
		{
			throw Exception("Unable to retrieve Direct3D 11.1 device context interface!");
		}
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
		D3D11_SUBRESOURCE_DATA constantSubresourceData = {0};
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
		hr = this->d3dDevice->CreateBuffer(&constantBufferDesc, &constantSubresourceData, &this->constantBuffer);
		if (FAILED(hr))
		{
			throw Exception("Unable to create constant buffer!");
		}
		this->d3dDeviceContext->VSSetConstantBuffers(0, 1, this->constantBuffer.GetAddressOf());
		// initial calls
		this->_deviceClear(true);
		this->presentFrame();
		this->setOrthoProjection(gvec2((float)window->getWidth(), (float)window->getHeight()));
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
		// possible Microsoft bug, required for SwapChainPanel to update its layout 
		reinterpret_cast<IUnknown*>(WinRT::App->Overlay)->QueryInterface(IID_PPV_ARGS(&this->swapChainNative));
		this->swapChainNative->SetSwapChain(this->swapChain.Get());
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

	void DirectX11_RenderSystem::_createSwapChain(int width, int height)
	{
		// Once the swap chain desc is configured, it must be
		// created on the same adapter as the existing D3D Device.
		HRESULT hr;
		ComPtr<IDXGIDevice3> dxgiDevice;
		hr = this->d3dDevice.As(&dxgiDevice);
		if (FAILED(hr))
		{
			throw Exception("Unable to retrieve DXGI device!");
		}
		hr = dxgiDevice->SetMaximumFrameLatency(1);
		if (FAILED(hr))
		{
			throw Exception("Unable to set MaximumFrameLatency!");
		}
		ComPtr<IDXGIAdapter> dxgiAdapter;
		hr = dxgiDevice->GetAdapter(&dxgiAdapter);
		if (FAILED(hr))
		{
			throw Exception("Unable to get adapter from DXGI device!");
		}
		ComPtr<IDXGIFactory3> dxgiFactory;
		hr = dxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory));
		if (FAILED(hr))
		{
			throw Exception("Unable to get parent factory from DXGI adapter!");
		}
		SystemInfo info = april::getSystemInfo();
		int w = hround(info.displayResolution.x);
		int h = hround(info.displayResolution.y);
		if (w != width || h != height)
		{
			hlog::warnf(logTag, "On WinRT the window resolution (%d,%d) should match the display resolution (%d,%d) in order to avoid problems.", width, height, w, h);
		}
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { 0 };
		swapChainDesc.Stereo = false;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
		swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
		swapChainDesc.Width = width;
		swapChainDesc.Height = height;
		swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferCount = BACKBUFFER_COUNT;
		ComPtr<IDXGISwapChain1> _swapChain;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
		hr = dxgiFactory->CreateSwapChainForComposition(this->d3dDevice.Get(), &swapChainDesc, nullptr, &_swapChain);
		if (FAILED(hr))
		{
			throw Exception("Unable to create swap chain!");
		}
		_swapChain.As(&this->swapChain);
		reinterpret_cast<IUnknown*>(WinRT::App->Overlay)->QueryInterface(IID_PPV_ARGS(&this->swapChainNative));
		this->swapChainNative->SetSwapChain(this->swapChain.Get());
		// so... we have to apply an inverted scale to the swap chain?
		DXGI_MATRIX_3X2_F inverseScale = { 0 };
		inverseScale._11 = 1.0f / WinRT::App->Overlay->CompositionScaleX;
		inverseScale._22 = 1.0f / WinRT::App->Overlay->CompositionScaleY;
		this->swapChain->SetMatrixTransform(&inverseScale);
		this->_configureSwapChain();
		this->updateOrientation();
	}

	void DirectX11_RenderSystem::_resizeSwapChain(int width, int height)
	{
		this->d3dDeviceContext->OMSetRenderTargets(0, NULL, NULL);
		this->renderTargetView = nullptr;
		HRESULT hr = this->swapChain->ResizeBuffers(BACKBUFFER_COUNT, width, height, DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);
		if (FAILED(hr))
		{
			throw Exception("Unable to resize swap chain buffers!");
		}
		this->_configureSwapChain();
		this->updateOrientation();
	}

	void DirectX11_RenderSystem::_configureSwapChain()
	{
		ComPtr<ID3D11Texture2D> _backBuffer;
		HRESULT hr = this->swapChain->GetBuffer(0, IID_PPV_ARGS(&_backBuffer));
		if (FAILED(hr))
		{
			throw Exception("Unable to get swap chain back buffer!");
		}
		// Create a descriptor for the RenderTargetView.
		CD3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc(D3D11_RTV_DIMENSION_TEXTURE2DARRAY, DXGI_FORMAT_B8G8R8A8_UNORM, 0, 0, 1);
		hr = this->d3dDevice->CreateRenderTargetView(_backBuffer.Get(), &renderTargetViewDesc, &this->renderTargetView);
		if (FAILED(hr))
		{
			throw Exception("Unable to create render target view!");
		}
		// has to use GetAddressOf(), because the parameter is a pointer to an array of render target views
		this->d3dDeviceContext->OMSetRenderTargets(1, this->renderTargetView.GetAddressOf(), NULL);
	}

	void DirectX11_RenderSystem::_configureDevice()
	{
		if (this->swapChain != nullptr)
		{
			this->_resizeSwapChain(april::window->getWidth(), april::window->getHeight());
		}
		else
		{
			this->_createSwapChain(april::window->getWidth(), april::window->getHeight());
		}
		ComPtr<ID3D11Texture2D> _backBuffer;
		HRESULT hr = this->swapChain->GetBuffer(0, IID_PPV_ARGS(&_backBuffer));
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
		hr = this->d3dDevice->CreateRasterizerState(&rasterDesc, &this->rasterState);
		if (FAILED(hr))
		{
			throw Exception("Unable to create raster state!");
		}
		this->d3dDeviceContext->RSSetState(this->rasterState.Get());
		D3D11_TEXTURE2D_DESC backBufferDesc = {0};
		_backBuffer->GetDesc(&backBufferDesc);
		SystemInfo info = april::getSystemInfo();
		this->setViewport(grect(0.0f, 0.0f, (float)backBufferDesc.Width, (float)backBufferDesc.Height)); // just to be on the safe side and prevent floating point errors
		// blend modes
		D3D11_BLEND_DESC blendDesc = {0};
		blendDesc.AlphaToCoverageEnable = false;
		blendDesc.IndependentBlendEnable = false;
		blendDesc.RenderTarget[0].BlendEnable = true;
		blendDesc.RenderTarget[0].RenderTargetWriteMask = (D3D11_COLOR_WRITE_ENABLE_RED |
			D3D11_COLOR_WRITE_ENABLE_GREEN | D3D11_COLOR_WRITE_ENABLE_BLUE);
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
		blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
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
		this->presentFrame();
	}

	float DirectX11_RenderSystem::getPixelOffset()
	{
		return 0.0f;
	}

	int DirectX11_RenderSystem::getVRam()
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
		DirectX_RenderSystem::_deviceChangeResolution(w, h, fullscreen);
		if (this->swapChain != nullptr)
		{
			this->_resizeSwapChain(april::window->getWidth(), april::window->getHeight());
		}
		else
		{
			this->_createSwapChain(april::window->getWidth(), april::window->getHeight());
		}
	}

	void DirectX11_RenderSystem::_setDeviceViewport(const grect& rect)
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
		D3D11_VIEWPORT dx11Viewport;
		dx11Viewport.MinDepth = D3D11_MIN_DEPTH;
		dx11Viewport.MaxDepth = D3D11_MAX_DEPTH;
		// these double-casts are to ensure consistent behavior among rendering systems
		dx11Viewport.TopLeftX = (float)(int)viewport.x;
		dx11Viewport.TopLeftY = (float)(int)viewport.y;
		dx11Viewport.Width = (float)(int)viewport.w;
		dx11Viewport.Height = (float)(int)viewport.h;
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
		hlog::error(logTag, "Not implemented!");
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
			if (filter == Texture::FILTER_LINEAR && addressMode == Texture::ADDRESS_WRAP)
			{
				sampler = this->samplerLinearWrap;
			}
			else if (filter == Texture::FILTER_LINEAR && addressMode == Texture::ADDRESS_CLAMP)
			{
				sampler = this->samplerLinearClamp;
			}
			else if (filter == Texture::FILTER_NEAREST && addressMode == Texture::ADDRESS_WRAP)
			{
				sampler = this->samplerNearestWrap;
			}
			else if (filter == Texture::FILTER_NEAREST && addressMode == Texture::ADDRESS_CLAMP)
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

	void DirectX11_RenderSystem::_setDeviceTextureFilter(Texture::Filter textureFilter)
	{
		// not used
	}

	void DirectX11_RenderSystem::_setDeviceTextureAddressMode(Texture::AddressMode textureAddressMode)
	{
		// not used
	}

	void DirectX11_RenderSystem::_setDeviceBlendMode(BlendMode blendMode)
	{
		static const float blendFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		if (blendMode == BM_ALPHA || blendMode == BM_DEFAULT)
		{
			this->d3dDeviceContext->OMSetBlendState(this->blendStateAlpha.Get(), blendFactor, 0xFFFFFFFF);
		}
		else if (blendMode == BM_ADD)
		{
			this->d3dDeviceContext->OMSetBlendState(this->blendStateAdd.Get(), blendFactor, 0xFFFFFFFF);
		}
		else if (blendMode == BM_SUBTRACT)
		{
			this->d3dDeviceContext->OMSetBlendState(this->blendStateSubtract.Get(), blendFactor, 0xFFFFFFFF);
		}
		else if (blendMode == BM_OVERWRITE)
		{
			this->d3dDeviceContext->OMSetBlendState(this->blendStateOverwrite.Get(), blendFactor, 0xFFFFFFFF);
		}
		else
		{
			hlog::error(logTag, "Trying to set unsupported blend mode!");
		}
	}

	void DirectX11_RenderSystem::_setDeviceColorMode(ColorMode colorMode, float colorModeFactor, bool useTexture, bool useColor, const Color& systemColor)
	{
		this->deviceState_constantBufferChanged = true;
	}

	void DirectX11_RenderSystem::_updateDeviceState(bool forceUpdate)
	{
		DirectX_RenderSystem::_updateDeviceState(forceUpdate);
		this->_updateShader(forceUpdate);
	}

	void DirectX11_RenderSystem::_updateShader(bool forceUpdate)
	{
		// select shader
		ShaderComposition* shader = NULL;
		if (this->deviceState->colorMode == CM_MULTIPLY || this->deviceState->colorMode == CM_DEFAULT)
		{
			shader = _SELECT_SHADER(this->deviceState->useTexture, this->deviceState->useColor, Multiply);
		}
		else if (this->deviceState->colorMode == CM_ALPHA_MAP)
		{
			shader = _SELECT_SHADER(this->deviceState->useTexture, this->deviceState->useColor, AlphaMap);
		}
		else if (this->deviceState->colorMode == CM_LERP)
		{
			shader = _SELECT_SHADER(this->deviceState->useTexture, this->deviceState->useColor, Lerp);
		}
		// change shaders
		bool shaderChanged = false;
		bool inputLayoutChanged = false;
		bool vertexShaderChanged = false;
		bool pixelShaderChanged = false;
		if (this->deviceState_shader != shader)
		{
			shaderChanged = true;
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
			this->deviceState_constantBufferChanged = true;
		}
	}

	void DirectX11_RenderSystem::_deviceClear(bool depth)
	{
		static const float clearColor[4] = {0.0f, 0.0f, 0.0f, 0.0f};
		// TODOa - should use current renderTargetView, not global one
		this->d3dDeviceContext->ClearRenderTargetView(this->renderTargetView.Get(), clearColor);
	}
	
	void DirectX11_RenderSystem::_deviceClear(Color color, bool depth)
	{
		static float clearColor[4] = {0.0f, 0.0f, 0.0f, 0.0f};
		clearColor[0] = color.b_f();
		clearColor[1] = color.g_f();
		clearColor[2] = color.r_f();
		clearColor[3] = color.a_f();
		// TODO - should use current renderTargetView, not global one
		this->d3dDeviceContext->ClearRenderTargetView(this->renderTargetView.Get(), clearColor);
	}

	void DirectX11_RenderSystem::_deviceClearDepth()
	{
		static const float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		// TODO - should use current renderTargetView, not global one
		this->d3dDeviceContext->ClearRenderTargetView(this->renderTargetView.Get(), clearColor);
	}

	void DirectX11_RenderSystem::_deviceRender(RenderOperation renderOperation, PlainVertex* v, int nVertices)
	{
		this->_setDX11VertexBuffer(renderOperation, v, nVertices, sizeof(PlainVertex));
		this->d3dDeviceContext->Draw(nVertices, 0);
	}

	void DirectX11_RenderSystem::_deviceRender(RenderOperation renderOperation, TexturedVertex* v, int nVertices)
	{
		this->_setDX11VertexBuffer(renderOperation, v, nVertices, sizeof(TexturedVertex));
		this->d3dDeviceContext->Draw(nVertices, 0);
	}

	void DirectX11_RenderSystem::_deviceRender(RenderOperation renderOperation, ColoredVertex* v, int nVertices)
	{
		this->_setDX11VertexBuffer(renderOperation, v, nVertices, sizeof(ColoredVertex));
		this->d3dDeviceContext->Draw(nVertices, 0);
	}

	void DirectX11_RenderSystem::_deviceRender(RenderOperation renderOperation, ColoredTexturedVertex* v, int nVertices)
	{
		this->_setDX11VertexBuffer(renderOperation, v, nVertices, sizeof(ColoredTexturedVertex));
		this->d3dDeviceContext->Draw(nVertices, 0);
	}

	void DirectX11_RenderSystem::_setDX11VertexBuffer(RenderOperation renderOperation, void* data, int nVertices, unsigned int vertexSize)
	{
		if (this->deviceState_renderOperation != renderOperation)
		{
			this->d3dDeviceContext->IASetPrimitiveTopology(_dx11RenderOperations[renderOperation]);
			this->deviceState_renderOperation = renderOperation;
		}
		unsigned int size = (unsigned int)(vertexSize * nVertices);
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

	Image::Format DirectX11_RenderSystem::getNativeTextureFormat(Image::Format format)
	{
		if (format == Image::FORMAT_RGBA || format == Image::FORMAT_ARGB || format == Image::FORMAT_BGRA || format == Image::FORMAT_ABGR)
		{
			return Image::FORMAT_BGRA;
		}
		if (format == Image::FORMAT_RGBX || format == Image::FORMAT_XRGB || format == Image::FORMAT_BGRX ||
			format == Image::FORMAT_XBGR || format == Image::FORMAT_RGB || format == Image::FORMAT_BGR)
		{
			return Image::FORMAT_BGRX;
		}
		if (format == Image::FORMAT_ALPHA)
		{
			return Image::FORMAT_ALPHA;
		}
		if (format == Image::FORMAT_GRAYSCALE)
		{
			return Image::FORMAT_GRAYSCALE;
		}
		if (format == Image::FORMAT_PALETTE)
		{
			return Image::FORMAT_PALETTE;
		}
		return Image::FORMAT_INVALID;
	}
	
	unsigned int DirectX11_RenderSystem::getNativeColorUInt(const april::Color& color)
	{
		return ((color.a << 24) | (color.b << 16) | (color.g << 8) | color.r);
	}

	Image* DirectX11_RenderSystem::takeScreenshot(Image::Format format)
	{
		// TODOa - if possible
		hlog::warn(logTag, "DirectX11_RenderSystem::takeScreenshot() not implemented!");
		return NULL;
	}
	
	void DirectX11_RenderSystem::presentFrame()
	{
		this->swapChain->Present(1, 0);
		// has to use GetAddressOf(), because the parameter is a pointer to an array of render target views
		this->d3dDeviceContext->OMSetRenderTargets(1, this->renderTargetView.GetAddressOf(), NULL);
	}

	void DirectX11_RenderSystem::updateOrientation()
	{
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
		ComPtr<IDXGIDevice3> dxgiDevice;
		HRESULT hr = this->d3dDevice.As(&dxgiDevice);
		if (FAILED(hr))
		{
			throw Exception("Unable to retrieve DXGI device!");
		}
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
