/// @file
/// @author  Boris Mikic
/// @version 2.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _DIRECTX11
#include <d3d11_1.h>
#include <stdio.h>

#include <gtypes/Vector2.h>
#include <hltypes/exception.h>
#include <hltypes/hlog.h>
#include <hltypes/hplatform.h>
#include <hltypes/hthread.h>

#include "april.h"
#include "DirectX11_PixelShader.h"
#include "DirectX11_RenderSystem.h"
#include "DirectX11_Texture.h"
#include "DirectX11_VertexShader.h"
#include "ImageSource.h"
#include "Keys.h"
#include "Platform.h"
#include "Timer.h"
#include "WinRT_Window.h"

#define VERTICES_BUFFER_COUNT 65536

#define UINT_RGBA_TO_ABGR(c) ((((c) >> 24) & 0xFF) | (((c) << 24) & 0xFF000000) | (((c) >> 8) & 0xFF00) | (((c) << 8) & 0xFF0000))

using namespace Microsoft::WRL;

namespace april
{
	static ColoredTexturedVertex static_ctv[VERTICES_BUFFER_COUNT];
	static ColoredVertex static_cv[VERTICES_BUFFER_COUNT];
	static TexturedVertex static_tv[VERTICES_BUFFER_COUNT];

	// TODO - refactor
	harray<DirectX11_Texture*> gRenderTargets;

	// TODO - refactor
	int DirectX11_RenderSystem::_getMaxTextureSize()
	{
		// depends on FEATURE_LEVEL, while 9.3 supports 4096, 9.2 and 9.1 support only 2048 so using 2048 is considered safe
		return D3D_FL9_1_REQ_TEXTURE1D_U_DIMENSION;
	}

	D3D11_PRIMITIVE_TOPOLOGY dx11_render_ops[]=
	{
		D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED,
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST,	// ROP_TRIANGLE_LIST
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP,	// ROP_TRIANGLE_STRIP
		D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED,		// triangle fans are deprecated in DX11
		D3D11_PRIMITIVE_TOPOLOGY_LINELIST,		// ROP_LINE_LIST
		D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP,		// ROP_LINE_STRIP
		D3D11_PRIMITIVE_TOPOLOGY_POINTLIST,		// ROP_POINT_LIST
	};

	unsigned int _numPrimitives(RenderOp renderOp, int nVertices)
	{
		switch (renderOp)
		{
		case TriangleList:
			return nVertices / 3;
		case TriangleStrip:
			return nVertices - 2;
		case TriangleFan:
			return nVertices - 1;
		case LineList:
			return nVertices / 2;
		case LineStrip:
			return nVertices - 1;
		case PointList:
			return nVertices;
		}
		return 0;
	}
	
	DirectX11_RenderSystem::DirectX11_RenderSystem() : RenderSystem(), zBufferEnabled(false),
		activeTextureBlendMode(DEFAULT), activeTexture(NULL), renderTarget(NULL),
		activeTextureColorMode(NORMAL), activeTextureColorModeAlpha(255),
		activeTextureFilter(Texture::FILTER_LINEAR), activeTextureAddressMode(Texture::ADDRESS_WRAP),
		matrixDirty(true)
	{
		this->name = APRIL_RS_DIRECTX11;
		this->d3dDevice = nullptr;
		this->d3dDeviceContext = nullptr;
		this->swapChain = nullptr;
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
		this->inputLayoutColored = nullptr;
		this->inputLayoutTextured = nullptr;
		this->inputLayoutColoredTextured = nullptr;
		this->vertexShaderPlain = NULL;
		this->pixelShaderPlain = NULL;
		this->vertexShaderColored = NULL;
		this->pixelShaderColored = NULL;
		this->vertexShaderTextured = NULL;
		this->vertexShaderTextured = NULL;
		this->vertexShaderColoredTextured = NULL;
		this->pixelShaderColoredTextured = NULL;
	}

	DirectX11_RenderSystem::~DirectX11_RenderSystem()
	{
		this->destroy();
	}

	bool DirectX11_RenderSystem::create(chstr options)
	{
		if (!RenderSystem::create(options))
		{
			return false;
		}
		this->zBufferEnabled = options.contains("zbuffer");
		this->activeTextureBlendMode = DEFAULT;
		this->activeTexture = NULL;
		this->renderTarget = NULL;
		this->activeTextureColorMode = NORMAL;
		this->activeTextureColorModeAlpha = 255;
		this->activeTextureFilter = Texture::FILTER_LINEAR;
		this->activeTextureAddressMode = Texture::ADDRESS_WRAP;
		this->matrixDirty = true;
		this->d3dDevice = nullptr;
		this->d3dDeviceContext = nullptr;
		this->swapChain = nullptr;
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
		this->inputLayoutColored = nullptr;
		this->inputLayoutTextured = nullptr;
		this->inputLayoutColoredTextured = nullptr;
		this->vertexShaderPlain = NULL;
		this->pixelShaderPlain = NULL;
		this->vertexShaderColored = NULL;
		this->pixelShaderColored = NULL;
		this->vertexShaderTextured = NULL;
		this->vertexShaderTextured = NULL;
		this->vertexShaderColoredTextured = NULL;
		this->pixelShaderColoredTextured = NULL;
		return true;
	}

