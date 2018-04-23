/// @file
/// @version 5.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifdef _DIRECTX9
#include <stdio.h>

#define __HL_INCLUDE_PLATFORM_HEADERS
#include <gtypes/Vector2.h>
#include <hltypes/hexception.h>
#include <hltypes/hlog.h>
#include <hltypes/hplatform.h>
#include <hltypes/hthread.h>

#include "Application.h"
#include "april.h"
#include "DirectX9_PixelShader.h"
#include "DirectX9_RenderSystem.h"
#include "DirectX9_Texture.h"
#include "DirectX9_VertexShader.h"
#include "Image.h"
#include "Keys.h"
#include "Platform.h"
#include "RenderCommand.h"
#include "RenderState.h"
#include "Timer.h"
#include "Win32_Window.h"

#define VERTICES_BUFFER_COUNT 65536
#define APRIL_DX9_CHILD L"AprilDX9Child"

#define IS_WINDOW_RESIZABLE (april::window->getName() == april::WindowType::Win32.getName() && april::window->getOptions().resizable)

namespace april
{
	static ColoredTexturedVertex static_ctv[VERTICES_BUFFER_COUNT];
	static ColoredVertex static_cv[VERTICES_BUFFER_COUNT];

	D3DPRIMITIVETYPE DirectX9_RenderSystem::_dx9RenderOperations[]=
	{
		D3DPT_TRIANGLELIST,
		D3DPT_TRIANGLESTRIP,
		D3DPT_LINELIST,
		D3DPT_LINESTRIP,
		D3DPT_POINTLIST,
		D3DPT_TRIANGLEFAN,
	};

	DirectX9_RenderSystem::DirectX9_RenderSystem() : DirectX_RenderSystem(), d3d(NULL), d3dDevice(NULL), d3dpp(NULL), backBuffer(NULL), childHWnd(0), renderTarget(NULL)
	{
		this->name = april::RenderSystemType::DirectX9.getName();
		this->pixelOffset = 0.5f;
		this->_supportsA8Surface = false;
		this->_deviceInit();
	}

	DirectX9_RenderSystem::~DirectX9_RenderSystem()
	{
		this->destroy();
	}

	void DirectX9_RenderSystem::_deviceInit()
	{
		this->d3d = NULL;
		this->d3dDevice = NULL;
		this->d3dpp = NULL;
		this->backBuffer = NULL;
		this->childHWnd = 0;
		this->renderTarget = NULL;
	}

	bool DirectX9_RenderSystem::_deviceCreate(RenderSystem::Options options)
	{
		this->d3d = Direct3DCreate9(D3D_SDK_VERSION);
		if (this->d3d == NULL)
		{
			hlog::error(logTag, "Unable to create Direct3D9 object!");
			return false;
		}
		this->d3dpp = new _D3DPRESENT_PARAMETERS_();
		return true;
	}

	bool DirectX9_RenderSystem::_deviceDestroy()
	{
		if (this->d3dpp != NULL)
		{
			delete this->d3dpp;
		}
		if (this->d3dDevice != NULL)
		{
			this->d3dDevice->Release();
		}
		if (this->d3d != NULL)
		{
			this->d3d->Release();
		}
		if (this->childHWnd != 0)
		{
			DestroyWindow(this->childHWnd);
			UnregisterClassW(APRIL_DX9_CHILD, GetModuleHandle(0));
		}
		return true;
	}