	bool DirectX11_RenderSystem::destroy()
	{
		if (!RenderSystem::destroy())
		{
			return false;
		}
		_HL_TRY_DELETE(this->vertexShaderPlain);
		_HL_TRY_DELETE(this->pixelShaderPlain);
		_HL_TRY_DELETE(this->vertexShaderColored);
		_HL_TRY_DELETE(this->pixelShaderColored);
		_HL_TRY_DELETE(this->vertexShaderTextured);
		_HL_TRY_DELETE(this->pixelShaderTextured);
		_HL_TRY_DELETE(this->vertexShaderColoredTextured);
		_HL_TRY_DELETE(this->pixelShaderColoredTextured);
		this->inputLayoutPlain = nullptr;
		this->inputLayoutColored = nullptr;
		this->inputLayoutTextured = nullptr;
		this->inputLayoutColoredTextured = nullptr;
		this->vertexBuffer = nullptr;
		this->constantBuffer = nullptr;
		this->samplerLinearWrap = nullptr;
		this->samplerLinearClamp = nullptr;
		this->samplerNearestWrap = nullptr;
		this->samplerNearestClamp = nullptr;
		this->blendStateAlpha = nullptr;
		this->blendStateAdd = nullptr;
		this->blendStateSubtract = nullptr;
		this->blendStateOverwrite = nullptr;
		this->renderTargetView = nullptr;
		this->rasterState = nullptr;
		this->swapChain = nullptr;
		this->d3dDeviceContext = nullptr;
		this->d3dDevice = nullptr;
		return true;
	}

	void DirectX11_RenderSystem::assignWindow(Window* window)
	{
		unsigned int creationFlags = 0;
#ifdef _DEBUG
		creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#else
		creationFlags |= D3D11_CREATE_DEVICE_PREVENT_ALTERING_LAYER_SETTINGS_FROM_REGISTRY; // prevents debug hooks and hacking
#endif
		D3D_FEATURE_LEVEL featureLevels[] =
		{
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0,
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
			throw hl_exception("Unable to create DX11 device!");
		}
		hr = _d3dDevice.As(&this->d3dDevice);
		if (FAILED(hr))
		{
			throw hl_exception("Unable to retrieve Direct3D 11.1 device interface!");
		}
		hr = _d3dDeviceContext.As(&this->d3dDeviceContext);
		if (FAILED(hr))
		{
			throw hl_exception("Unable to retrieve Direct3D 11.1 device context interface!");
		}
		// device config
		this->_configureDevice();
		// has to use GetAddressOf(), because the parameter is a pointer to an array of render target views
		this->d3dDeviceContext->OMSetRenderTargets(1, this->renderTargetView.GetAddressOf(), NULL);
		// initial vertex buffer data
		this->vertexBufferData.pSysMem = NULL;
		this->vertexBufferData.SysMemPitch = 0;
		this->vertexBufferData.SysMemSlicePitch = 0;
		this->vertexBufferDesc.ByteWidth = 0;
		this->vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		this->vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		this->vertexBufferDesc.CPUAccessFlags = 0;
		this->vertexBufferDesc.MiscFlags = 0;
		this->vertexBufferDesc.StructureByteStride = 0;
		// initial constant buffer
		D3D11_SUBRESOURCE_DATA constantSubresourceData = {0};
		constantSubresourceData.pSysMem = &this->constantBufferData;
		constantSubresourceData.SysMemPitch = 0;
		constantSubresourceData.SysMemSlicePitch = 0;
		D3D11_BUFFER_DESC constantBufferDesc = {0};
		constantBufferDesc.ByteWidth = sizeof(this->constantBufferData);
		constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		constantBufferDesc.CPUAccessFlags = 0;
		constantBufferDesc.MiscFlags = 0;
		constantBufferDesc.StructureByteStride = 0;
		hr = this->d3dDevice->CreateBuffer(&constantBufferDesc, &constantSubresourceData, &this->constantBuffer);
		if (FAILED(hr))
		{
			throw hl_exception("Unable to create constant buffer!");
		}
		this->matrixDirty = true;
		// initial calls
		this->clear(true, false);
		this->presentFrame();
		this->orthoProjection.setSize((float)window->getWidth(), (float)window->getHeight());
		// default shaders
		if (this->vertexShaderPlain == NULL)
		{
			this->vertexShaderPlain = (DirectX11_VertexShader*)this->createVertexShader("april/VertexShader_Plain.cso");
		}
		if (this->pixelShaderPlain == NULL)
		{
			this->pixelShaderPlain = (DirectX11_PixelShader*)this->createPixelShader("april/PixelShader_Plain.cso");
		}
		if (this->vertexShaderTextured == NULL)
		{
			this->vertexShaderTextured = (DirectX11_VertexShader*)this->createVertexShader("april/VertexShader_Textured.cso");
		}
		if (this->pixelShaderTextured == NULL)
		{
			this->pixelShaderTextured = (DirectX11_PixelShader*)this->createPixelShader("april/PixelShader_Textured.cso");
		}
		if (this->vertexShaderColored == NULL)
		{
			this->vertexShaderColored = (DirectX11_VertexShader*)this->createVertexShader("april/VertexShader_Colored.cso");
		}
		if (this->pixelShaderColored == NULL)
		{
			this->pixelShaderColored = (DirectX11_PixelShader*)this->createPixelShader("april/PixelShader_Colored.cso");
		}
		if (this->vertexShaderColoredTextured == NULL)
		{
			this->vertexShaderColoredTextured = (DirectX11_VertexShader*)this->createVertexShader("april/VertexShader_ColoredTextured.cso");
		}
		if (this->pixelShaderColoredTextured == NULL)
		{
			this->pixelShaderColoredTextured = (DirectX11_PixelShader*)this->createPixelShader("april/PixelShader_ColoredTextured.cso");
		}
		// input layouts for default shaders
		D3D11_INPUT_ELEMENT_DESC _position = {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0};
		D3D11_INPUT_ELEMENT_DESC _color = {"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0};
		if (this->inputLayoutPlain == nullptr)
		{
			const D3D11_INPUT_ELEMENT_DESC inputLayoutDescPlain[] =
			{
				_position,
			};
			hr = this->d3dDevice->CreateInputLayout(inputLayoutDescPlain, ARRAYSIZE(inputLayoutDescPlain),
				this->vertexShaderPlain->shaderData, this->vertexShaderPlain->shaderSize, &this->inputLayoutPlain);
			if (FAILED(hr))
			{
				throw hl_exception("Unable to create input layout for plain shader!");
			}
		}
		if (this->inputLayoutColored == nullptr)
		{
			const D3D11_INPUT_ELEMENT_DESC inputLayoutDescColored[] =
			{
				_position,
				_color,
			};
			hr = this->d3dDevice->CreateInputLayout(inputLayoutDescColored, ARRAYSIZE(inputLayoutDescColored),
				this->vertexShaderColored->shaderData, this->vertexShaderColored->shaderSize, &this->inputLayoutColored);
			if (FAILED(hr))
			{
				throw hl_exception("Unable to create input layout for colored shader!");
			}
		}
		if (this->inputLayoutTextured == nullptr)
		{
			const D3D11_INPUT_ELEMENT_DESC inputLayoutDescTextured[] =
			{
				_position,
				{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
			};
			hr = this->d3dDevice->CreateInputLayout(inputLayoutDescTextured, ARRAYSIZE(inputLayoutDescTextured),
				this->vertexShaderTextured->shaderData, this->vertexShaderTextured->shaderSize, &this->inputLayoutTextured);
			if (FAILED(hr))
			{
				throw hl_exception("Unable to create input layout for textured shader!");
			}
		}
		if (this->inputLayoutColoredTextured == nullptr)
		{
			const D3D11_INPUT_ELEMENT_DESC inputLayoutDescColoredTextured[] =
			{
				_position,
				_color,
				{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0},
			};
			hr = this->d3dDevice->CreateInputLayout(inputLayoutDescColoredTextured, ARRAYSIZE(inputLayoutDescColoredTextured),
				this->vertexShaderColoredTextured->shaderData, this->vertexShaderColoredTextured->shaderSize, &this->inputLayoutColoredTextured);
			if (FAILED(hr))
			{
				throw hl_exception("Unable to create input layout for colored-textured shader!");
			}
		}
	}

	void DirectX11_RenderSystem::_createSwapChain(int width, int height)
	{
		// Once the swap chain desc is configured, it must be
		// created on the same adapter as the existing D3D Device.
		HRESULT hr;
		ComPtr<IDXGIDevice2> dxgiDevice;
		hr = this->d3dDevice.As(&dxgiDevice);
		if (FAILED(hr))
		{
			throw hl_exception("Unable to retrieve DXGI device!");
		}
		hr = dxgiDevice->SetMaximumFrameLatency(1);
		if (FAILED(hr))
		{
			throw hl_exception("Unable to set MaximumFrameLatency!");
		}
		ComPtr<IDXGIAdapter> dxgiAdapter;
		hr = dxgiDevice->GetAdapter(&dxgiAdapter);
		if (FAILED(hr))
		{
			throw hl_exception("Unable to get adapter from DXGI device!");
		}
		ComPtr<IDXGIFactory2> dxgiFactory;
		hr = dxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory));
		if (FAILED(hr))
		{
			throw hl_exception("Unable to get parent factory from DXGI adapter!");
		}
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {0};
		swapChainDesc.Stereo = false;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.Scaling = DXGI_SCALING_NONE;
		swapChainDesc.Flags = 0;
		swapChainDesc.Width = width;
		swapChainDesc.Height = height;
		swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferCount = 2;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
		hr = dxgiFactory->CreateSwapChainForCoreWindow(this->d3dDevice.Get(),
			reinterpret_cast<IUnknown*>(april::WinRT::View->getCoreWindow()), &swapChainDesc, NULL, &this->swapChain);
		if (FAILED(hr))
		{
			throw hl_exception("Unable to create swap chain!");
		}
	}