	void DirectX9_RenderSystem::_deviceAssignWindow(Window* window)
	{
		HWND hWnd = (HWND)window->getBackendId();
		memset(this->d3dpp, 0, sizeof(*this->d3dpp));
		bool resizable = IS_WINDOW_RESIZABLE;
		this->d3dpp->Windowed = !april::window->isFullscreen();
		int w = april::window->getWidth();
		int h = april::window->getHeight();
		if (resizable)
		{
			SystemInfo info = april::getSystemInfo();
			w = (int)info.displayResolution.x;
			h = (int)info.displayResolution.y;
		}
		this->d3dpp->BackBufferWidth = w;
		this->d3dpp->BackBufferHeight = h;
		this->d3dpp->BackBufferFormat = D3DFMT_X8R8G8B8;
		this->d3dpp->PresentationInterval = (this->options.vSync ? D3DPRESENT_INTERVAL_ONE : D3DPRESENT_INTERVAL_IMMEDIATE);
		if (this->options.depthBuffer)
		{
			this->d3dpp->EnableAutoDepthStencil = TRUE;
			this->d3dpp->AutoDepthStencilFormat = D3DFMT_D16;
		}
		this->d3dpp->SwapEffect = D3DSWAPEFFECT_COPY; // COPY is being used here as otherwise some weird tearing manifests during rendering
		this->d3dpp->hDeviceWindow = hWnd;
		if (resizable)
		{
			WNDCLASSEXW wc;
			memset(&wc, 0, sizeof(WNDCLASSEX));
			wc.cbSize = sizeof(WNDCLASSEX);
			wc.lpfnWndProc = &Win32_Window::childProcessCallback;
			wc.hInstance = GetModuleHandle(0);
			wc.hCursor = GetCursor();
			wc.lpszClassName = APRIL_DX9_CHILD;
			wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
			RegisterClassExW(&wc);
			this->childHWnd = CreateWindowW(APRIL_DX9_CHILD, APRIL_DX9_CHILD, WS_CHILD, 0, 0, w, h, hWnd, NULL, wc.hInstance, NULL);
			if (!window->isFullscreen())
			{
				this->_tryAssignChildWindow();
			}
		}
		HRESULT hr = this->d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_FPU_PRESERVE | D3DCREATE_HARDWARE_VERTEXPROCESSING, this->d3dpp, &this->d3dDevice);
		if (FAILED(hr))
		{
			hr = this->d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_FPU_PRESERVE | D3DCREATE_SOFTWARE_VERTEXPROCESSING, this->d3dpp, &this->d3dDevice);
			if (FAILED(hr))
			{
				throw Exception("Unable to create Direct3D Device!");
			}
		}
		if (!this->_supportsA8Surface)
		{
			IDirect3DSurface9* surface = NULL;
			HRESULT hr = this->d3dDevice->CreateOffscreenPlainSurface(16, 16, D3DFMT_A8, D3DPOOL_SYSTEMMEM, &surface, NULL);
			if (!FAILED(hr))
			{
				this->_supportsA8Surface = true;
				surface->Release();
			}
		}
		// device config
		this->_updateIntermediateRenderTexture();
		this->d3dDevice->GetRenderTarget(0, &this->backBuffer);
		this->d3dDevice->SetRenderTarget(0, ((DirectX9_Texture*)this->_intermediateRenderTexture)->_getSurface());
		this->_deviceClear(false);
		this->d3dDevice->BeginScene();
		this->renderTarget = NULL;
		this->_devicePresentFrame(true);
	}

	void DirectX9_RenderSystem::_deviceReset()
	{
		RenderSystem::_deviceReset();
		this->d3dDevice->EndScene();
		this->_deviceUnloadTextures();
		this->backBuffer->Release();
		this->backBuffer = NULL;
		HRESULT hr;
		while (april::application->getState() == Application::State::Running)
		{
			hlog::write(logTag, "Resetting device...");
			if (this->d3dpp->BackBufferWidth <= 0 || this->d3dpp->BackBufferHeight <= 0)
			{
				throw Exception(hsprintf("Backbuffer size is invalid: %d x %d", this->d3dpp->BackBufferWidth, this->d3dpp->BackBufferHeight));
			}
			hr = this->d3dDevice->Reset(this->d3dpp);
			if (!FAILED(hr))
			{
				break;
			}
			if (hr == D3DERR_DRIVERINTERNALERROR)
			{
				throw Exception("Unable to reset Direct3D device, Driver Internal Error!");
			}
			else if (hr == D3DERR_DEVICEREMOVED)
			{
				throw Exception("Unable to reset Direct3D device, 'Device removed' error reported!");
			}
			else if (hr == D3DERR_OUTOFVIDEOMEMORY)
			{
				throw Exception("Unable to reset Direct3D device, Out of Video Memory!");
			}
			else if (hr == D3DERR_INVALIDCALL)
			{
				throw Exception("Unable to reset Direct3D device, device reports 'invalid call'!");
			}
			else
			{
				hlog::errorf(logTag, "Failed to reset device!, context: DirectX9_RenderSystem::reset() hresult: %u", hr);
			}
		}
		this->d3dDevice->GetRenderTarget(0, &this->backBuffer); // update backbuffer pointer
		this->d3dDevice->BeginScene();
		hlog::write(logTag, "Direct3D9 Device restored.");
	}

	void DirectX9_RenderSystem::_deviceSetupCaps()
	{
		D3DCAPS9 d3dCaps;
		memset(&d3dCaps, 0, sizeof(D3DCAPS9));
		HRESULT hr = this->d3dDevice->GetDeviceCaps(&d3dCaps);
		if (FAILED(hr))
		{
			return;
		}
		this->caps.maxTextureSize = d3dCaps.MaxTextureWidth;
		bool pow2 = ((d3dCaps.TextureCaps & D3DPTEXTURECAPS_POW2) != 0);
		bool nonPow2conditional = ((d3dCaps.TextureCaps & D3DPTEXTURECAPS_NONPOW2CONDITIONAL) != 0);
		this->caps.npotTextures = (!pow2 && !nonPow2conditional); // this is how the docs say it is defined
		this->caps.npotTexturesLimited = (this->caps.npotTextures || pow2 && nonPow2conditional);
		if (!this->_supportsA8Surface)
		{
			this->caps.textureFormats /= Image::Format::Alpha;
			this->caps.textureFormats /= Image::Format::Greyscale;
		}
	}

	void DirectX9_RenderSystem::_deviceSetup()
	{
		// calls on init and device reset
		this->d3dDevice->SetRenderState(D3DRS_LIGHTING, 0);
		this->d3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
		this->d3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, 1);
		this->d3dDevice->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, 1);
		Caps caps = this->getCaps();
		if (!caps.npotTexturesLimited && !caps.npotTextures)
		{
			this->d3dDevice->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
		}
	}

	void DirectX9_RenderSystem::_tryAssignChildWindow()
	{
		this->d3dpp->hDeviceWindow = this->childHWnd;
		ShowWindow(this->childHWnd, SW_SHOW);
		UpdateWindow(this->childHWnd);
	}

	void DirectX9_RenderSystem::_tryUnassignChildWindow()
	{
		HWND hWnd = (HWND)april::window->getBackendId();
		this->d3dpp->hDeviceWindow = hWnd;
		ShowWindow(hWnd, SW_SHOW);
		UpdateWindow(hWnd);
	}

	void DirectX9_RenderSystem::_deviceSetupDisplayModes()
	{
		IDirect3D9* d3d = this->d3d;
		if (this->d3d == NULL)
		{
			d3d = Direct3DCreate9(D3D_SDK_VERSION);
			if (d3d == NULL)
			{
				hlog::error(logTag, "Unable to create Direct3D9 object!");
				return;
			}
		}
		unsigned int modeCount = d3d->GetAdapterModeCount(D3DADAPTER_DEFAULT, D3DFMT_X8R8G8B8);
		HRESULT hr;
		D3DDISPLAYMODE displayMode;
		for_itert (unsigned int, i, 0, modeCount)
		{
			memset(&displayMode, 0, sizeof(D3DDISPLAYMODE));
			hr = d3d->EnumAdapterModes(D3DADAPTER_DEFAULT, D3DFMT_X8R8G8B8, i, &displayMode);
			if (!FAILED(hr)) 
			{
				this->displayModes += RenderSystem::DisplayMode(displayMode.Width, displayMode.Height, displayMode.RefreshRate);
			}
		}
		if (this->d3d == NULL)
		{
			d3d->Release();
		}
	}

	int DirectX9_RenderSystem::getVRam() const
	{
		if (this->d3dDevice == NULL)
		{
			return 0;
		}
		return (this->d3dDevice->GetAvailableTextureMem() / (1024 * 1024));
	}

	Texture* DirectX9_RenderSystem::_deviceCreateTexture(bool fromResource)
	{
		return new DirectX9_Texture(fromResource);
	}

	PixelShader* DirectX9_RenderSystem::_deviceCreatePixelShader()
	{
		return new DirectX9_PixelShader();
	}

	VertexShader* DirectX9_RenderSystem::_deviceCreateVertexShader()
	{
		return new DirectX9_VertexShader();
	}

	void DirectX9_RenderSystem::_deviceChangeResolution(int w, int h, bool fullscreen)
	{
		if (this->backBuffer == NULL)
		{
			return;
		}
		if (w <= 0 || h <= 0)
		{
			hlog::warnf(logTag, "Cannot set resolution to: %d x %d", w, h);
			return;
		}
		bool resizable = IS_WINDOW_RESIZABLE;
		bool oldFullscreen = (this->d3dpp->Windowed == FALSE);
		this->d3dpp->Windowed = !fullscreen;
		if (fullscreen != oldFullscreen || !resizable)
		{
			if (resizable)
			{
				if (!fullscreen)
				{
					this->_tryAssignChildWindow();
				}
				else
				{
					this->_tryUnassignChildWindow();
				}
				this->setViewport(grect(0.0f, 0.0f, (float)w, (float)h));
				SystemInfo info = april::getSystemInfo();
				w = (int)info.displayResolution.x;
				h = (int)info.displayResolution.y;
			}
			this->d3dpp->BackBufferWidth = w;
			this->d3dpp->BackBufferHeight = h;
			this->reset();
		}
		else
		{
			if (resizable)
			{
				this->_tryAssignChildWindow();
			}
			this->setViewport(grect(0.0f, 0.0f, (float)w, (float)h));
		}
	}

	void DirectX9_RenderSystem::_setDeviceViewport(cgrect rect)
	{
		D3DVIEWPORT9 viewport;
		viewport.MinZ = 0.0f;
		viewport.MaxZ = 1.0f;
		viewport.X = (int)rect.x;
		viewport.Y = (int)rect.y;
		viewport.Width = (int)rect.w;
		viewport.Height = (int)rect.h;
		this->d3dDevice->SetViewport(&viewport);
	}

	void DirectX9_RenderSystem::_setDeviceModelviewMatrix(const gmat4& matrix)
	{
		this->d3dDevice->SetTransform(D3DTS_VIEW, (D3DMATRIX*)matrix.data);
	}

	void DirectX9_RenderSystem::_setDeviceProjectionMatrix(const gmat4& matrix)
	{
		this->d3dDevice->SetTransform(D3DTS_PROJECTION, (D3DMATRIX*)matrix.data);
	}

	void DirectX9_RenderSystem::_setDeviceDepthBuffer(bool enabled, bool writeEnabled)
	{
		if (this->options.depthBuffer)
		{
			this->d3dDevice->SetRenderState(D3DRS_ZENABLE, enabled);
			this->d3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, (enabled && writeEnabled));
			this->d3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, enabled);
			this->d3dDevice->SetRenderState(D3DRS_ALPHAREF, 0);
			this->d3dDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
		}
	}

	void DirectX9_RenderSystem::_setDeviceRenderMode(bool useTexture, bool useColor)
	{
		DWORD mode = D3DFVF_XYZ;
		if (useTexture)
		{
			mode |= D3DFVF_TEX1;
		}
		if (useColor)
		{
			mode |= D3DFVF_DIFFUSE;
		}
		this->d3dDevice->SetFVF(mode);
	}

	void DirectX9_RenderSystem::_setDeviceTexture(Texture* texture)
	{
		if (texture != NULL)
		{
			DirectX9_Texture* currentTexture = (DirectX9_Texture*)texture;
			this->d3dDevice->SetTexture(0, currentTexture->d3dTexture);
			Caps caps = this->getCaps();
			if (!caps.npotTexturesLimited && !caps.npotTextures)
			{
				if (currentTexture->effectiveWidth != 1.0f || currentTexture->effectiveHeight != 1.0f)
				{
					static gmat4 matrix;
					matrix.setScale(currentTexture->effectiveWidth, currentTexture->effectiveHeight, 1.0f);
					this->d3dDevice->SetTransform(D3DTS_TEXTURE0, (D3DMATRIX*)matrix.data);
				}
				else
				{
					static gmat4 matrix;
					this->d3dDevice->SetTransform(D3DTS_TEXTURE0, (D3DMATRIX*)matrix.data);
				}
			}
		}
		else
		{
			this->d3dDevice->SetTexture(0, NULL);
		}
	}

	void DirectX9_RenderSystem::_setDeviceTextureFilter(const Texture::Filter& textureFilter)
	{
		if (textureFilter == Texture::Filter::Linear)
		{
			this->d3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
			this->d3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		}
		else if (textureFilter == Texture::Filter::Nearest)
		{
			this->d3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
			this->d3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
		}
		else
		{
			hlog::warn(logTag, "Trying to set unsupported texture filter!");
		}
	}

	void DirectX9_RenderSystem::_setDeviceTextureAddressMode(const Texture::AddressMode& textureAddressMode)
	{
		if (textureAddressMode == Texture::AddressMode::Wrap)
		{
			this->d3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
			this->d3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
		}
		else if (textureAddressMode == Texture::AddressMode::Clamp)
		{
			this->d3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
			this->d3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
		}
		else
		{
			hlog::warn(logTag, "Trying to set unsupported texture address mode!");
		}
	}

	void DirectX9_RenderSystem::_setDeviceBlendMode(const BlendMode& blendMode)
	{
		if (blendMode == BlendMode::Alpha)
		{
			this->d3dDevice->SetRenderState(D3DRS_BLENDOPALPHA, D3DBLENDOP_ADD);
			this->d3dDevice->SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_ONE);
			this->d3dDevice->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_INVSRCALPHA);
			this->d3dDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
			this->d3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			this->d3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		}
		else if (blendMode == BlendMode::Add)
		{
			this->d3dDevice->SetRenderState(D3DRS_BLENDOPALPHA, D3DBLENDOP_ADD);
			this->d3dDevice->SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_ONE);
			this->d3dDevice->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_INVSRCALPHA);
			this->d3dDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
			this->d3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			this->d3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
		}
		else if (blendMode == BlendMode::Subtract)
		{
			this->d3dDevice->SetRenderState(D3DRS_BLENDOPALPHA, D3DBLENDOP_ADD);
			this->d3dDevice->SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_ONE);
			this->d3dDevice->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_INVSRCALPHA);
			this->d3dDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_REVSUBTRACT);
			this->d3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			this->d3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
		}
		else if (blendMode == BlendMode::Overwrite)
		{
			this->d3dDevice->SetRenderState(D3DRS_BLENDOPALPHA, D3DBLENDOP_ADD);
			this->d3dDevice->SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_ONE);
			this->d3dDevice->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_ZERO);
			this->d3dDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
			this->d3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			this->d3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
		}
		else
		{
			hlog::warn(logTag, "Trying to set unsupported blend mode!");
		}
	}

	void DirectX9_RenderSystem::_setDeviceColorMode(const ColorMode& colorMode, float factor, bool useTexture, bool useColor, const Color& systemColor)
	{
		// D3DRS_TEXTUREFACTOR is used for the system color due to some drivers not being able to use D3DTSS_CONSTANT properly
		static unsigned char colorFactor = 0;
		colorFactor = (unsigned char)(factor * 255);
		if (colorMode == ColorMode::Multiply)
		{
			if (useTexture)
			{
				this->d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
				this->d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
				this->d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
				this->d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			}
			else
			{
				this->d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);
				this->d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG2);
			}
			if (useColor)
			{
				this->d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
				this->d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
			}
			else
			{
				this->d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);
				this->d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TFACTOR);
				this->d3dDevice->SetRenderState(D3DRS_TEXTUREFACTOR, D3DCOLOR_ARGB(systemColor.a, systemColor.r, systemColor.g, systemColor.b));
			}
		}
		else if (colorMode == ColorMode::AlphaMap)
		{
			this->d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG2);
			if (useTexture)
			{
				this->d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
				this->d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
			}
			else
			{
				this->d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);
			}
			if (useColor)
			{
				this->d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
				this->d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
			}
			else
			{
				this->d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);
				this->d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TFACTOR);
				this->d3dDevice->SetRenderState(D3DRS_TEXTUREFACTOR, D3DCOLOR_ARGB(systemColor.a, systemColor.r, systemColor.g, systemColor.b));
			}
		}
		else if (colorMode == ColorMode::Lerp)
		{
			if (useTexture)
			{
				this->d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
				this->d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_LERP);
				this->d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
				this->d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TEXTURE);
				this->d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG0, D3DTA_CONSTANT);
				this->d3dDevice->SetTextureStageState(0, D3DTSS_CONSTANT, D3DCOLOR_ARGB(255, colorFactor, colorFactor, colorFactor));
			}
			else
			{
				this->d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);
				this->d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
			}
			if (useColor)
			{
				this->d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
				this->d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
			}
			else
			{
				this->d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);
				this->d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TFACTOR);
				this->d3dDevice->SetRenderState(D3DRS_TEXTUREFACTOR, D3DCOLOR_ARGB(systemColor.a, systemColor.r, systemColor.g, systemColor.b));
			}
		}
		else
		{
			hlog::warn(logTag, "Trying to set unsupported color mode!");
		}
	}

	void DirectX9_RenderSystem::_deviceClear(bool depth)
	{
		DWORD flags = D3DCLEAR_TARGET;
		if (depth)
		{
			flags |= D3DCLEAR_ZBUFFER;
		}
		this->d3dDevice->Clear(0, NULL, flags, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);
	}

	void DirectX9_RenderSystem::_deviceClear(const Color& color, bool depth)
	{
		DWORD flags = D3DCLEAR_TARGET;
		if (depth)
		{
			flags |= D3DCLEAR_ZBUFFER;
		}
		this->d3dDevice->Clear(0, NULL, flags, this->getNativeColorUInt(color), 1.0f, 0);
	}

	void DirectX9_RenderSystem::_deviceClearDepth()
	{
		this->d3dDevice->Clear(0, NULL, D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);
	}

	void DirectX9_RenderSystem::_deviceRender(const RenderOperation& renderOperation, const PlainVertex* vertices, int count)
	{
		this->d3dDevice->DrawPrimitiveUP(_dx9RenderOperations[renderOperation.value], this->_numPrimitives(renderOperation, count), vertices, sizeof(PlainVertex));
	}

	void DirectX9_RenderSystem::_deviceRender(const RenderOperation& renderOperation, const TexturedVertex* vertices, int count)
	{
		this->d3dDevice->DrawPrimitiveUP(_dx9RenderOperations[renderOperation.value], this->_numPrimitives(renderOperation, count), vertices, sizeof(TexturedVertex));
	}

	void DirectX9_RenderSystem::_deviceRender(const RenderOperation& renderOperation, const ColoredVertex* vertices, int count)
	{
		this->d3dDevice->DrawPrimitiveUP(_dx9RenderOperations[renderOperation.value], this->_numPrimitives(renderOperation, count), vertices, sizeof(ColoredVertex));
	}

	void DirectX9_RenderSystem::_deviceRender(const RenderOperation& renderOperation, const ColoredTexturedVertex* vertices, int count)
	{
		this->d3dDevice->DrawPrimitiveUP(_dx9RenderOperations[renderOperation.value], this->_numPrimitives(renderOperation, count), vertices, sizeof(ColoredTexturedVertex));
	}

	Image::Format DirectX9_RenderSystem::getNativeTextureFormat(Image::Format format) const
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
		if (format == Image::Format::Alpha || format == Image::Format::Greyscale || format == Image::Format::Compressed || format == Image::Format::Palette)
		{
			return format;
		}
		return Image::Format::Invalid;
	}

	unsigned int DirectX9_RenderSystem::getNativeColorUInt(const april::Color& color) const
	{
		return D3DCOLOR_ARGB(color.a, color.r, color.g, color.b);
	}

	Image* DirectX9_RenderSystem::takeScreenshot(Image::Format format)
	{
#ifdef _DEBUG
		hlog::write(logTag, "Taking screenshot...");
#endif
		D3DSURFACE_DESC desc;
		this->backBuffer->GetDesc(&desc);
		if (desc.Format != D3DFMT_X8R8G8B8)
		{
			hlog::error(logTag, "Failed to grab screenshot, backbuffer format not supported, expected X8R8G8B8, got: " + hstr(desc.Format));
			return NULL;
		}
		IDirect3DSurface9* buffer;
		HRESULT hr = this->d3dDevice->CreateOffscreenPlainSurface(desc.Width, desc.Height, desc.Format, D3DPOOL_SYSTEMMEM, &buffer, NULL);
		if (FAILED(hr))
		{
			hlog::error(logTag, "Failed to grab screenshot, CreateOffscreenPlainSurface() call failed.");
			return NULL;
		}
		hr = this->d3dDevice->GetRenderTargetData(this->backBuffer, buffer);
		if (FAILED(hr))
		{
			hlog::error(logTag, "Failed to grab screenshot, GetRenderTargetData() call failed.");
			buffer->Release();
			return NULL;
		}		
		D3DLOCKED_RECT rect;
		hr = buffer->LockRect(&rect, NULL, D3DLOCK_DONOTWAIT);
		if (FAILED(hr))
		{
			hlog::error(logTag, "Failed to grab screenshot, surface lock failed.");
			buffer->Release();
			return NULL;
		}
		unsigned char* data = NULL;
		Image* image = NULL;
		if (Image::convertToFormat(desc.Width, desc.Height, (unsigned char*)rect.pBits, Image::Format::BGRX, &data, format, false))
		{
			image = Image::create(desc.Width, desc.Height, data, format);
			delete[] data;
		}
		buffer->UnlockRect();
		buffer->Release();
		return image;
	}
	
	void DirectX9_RenderSystem::_devicePresentFrame(bool systemEnabled)
	{
		RenderSystem::_devicePresentFrame(systemEnabled);
		//this->d3dDevice->EndScene();
		//this->d3dDevice->SetRenderTarget(0, ((DirectX9_Texture*)this->_intermediateRenderTexture)->_getSurface());
		this->d3dDevice->SetRenderTarget(0, this->backBuffer);
		//this->d3dDevice->BeginScene();
		this->_presentIntermediateRenderTexture();
		this->d3dDevice->EndScene();
		HRESULT hr = this->d3dDevice->Present(NULL, NULL, NULL, NULL);
		if (hr == D3DERR_DEVICELOST)
		{
			hlog::write(logTag, "Direct3D9 Device lost, attempting to restore...");
			this->_deviceUnloadTextures();
			this->backBuffer->Release();
			this->backBuffer = NULL;
			while (april::application->getState() == Application::State::Running)
			{
				hr = this->d3dDevice->TestCooperativeLevel();
				if (!FAILED(hr))
				{
					break;
				}
				if (hr == D3DERR_DEVICENOTRESET)
				{
					hlog::write(logTag, "Resetting device...");
					hr = this->d3dDevice->Reset(this->d3dpp);
					if (!FAILED(hr))
					{
						break;
					}
					if (hr == D3DERR_DRIVERINTERNALERROR)
					{
						throw Exception("Unable to reset Direct3D device, Driver Internal Error!");
					}
					else if (hr == D3DERR_DEVICEREMOVED)
					{
						throw Exception("Unable to reset Direct3D device, 'Device removed' error reported!");
					}
					else if (hr == D3DERR_OUTOFVIDEOMEMORY)
					{
						throw Exception("Unable to reset Direct3D device, Out of Video Memory!");
					}
					else if (hr == D3DERR_DRIVERINTERNALERROR)
					{
						throw Exception("Unable to reset Direct3D device, Driver internal error!");
					}
					else
					{
						hlog::errorf(logTag, "Failed to reset device!, context: DirectX9_RenderSystem::_devicePresentFrame() hresult: %08X", hr);
					}
				}
				else if (hr == D3DERR_DRIVERINTERNALERROR)
				{
					throw Exception("Unable to reset Direct3D device, Driver Internal Error while testing cooperative level!");
				}
				for_iter (i, 0, 10)
				{
					april::window->checkEvents();
					hthread::sleep(100.0f);
				}
			}
			this->_deviceSetup();
			this->d3dDevice->GetRenderTarget(0, &this->backBuffer); // update backbuffer pointer
			this->d3dDevice->SetRenderTarget(0, ((DirectX9_Texture*)this->_intermediateRenderTexture)->_getSurface());
			this->d3dDevice->BeginScene();
			this->_updateDeviceState(this->state, true);
			hlog::write(logTag, "Direct3D9 Device restored.");
		}
		else
		{
			if (hr == D3DERR_WASSTILLDRAWING)
			{
				for_iter (i, 0, 100)
				{
					hr = this->d3dDevice->Present(NULL, NULL, NULL, NULL);
					if (!FAILED(hr))
					{
						break;
					}
					hthread::sleep(1.0f);
				}
			}
			this->d3dDevice->SetRenderTarget(0, ((DirectX9_Texture*)this->_intermediateRenderTexture)->_getSurface());
			this->d3dDevice->BeginScene();
		}
	}

	Texture* DirectX9_RenderSystem::getRenderTarget()
	{
		return this->renderTarget;
	}

	void DirectX9_RenderSystem::setRenderTarget(Texture* source)
	{
		if (this->renderTarget != NULL)
		{
			this->d3dDevice->EndScene();
		}
		DirectX9_Texture* texture = (DirectX9_Texture*)source;
		if (texture == NULL)
		{
			this->d3dDevice->SetRenderTarget(0, ((DirectX9_Texture*)this->_intermediateRenderTexture)->_getSurface());
		}
		else
		{
			this->d3dDevice->SetRenderTarget(0, texture->_getSurface());
		}
		this->renderTarget = texture;
		if (this->renderTarget != NULL)
		{
			this->d3dDevice->BeginScene();
		}
	}

	void DirectX9_RenderSystem::setPixelShader(PixelShader* pixelShader)
	{
		DirectX9_PixelShader* shader = (DirectX9_PixelShader*)pixelShader;
		if (shader != NULL)
		{
			this->d3dDevice->SetPixelShader(shader->dx9Shader);
		}
		else
		{
			this->d3dDevice->SetPixelShader(NULL);
		}
	}

	void DirectX9_RenderSystem::setVertexShader(VertexShader* vertexShader)
	{
		DirectX9_VertexShader* shader = (DirectX9_VertexShader*)vertexShader;
		if (shader != NULL)
		{
			this->d3dDevice->SetVertexShader(shader->dx9Shader);
		}
		else
		{
			this->d3dDevice->SetVertexShader(NULL);
		}
	}

}
#endif