	void DirectX11_RenderSystem::_configureDevice()
	{
		HRESULT hr;
		if (this->swapChain != nullptr)
		{
			hr = this->swapChain->ResizeBuffers(2, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, 0);
			if (FAILED(hr))
			{
				throw hl_exception("Unable to resize swap chain buffers!");
			}
		}
		else
		{
			this->_createSwapChain(april::window->getWidth(), april::window->getHeight());
		}
		ComPtr<ID3D11Texture2D> _backBuffer;
		hr = this->swapChain->GetBuffer(0, IID_PPV_ARGS(&_backBuffer));
		if (FAILED(hr))
		{
			throw hl_exception("Unable to get swap chain back buffer!");
		}
		hr = this->d3dDevice->CreateRenderTargetView(_backBuffer.Get(), NULL, &this->renderTargetView);
		if (FAILED(hr))
		{
			throw hl_exception("Unable to create render target view!");
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
		hr = this->d3dDevice->CreateRasterizerState(&rasterDesc, &this->rasterState);
		if (FAILED(hr))
		{
			throw hl_exception("Unable to create raster state!");
		}
		this->d3dDeviceContext->RSSetState(this->rasterState.Get());
		D3D11_TEXTURE2D_DESC backBufferDesc = {0};
		_backBuffer->GetDesc(&backBufferDesc);
		this->setViewport(grect(0.0f, 0.0f, (float)backBufferDesc.Width, (float)backBufferDesc.Height));
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
		this->setTextureBlendMode(DEFAULT);
		this->setTextureColorMode(NORMAL);
		this->setTextureAddressMode(Texture::ADDRESS_WRAP);
		this->setTextureFilter(Texture::FILTER_LINEAR);
	}

	harray<DisplayMode> DirectX11_RenderSystem::getSupportedDisplayModes()
	{
		if (this->supportedDisplayModes.size() == 0)
		{
			gvec2 resolution = april::getSystemInfo().displayResolution;
			DisplayMode displayMode;
			displayMode.width = (int)resolution.x;
			displayMode.height = (int)resolution.y;
			displayMode.refreshRate = 60;
			this->supportedDisplayModes += displayMode;
		}
		return this->supportedDisplayModes;
	}

	grect DirectX11_RenderSystem::getViewport()
	{
		D3D11_VIEWPORT viewport;
		unsigned int count = 1;
		this->d3dDeviceContext->RSGetViewports(&count, &viewport);
		return grect((float)viewport.TopLeftX, (float)viewport.TopLeftY, (float)viewport.Width - 1, (float)viewport.Height - 1);
	}

	void DirectX11_RenderSystem::setViewport(grect rect)
	{
		D3D11_VIEWPORT viewport;
		viewport.MinDepth = D3D11_MIN_DEPTH;
		viewport.MaxDepth = D3D11_MAX_DEPTH;
		// these double-casts are to ensure consistent behavior among rendering systems
		viewport.TopLeftX = (float)(int)rect.x;
		viewport.TopLeftY = (float)(int)rect.y;
		viewport.Width = (float)(int)rect.w + 1;
		viewport.Height = (float)(int)rect.h + 1;
		this->d3dDeviceContext->RSSetViewports(1, &viewport);
	}

	void DirectX11_RenderSystem::setTextureBlendMode(BlendMode textureBlendMode)
	{
		switch (textureBlendMode)
		{
		case DEFAULT:
		case ALPHA_BLEND:
		case ADD:
		case SUBTRACT:
		case OVERWRITE:
			this->activeTextureBlendMode = textureBlendMode;
			break;
		default:
			hlog::warn(april::logTag, "Trying to set unsupported texture blend mode!");
			break;
		}
	}

	void DirectX11_RenderSystem::setTextureColorMode(ColorMode textureColorMode, unsigned char alpha)
	{
		this->activeTextureColorModeAlpha = 255;
		switch (textureColorMode)
		{
		case LERP: // LERP also needs alpha
			this->activeTextureColorModeAlpha = alpha;
		case NORMAL:
		case MULTIPLY:
		case ALPHA_MAP:
			this->activeTextureColorMode = textureColorMode;
			break;
		default:
			hlog::warn(april::logTag, "Trying to set unsupported texture color mode!");
			break;
		}
	}

	void DirectX11_RenderSystem::setTextureFilter(Texture::Filter textureFilter)
	{
		switch (textureFilter)
		{
		case Texture::FILTER_LINEAR:
		case Texture::FILTER_NEAREST:
			this->activeTextureFilter = textureFilter;
			break;
		default:
			hlog::warn(april::logTag, "Trying to set unsupported texture filter!");
			break;
		}
	}

	void DirectX11_RenderSystem::setTextureAddressMode(Texture::AddressMode textureAddressMode)
	{
		switch (textureAddressMode)
		{
		case Texture::ADDRESS_WRAP:
		case Texture::ADDRESS_CLAMP:
			this->textureAddressMode = textureAddressMode;
			break;
		default:
			hlog::warn(april::logTag, "Trying to set unsupported texture address mode!");
			break;
		}
	}

	void DirectX11_RenderSystem::setTexture(Texture* texture)
	{
		this->activeTexture = (DirectX11_Texture*)texture;
		if (this->activeTexture != NULL)
		{
			Texture::Filter filter = this->activeTexture->getFilter();
			if (this->textureFilter != filter)
			{
				this->setTextureFilter(filter);
			}
			Texture::AddressMode addressMode = this->activeTexture->getAddressMode();
			if (this->textureAddressMode != addressMode)
			{
				this->setTextureAddressMode(addressMode);
			}
			this->activeTexture->load();
		}
	}

	Texture* DirectX11_RenderSystem::getRenderTarget()
	{
		return this->renderTarget;
	}
	
	void DirectX11_RenderSystem::setRenderTarget(Texture* source)
	{
		// TODO - test, this code is experimental
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
			hlog::error(april::logTag, "Texture not created as rendertarget: " + texture->_getInternalName());
			return;
		}
		this->renderTarget = texture;
	}
	
	void DirectX11_RenderSystem::setPixelShader(PixelShader* pixelShader)
	{
		this->activePixelShader = (DirectX11_PixelShader*)pixelShader;
	}

	void DirectX11_RenderSystem::setVertexShader(VertexShader* vertexShader)
	{
		this->activeVertexShader = (DirectX11_VertexShader*)vertexShader;
	}

	void DirectX11_RenderSystem::_setPixelShader(DirectX11_PixelShader* pixelShader)
	{
		if (this->activePixelShader != NULL)
		{
			pixelShader = this->activePixelShader;
		}
		this->d3dDeviceContext->PSSetShader(pixelShader->dx11Shader.Get(), NULL, 0);
	}

	void DirectX11_RenderSystem::_setVertexShader(DirectX11_VertexShader* vertexShader)
	{
		if (this->activeVertexShader != NULL)
		{
			vertexShader = this->activeVertexShader;
		}
		this->d3dDeviceContext->VSSetShader(vertexShader->dx11Shader.Get(), NULL, 0);
	}

	void DirectX11_RenderSystem::setResolution(int w, int h)
	{
		RenderSystem::setResolution(w, h);
		hlog::writef(april::logTag, "Resetting device for %d x %d...", april::window->getWidth(), april::window->getHeight());
		this->_createSwapChain(w, h);
		this->_setModelviewMatrix(this->modelviewMatrix);
		this->_setProjectionMatrix(this->projectionMatrix);
	}

	Texture* DirectX11_RenderSystem::_createTexture(chstr filename)
	{
		return new DirectX11_Texture(filename);
	}

	Texture* DirectX11_RenderSystem::createTexture(int w, int h, unsigned char* rgba)
	{
		return new DirectX11_Texture(w, h, rgba);
	}
	
	Texture* DirectX11_RenderSystem::createTexture(int w, int h, Texture::Format format, Texture::Type type, Color color)
	{
		return new DirectX11_Texture(w, h, format, type, color);
	}
	
	PixelShader* DirectX11_RenderSystem::createPixelShader()
	{
		return new DirectX11_PixelShader();
	}

	PixelShader* DirectX11_RenderSystem::createPixelShader(chstr filename)
	{
		return new DirectX11_PixelShader(filename);
	}

	VertexShader* DirectX11_RenderSystem::createVertexShader()
	{
		return new DirectX11_VertexShader();
	}

	VertexShader* DirectX11_RenderSystem::createVertexShader(chstr filename)
	{
		return new DirectX11_VertexShader(filename);
	}

	void DirectX11_RenderSystem::clear(bool useColor, bool depth)
	{
		static const float clearColor[4] = {0.0f, 0.0f, 0.0f, 0.0f};
		// TODO - should use current renderTargetView, not global one
		this->d3dDeviceContext->ClearRenderTargetView(this->renderTargetView.Get(), clearColor);
	}
	
	void DirectX11_RenderSystem::clear(bool depth, grect rect, Color color)
	{
		const float clearColor[4] = {color.b_f(), color.g_f(), color.r_f(), color.a_f()};
		D3D11_RECT area;
		area.left = (int)rect.x;
		area.top = (int)rect.y;
		area.right = (int)(rect.x + rect.w);
		area.bottom = (int)(rect.y + rect.h);
		// TODO - should use current renderTargetView, not global one
		this->d3dDeviceContext->ClearView(this->renderTargetView.Get(), clearColor, &area, 1);
	}

	void DirectX11_RenderSystem::_updateVertexBuffer(int vertexSize, int nVertices, void* data)
	{
		static unsigned int size;
		size = (unsigned int)(vertexSize * nVertices);
		if (size > this->vertexBufferDesc.ByteWidth)
		{
			this->vertexBuffer = nullptr;
			this->vertexBufferData.pSysMem = data;
			this->vertexBufferDesc.ByteWidth = size;
			this->d3dDevice->CreateBuffer(&this->vertexBufferDesc, &this->vertexBufferData, &this->vertexBuffer);
		}
		else
		{
			// the usage of a box is required, because otherwise updating could cause an access violation when trying to read the data after the valid buffer
			static D3D11_BOX box;
			box.left = 0;
			box.right = size;
			box.top = 0;
			box.bottom = 1;
			box.front = 0;
			box.back = 1;
			this->d3dDeviceContext->UpdateSubresource1(this->vertexBuffer.Get(), 0, &box, data, 0, 0, D3D11_COPY_DISCARD);
		}
		static unsigned int stride;
		static unsigned int offset;
		stride = (unsigned int)vertexSize;
		offset = 0;
		this->d3dDeviceContext->IASetVertexBuffers(0, 1, this->vertexBuffer.GetAddressOf(), &stride, &offset);
	}
	
	void DirectX11_RenderSystem::_updateConstantBuffer(Color color)
	{
		if (this->matrixDirty)
		{
			this->matrixDirty = false;
			this->constantBufferData.matrix = (this->projectionMatrix * this->modelviewMatrix).transposed();
		}
		this->constantBufferData.color.set(color.r_f(), color.g_f(), color.b_f(), color.a_f());
		this->constantBufferData.colorModeData.set((float)this->activeTextureColorMode, this->activeTextureColorModeAlpha / 255.0f, 0.0f, 0.0f);
        this->d3dDeviceContext->UpdateSubresource(this->constantBuffer.Get(), 0, NULL, &this->constantBufferData, 0, 0);
		this->d3dDeviceContext->VSSetConstantBuffers(0, 1, this->constantBuffer.GetAddressOf());
	}

	void DirectX11_RenderSystem::_updateBlending()
	{
		static float blendFactor[4] = {1.0f, 1.0f, 1.0f, 1.0f};
		switch (this->activeTextureBlendMode)
		{
		case DEFAULT:
		case ALPHA_BLEND:
			this->d3dDeviceContext->OMSetBlendState(this->blendStateAlpha.Get(), blendFactor, 0xFFFFFFFF);
			break;
		case ADD:
			this->d3dDeviceContext->OMSetBlendState(this->blendStateAdd.Get(), blendFactor, 0xFFFFFFFF);
			break;
		case SUBTRACT:
			this->d3dDeviceContext->OMSetBlendState(this->blendStateSubtract.Get(), blendFactor, 0xFFFFFFFF);
			break;
		case OVERWRITE:
			this->d3dDeviceContext->OMSetBlendState(this->blendStateOverwrite.Get(), blendFactor, 0xFFFFFFFF);
			break;
		}
	}

	void DirectX11_RenderSystem::_updateTexture(bool use)
	{
		if (use && this->activeTexture != NULL)
		{
            this->d3dDeviceContext->PSSetShaderResources(0, 1, this->activeTexture->d3dView.GetAddressOf());
			if (this->activeTextureFilter == Texture::FILTER_LINEAR &&
				this->activeTextureAddressMode == Texture::ADDRESS_WRAP)
			{
				this->d3dDeviceContext->PSSetSamplers(0, 1, this->samplerLinearWrap.GetAddressOf());
			}
			else if (this->activeTextureFilter == Texture::FILTER_LINEAR &&
				this->activeTextureAddressMode == Texture::ADDRESS_CLAMP)
			{
				this->d3dDeviceContext->PSSetSamplers(0, 1, this->samplerLinearClamp.GetAddressOf());
			}
			else if (this->activeTextureFilter == Texture::FILTER_NEAREST &&
				this->activeTextureAddressMode == Texture::ADDRESS_WRAP)
			{
				this->d3dDeviceContext->PSSetSamplers(0, 1, this->samplerNearestWrap.GetAddressOf());
			}
			else if (this->activeTextureFilter == Texture::FILTER_NEAREST &&
				this->activeTextureAddressMode == Texture::ADDRESS_CLAMP)
			{
				this->d3dDeviceContext->PSSetSamplers(0, 1, this->samplerNearestClamp.GetAddressOf());
			}
		}
	}
	
	void DirectX11_RenderSystem::render(RenderOp renderOp, PlainVertex* v, int nVertices)
	{
		this->render(renderOp, v, nVertices, april::Color::White);
	}

	void DirectX11_RenderSystem::render(RenderOp renderOp, PlainVertex* v, int nVertices, Color color)
	{
		this->d3dDeviceContext->IASetPrimitiveTopology(dx11_render_ops[renderOp]);
		this->d3dDeviceContext->IASetInputLayout(this->inputLayoutPlain.Get());
		this->_updateVertexBuffer(sizeof(PlainVertex), nVertices, v);
		this->_setVertexShader(this->vertexShaderPlain);
		this->_setPixelShader(this->pixelShaderPlain);
		this->_updateConstantBuffer(color);
		this->_updateBlending();
		this->_updateTexture(false);
		this->d3dDeviceContext->Draw(nVertices, 0);
	}
	
	void DirectX11_RenderSystem::render(RenderOp renderOp, TexturedVertex* v, int nVertices)
	{
		this->render(renderOp, v, nVertices, april::Color::White);
	}

	void DirectX11_RenderSystem::render(RenderOp renderOp, TexturedVertex* v, int nVertices, Color color)
	{
		TexturedVertex* tv = (nVertices <= VERTICES_BUFFER_COUNT) ? static_tv : new TexturedVertex[nVertices];
		memcpy(tv, v, sizeof(TexturedVertex) * nVertices);
		this->d3dDeviceContext->IASetPrimitiveTopology(dx11_render_ops[renderOp]);
		this->d3dDeviceContext->IASetInputLayout(this->inputLayoutTextured.Get());
		this->_updateVertexBuffer(sizeof(TexturedVertex), nVertices, v);
		this->_setVertexShader(this->vertexShaderTextured);
		this->_setPixelShader(this->pixelShaderTextured);
		this->_updateConstantBuffer(color);
		this->_updateBlending();
		this->_updateTexture(true);
		this->d3dDeviceContext->Draw(nVertices, 0);
		if (nVertices > VERTICES_BUFFER_COUNT)
		{
			delete [] tv;
		}
	}

	void DirectX11_RenderSystem::render(RenderOp renderOp, ColoredVertex* v, int nVertices)
	{
		ColoredVertex* cv = (nVertices <= VERTICES_BUFFER_COUNT) ? static_cv : new ColoredVertex[nVertices];
		memcpy(cv, v, sizeof(ColoredVertex) * nVertices);
		for_iter (i, 0, nVertices)
		{
			cv[i].color = UINT_RGBA_TO_ABGR(v[i].color);
		}
		this->d3dDeviceContext->IASetPrimitiveTopology(dx11_render_ops[renderOp]);
		this->d3dDeviceContext->IASetInputLayout(this->inputLayoutColored.Get());
		this->_updateVertexBuffer(sizeof(ColoredVertex), nVertices, cv);
		this->_setVertexShader(this->vertexShaderColored);
		this->_setPixelShader(this->pixelShaderColored);
		this->_updateConstantBuffer(april::Color::White);
		this->_updateBlending();
		this->_updateTexture(false);
		this->d3dDeviceContext->Draw(nVertices, 0);
		if (nVertices > VERTICES_BUFFER_COUNT)
		{
			delete [] cv;
		}
	}

	void DirectX11_RenderSystem::render(RenderOp renderOp, ColoredTexturedVertex* v, int nVertices)
	{
		ColoredTexturedVertex* ctv = (nVertices <= VERTICES_BUFFER_COUNT) ? static_ctv : new ColoredTexturedVertex[nVertices];
		memcpy(ctv, v, sizeof(ColoredTexturedVertex) * nVertices);
		for_iter (i, 0, nVertices)
		{
			ctv[i].color = UINT_RGBA_TO_ABGR(v[i].color);
		}
		this->d3dDeviceContext->IASetPrimitiveTopology(dx11_render_ops[renderOp]);
		this->d3dDeviceContext->IASetInputLayout(this->inputLayoutColoredTextured.Get());
		this->_updateVertexBuffer(sizeof(ColoredTexturedVertex), nVertices, ctv);
		this->_setVertexShader(this->vertexShaderColoredTextured);
		this->_setPixelShader(this->pixelShaderColoredTextured);
		this->_updateConstantBuffer(april::Color::White);
		this->_updateBlending();
		this->_updateTexture(true);
		this->d3dDeviceContext->Draw(nVertices, 0);
		if (nVertices > VERTICES_BUFFER_COUNT)
		{
			delete [] ctv;
		}
	}

	void DirectX11_RenderSystem::_setModelviewMatrix(const gmat4& matrix)
	{
		this->matrixDirty = true;
	}

	void DirectX11_RenderSystem::_setProjectionMatrix(const gmat4& matrix)
	{
		this->matrixDirty = true;
	}

	ImageSource* DirectX11_RenderSystem::takeScreenshot(int bpp)
	{
		// TODO
		hlog::warn(april::logTag, "DirectX11_RenderSystem::takeScreenshot()");
		/*
#ifdef _DEBUG
		hlog::write(april::logTag, "Grabbing screenshot...");
#endif
		D3DSURFACE_DESC desc;
		this->backBuffer->GetDesc(&desc);
		if (desc.Format != D3DFMT_X8R8G8B8)
		{
			hlog::error(april::logTag, "Failed to grab screenshot, backbuffer format not supported, expected X8R8G8B8, got: " + hstr(desc.Format));
			return NULL;
		}
		IDirect3DSurface9* buffer;
		HRESULT hr = this->d3dDevice->CreateOffscreenPlainSurface(desc.Width, desc.Height, desc.Format, D3DPOOL_SYSTEMMEM, &buffer, NULL);
		if (hr != D3D_OK)
		{
			hlog::error(april::logTag, "Failed to grab screenshot, CreateOffscreenPlainSurface() call failed.");
			return NULL;
		}
		hr = this->d3dDevice->GetRenderTargetData(this->backBuffer, buffer);
		if (hr != D3D_OK)
		{
			hlog::error(april::logTag, "Failed to grab screenshot, GetRenderTargetData() call failed.");
			buffer->Release();
			return NULL;
		}		
		D3DLOCKED_RECT rect;
		hr = buffer->LockRect(&rect, NULL, D3DLOCK_DONOTWAIT);
		if (hr != D3D_OK)
		{
			hlog::error(april::logTag, "Failed to grab screenshot, surface lock failed.");
			buffer->Release();
			return NULL;
		}
		
		ImageSource* img = new ImageSource();
		img->w = desc.Width;
		img->h = desc.Height;
		img->bpp = bpp;
		img->format = (bpp == 4 ? AF_RGBA : AF_RGB);
		img->data = new unsigned char[img->w * img->h * img->bpp];
		unsigned char* p = img->data;
		unsigned char* src = (unsigned char*)rect.pBits;
		int x;
		memset(p, 255, img->w * img->h * img->bpp);
		for_iter (y, 0, img->h)
		{
			for (x = 0; x < img->w; x++, p += bpp)
			{
				p[0] = src[x * bpp + 2];
				p[1] = src[x * bpp + 1];
				p[2] = src[x * bpp];
			}
			src += rect.Pitch;
		}
		buffer->UnlockRect();
		buffer->Release();
		return img;
		*/
		return NULL;
	}
	
	void DirectX11_RenderSystem::presentFrame()
	{
		this->swapChain->Present(0, 0);
		// has to use GetAddressOf(), because the parameter is a pointer to an array of render target views
		this->d3dDeviceContext->OMSetRenderTargets(1, this->renderTargetView.GetAddressOf(), NULL);
	}

}

#endif
