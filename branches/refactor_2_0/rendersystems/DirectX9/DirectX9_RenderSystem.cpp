/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @version 2.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _DIRECTX9

#include <d3d9.h>
#include <d3dx9math.h>
#include <stdio.h>

#include <gtypes/Vector2.h>
#include <hltypes/exception.h>
#include <hltypes/hthread.h>

#include "april.h"
#include "DirectX9_PixelShader.h"
#include "DirectX9_RenderSystem.h"
#include "DirectX9_Texture.h"
#include "DirectX9_VertexShader.h"
#include "ImageSource.h"
#include "Keys.h"
#include "Timer.h"

#define PLAIN_FVF (D3DFVF_XYZ)
#define COLOR_FVF (D3DFVF_XYZ | D3DFVF_DIFFUSE)
#define TEX_FVF (D3DFVF_XYZ | D3DFVF_TEX1)
#define TEX_COLOR_FVF (D3DFVF_XYZ | D3DFVF_TEX1 | D3DFVF_DIFFUSE)
#define TEX_COLOR_TONE_FVF (D3DFVF_XYZ | D3DFVF_TEX1 | D3DFVF_DIFFUSE | D3DFVF_SPECULAR)
#define VERTICES_BUFFER_COUNT 8192
#define UINT_RGBA_TO_ARGB(c) ((((c) >> 8) & 0xFFFFFF) | (((c) & 0xFF) << 24))

namespace april
{
#ifdef _DEBUG
	char fpstitle[1024] = " [FPS:0]";
#endif

	D3DPRIMITIVETYPE dx9_render_ops[]=
	{
		D3DPT_FORCE_DWORD,
		D3DPT_TRIANGLELIST,  // ROP_TRIANGLE_LIST
		D3DPT_TRIANGLESTRIP, // ROP_TRIANGLE_STRIP
		D3DPT_TRIANGLEFAN,   // ROP_TRIANGLE_FAN
		D3DPT_LINELIST,      // ROP_LINE_LIST
		D3DPT_LINESTRIP,     // ROP_LINE_STRIP
		D3DPT_POINTLIST,     // ROP_POINT_LIST
	};

	ColoredTexturedVertex static_ctv[VERTICES_BUFFER_COUNT];
	ColoredVertex static_cv[VERTICES_BUFFER_COUNT];

	unsigned int numPrimitives(RenderOp rop, int nVertices)
	{
		switch (rop)
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
	
	DirectX9_RenderSystem::DirectX9_RenderSystem() : RenderSystem(), zBufferEnabled(false),
		textureCoordinatesEnabled(false), colorEnabled(false), d3d(NULL), d3dDevice(NULL),
		activeTexture(NULL), renderTarget(NULL), backBuffer(NULL)
	{
		this->name = APRIL_RS_DIRECTX9;
	}

	DirectX9_RenderSystem::~DirectX9_RenderSystem()
	{
		this->destroy();
	}

	bool DirectX9_RenderSystem::create(chstr options)
	{
		if (!RenderSystem::create(options))
		{
			return false;
		}
		this->zBufferEnabled = options.contains("zbuffer");
		this->textureCoordinatesEnabled = false;
		this->colorEnabled = false;
		this->renderTarget = NULL;
		this->backBuffer = NULL;
		// DIRECT3D
		this->d3d = Direct3DCreate9(D3D_SDK_VERSION);
		if (this->d3d == NULL)
		{
			this->destroy();
			throw hl_exception("unable to create Direct3D9 object!");
		}
		return true;
	}

	bool DirectX9_RenderSystem::destroy()
	{
		if (!RenderSystem::destroy())
		{
			return false;
		}
		if (this->d3dDevice != NULL)
		{
			this->d3dDevice->Release();
		}
		this->d3dDevice = NULL;
		if (this->d3d != NULL)
		{
			this->d3d->Release();
		}
		this->d3d = NULL;
		return true;
	}

	static D3DPRESENT_PARAMETERS d3dpp;

	void DirectX9_RenderSystem::assignWindow(Window* window)
	{
		HWND hWnd = (HWND)april::window->getBackendId();
		memset(&d3dpp, 0, sizeof(d3dpp));
		d3dpp.Windowed = !window->isFullscreen();
		d3dpp.BackBufferWidth = window->getWidth();
		d3dpp.BackBufferHeight = window->getHeight();
		d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
		d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
		if (this->zBufferEnabled)
		{
			d3dpp.EnableAutoDepthStencil = TRUE;
			d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
		}
		d3dpp.SwapEffect = D3DSWAPEFFECT_COPY;
		d3dpp.hDeviceWindow = hWnd;
		HRESULT result = this->d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &d3dDevice);
		if (result != D3D_OK)
		{
			result = this->d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &d3dDevice);
			if (result != D3D_OK)
			{
				throw hl_exception("Unable to create Direct3D Device!");
			}
		}
		// device config
		this->_configureDevice();
		this->clear(true, false);
		this->presentFrame();
		this->d3dDevice->GetRenderTarget(0, &this->backBuffer);
		this->renderTarget = NULL;
		this->d3dDevice->BeginScene();
		this->orthoProjection.setSize((float)window->getWidth(), (float)window->getHeight());
	}
	
	void DirectX9_RenderSystem::_configureDevice()
	{
		// calls on init and device reset
		this->d3dDevice->SetRenderState(D3DRS_LIGHTING, 0);
		this->d3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
		this->d3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, 1);
		this->d3dDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
		this->d3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		this->d3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		// separate alpha blending to use proper alpha blending
		this->d3dDevice->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, 1);
		this->d3dDevice->SetRenderState(D3DRS_BLENDOPALPHA, D3DBLENDOP_ADD);
		this->d3dDevice->SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_ONE);
		this->d3dDevice->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_INVSRCALPHA);
		// vertex color blending
		this->d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
		this->d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
		this->setTextureFilter(this->textureFilter);
	}
	
	Texture* DirectX9_RenderSystem::loadTexture(chstr filename, bool dynamic)
	{
		hstr name = this->_findTextureFile(filename);
		if (name == "")
		{
			return NULL;
		}
		if (this->forcedDynamicLoading)
		{
			dynamic = true;
		}
		if (dynamic)
		{
			april::log("creating dynamic DX9 texture '" + name + "'");
		}
		DirectX9_Texture* t = new DirectX9_Texture(name, dynamic);
		if (!dynamic && !t->load())
		{
			delete t;
			return NULL;
		}
		return t;
	}

	Texture* DirectX9_RenderSystem::createTexture(int w, int h, unsigned char* rgba)
	{
		return new DirectX9_Texture(w, h, rgba);
	}
	
	Texture* DirectX9_RenderSystem::createTexture(int w, int h, Texture::Format format, Texture::Type type, Color color)
	{
		return new DirectX9_Texture(w, h, format, type, color);
	}
	
	PixelShader* DirectX9_RenderSystem::createPixelShader()
	{
		return new DirectX9_PixelShader();
	}

	PixelShader* DirectX9_RenderSystem::createPixelShader(chstr filename)
	{
		return new DirectX9_PixelShader(filename);
	}

	VertexShader* DirectX9_RenderSystem::createVertexShader()
	{
		return new DirectX9_VertexShader();
	}

	VertexShader* DirectX9_RenderSystem::createVertexShader(chstr filename)
	{
		return new DirectX9_VertexShader(filename);
	}

	////////////////////////////////////////////////////////////////////

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

	grect DirectX9_RenderSystem::getViewport()
	{
		D3DVIEWPORT9 viewport;
		this->d3dDevice->GetViewport(&viewport);
		return grect((float)viewport.X, (float)viewport.Y, (float)viewport.Width, (float)viewport.Height);
	}

	void DirectX9_RenderSystem::setViewport(grect rect)
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

	void DirectX9_RenderSystem::setTextureFilter(Texture::Filter textureFilter)
	{
		switch (textureFilter)
		{
		case Texture::FILTER_LINEAR:
			this->d3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
			this->d3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
			break;
		case Texture::FILTER_NEAREST:
			this->d3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
			this->d3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
			break;
		default:
			april::log("trying to set unsupported texture filter!");
			break;
		}
		this->textureFilter = textureFilter;
	}

	void DirectX9_RenderSystem::setTextureAddressMode(Texture::AddressMode textureAddressMode)
	{
		switch (textureAddressMode)
		{
		case Texture::ADDRESS_WRAP:
			this->d3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
			this->d3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
			break;
		case Texture::ADDRESS_CLAMP:
			this->d3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
			this->d3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
			break;
		default:
			april::log("trying to set unsupported texture address mode!");
			break;
		}
		this->textureAddressMode = textureAddressMode;
	}

	void DirectX9_RenderSystem::setTexture(Texture* texture)
	{
		this->activeTexture = (DirectX9_Texture*)texture;
		if (this->activeTexture != NULL)
		{
			if (!this->activeTexture->isLoaded())
			{
				this->activeTexture->load();
			}
			this->activeTexture->_resetUnusedTimer();
			this->d3dDevice->SetTexture(0, this->activeTexture->_getTexture());
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
		}
		else
		{
			this->d3dDevice->SetTexture(0, NULL);
		}
	}

	void DirectX9_RenderSystem::setResolution(int w, int h)
	{
		RenderSystem::setResolution(w, h);
		this->backBuffer->Release();
		this->backBuffer = NULL;
		d3dpp.BackBufferWidth = april::window->getWidth();
		d3dpp.BackBufferHeight = april::window->getHeight();
		log(hsprintf("resetting device for %d x %d...", april::window->getWidth(), april::window->getHeight()));
		HRESULT hr = this->d3dDevice->Reset(&d3dpp);
		if (hr == D3DERR_DRIVERINTERNALERROR)
		{
			throw hl_exception("Unable to reset Direct3D device, Driver Internal Error!");
		}
		else if (hr == D3DERR_OUTOFVIDEOMEMORY)
		{
			throw hl_exception("Unable to reset Direct3D device, Out of Video Memory!");
		}
		else if (hr != D3D_OK)
		{
			log("Failed to reset device!");
		}
		this->_setModelviewMatrix(this->modelviewMatrix);
		this->_setProjectionMatrix(this->projectionMatrix);
		this->_configureDevice();
		this->d3dDevice->GetRenderTarget(0, &this->backBuffer); // update backbuffer pointer
		april::log("Direct3D9 Device restored");
		this->d3dDevice->BeginScene();
	}

	void DirectX9_RenderSystem::clear(bool useColor, bool depth)
	{
		DWORD flags = 0;
		if (useColor)
		{
			flags |= D3DCLEAR_TARGET;
		}
		if (depth && this->zBufferEnabled)
		{
			flags |= D3DCLEAR_ZBUFFER;
		}
		this->d3dDevice->Clear(0, NULL, flags, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);
	}
	
	void DirectX9_RenderSystem::clear(bool depth, grect rect, Color color)
	{
		DWORD flags = 0;
		flags |= D3DCLEAR_TARGET;
		if (depth)
		{
			flags |= D3DCLEAR_ZBUFFER;
		}
		D3DRECT area;
		area.x1 = (int)rect.x;
		area.y1 = (int)rect.y;
		area.x2 = (int)(rect.x + rect.w);
		area.y2 = (int)(rect.y + rect.h);
		this->d3dDevice->Clear(1, &area, flags, D3DCOLOR_ARGB((int)color.a, (int)color.r, (int)color.g, (int)color.b), 1.0f, 0);
	}
	
	ImageSource* DirectX9_RenderSystem::takeScreenshot(int bpp)
    {
		april::log("grabbing screenshot");
		D3DSURFACE_DESC desc;
		this->backBuffer->GetDesc(&desc);
		if (desc.Format != D3DFMT_X8R8G8B8)
		{
			april::log("failed to grab screenshot, backbuffer format not supported, expected X8R8G8B8, got: " + hstr(desc.Format));
			return NULL;
		}
		IDirect3DSurface9* buffer;
		HRESULT hr = this->d3dDevice->CreateOffscreenPlainSurface(desc.Width, desc.Height, desc.Format, D3DPOOL_SYSTEMMEM, &buffer, NULL);
		if (hr != D3D_OK)
		{
			april::log("failed to grab screenshot, CreateOffscreenPlainSurface() call failed");
			return NULL;
		}
		hr = this->d3dDevice->GetRenderTargetData(this->backBuffer, buffer);
		if (hr != D3D_OK)
		{
			april::log("failed to grab screenshot, GetRenderTargetData() call failed");
			buffer->Release();
			return NULL;
		}		
		D3DLOCKED_RECT rect;
		hr = buffer->LockRect(&rect, NULL, D3DLOCK_DONOTWAIT);
		if (hr != D3D_OK)
		{
			april::log("failed to grab screenshot, surface lock failed");
			buffer->Release();
			return NULL;
		}
		
		ImageSource* img = new ImageSource();
		img->w = desc.Width;
		img->h = desc.Height;
		img->bpp = bpp;
		img->format = (bpp == 4 ? Texture::FORMAT_RGBA : Texture::FORMAT_RGB);
		img->data = new unsigned char[img->w * img->h * 4];
		unsigned char* p = img->data;
		unsigned char* src = (unsigned char*)rect.pBits;
		int x;
		memset(p, 255, img->w * img->h * 4 * sizeof(unsigned char));
		for_iter (y, 0, img->h)
		{
			for (x = 0; x < img->w * bpp; x += bpp, p += bpp)
			{
				p[0] = src[x + 2];
				p[1] = src[x + 1];
				p[2] = src[x];
			}
			src += rect.Pitch;
		}
		buffer->UnlockRect();
		buffer->Release();
    	return img;
	}
	
	void DirectX9_RenderSystem::_setModelviewMatrix(const gmat4& matrix)
	{
		this->d3dDevice->SetTransform(D3DTS_VIEW, (D3DMATRIX*)matrix.data);
	}

	void DirectX9_RenderSystem::_setProjectionMatrix(const gmat4& matrix)
	{
		this->d3dDevice->SetTransform(D3DTS_PROJECTION, (D3DMATRIX*)matrix.data);
	}

	void DirectX9_RenderSystem::setBlendMode(BlendMode mode)
	{
		switch (mode)
		{
		case DEFAULT:
		case ALPHA_BLEND:
			this->d3dDevice->SetRenderState(D3DRS_BLENDOPALPHA, D3DBLENDOP_ADD);
			this->d3dDevice->SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_ONE);
			this->d3dDevice->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_INVSRCALPHA);
			this->d3dDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
			this->d3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			this->d3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
			break;
		case ADD:
			this->d3dDevice->SetRenderState(D3DRS_BLENDOPALPHA, D3DBLENDOP_ADD);
			this->d3dDevice->SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_ONE);
			this->d3dDevice->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_INVSRCALPHA);
			this->d3dDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
			this->d3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			this->d3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
			break;
		case SUBTRACT:
			this->d3dDevice->SetRenderState(D3DRS_BLENDOPALPHA, D3DBLENDOP_ADD);
			this->d3dDevice->SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_ONE);
			this->d3dDevice->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_INVSRCALPHA);
			this->d3dDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_REVSUBTRACT);
			this->d3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			this->d3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
			break;
		case OVERWRITE:
			this->d3dDevice->SetRenderState(D3DRS_BLENDOPALPHA, D3DBLENDOP_ADD);
			this->d3dDevice->SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_ONE);
			this->d3dDevice->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_ZERO);
			this->d3dDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
			this->d3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
			this->d3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
			break;
		}
	}

	void DirectX9_RenderSystem::setColorMode(ColorMode mode, unsigned char alpha)
	{
		switch (mode)
		{
		case NORMAL:
		case MULTIPLY:
			this->d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
			this->d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
			this->d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
			this->d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
			this->d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			this->d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
			break;
		case LERP:
			this->d3dDevice->SetRenderState(D3DRS_TEXTUREFACTOR, D3DCOLOR_ARGB(alpha, alpha, alpha, alpha));
			this->d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
			this->d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
			this->d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);
			this->d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_BLENDDIFFUSEALPHA);
			this->d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
			this->d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TEXTURE);
			break;
		}
	}

	void DirectX9_RenderSystem::render(RenderOp renderOp, PlainVertex* v, int nVertices)
	{
		if (this->activeTexture != NULL)
		{
			this->setTexture(NULL);
		}
		this->d3dDevice->SetFVF(PLAIN_FVF);
		this->d3dDevice->DrawPrimitiveUP(dx9_render_ops[renderOp], numPrimitives(renderOp, nVertices), v, sizeof(PlainVertex));
	}

	void DirectX9_RenderSystem::render(RenderOp renderOp, PlainVertex* v, int nVertices, Color color)
	{
		if (this->activeTexture != NULL)
		{
			this->setTexture(NULL);
		}
		unsigned int colorDx9 = D3DCOLOR_ARGB((int)color.a, (int)color.r, (int)color.g, (int)color.b);
		ColoredVertex* cv = (nVertices <= VERTICES_BUFFER_COUNT) ? static_cv : new ColoredVertex[nVertices];
		ColoredVertex* p = cv;
		for_iter (i, 0, nVertices)
		{
			p[i].x = v[i].x;
			p[i].y = v[i].y;
			p[i].z = v[i].z;
			p[i].color = colorDx9;
		}
		this->d3dDevice->SetFVF(COLOR_FVF);
		this->d3dDevice->DrawPrimitiveUP(dx9_render_ops[renderOp], numPrimitives(renderOp, nVertices), cv, sizeof(ColoredVertex));
		if (nVertices > VERTICES_BUFFER_COUNT)
		{
			delete [] cv;
		}
	}
	
	void DirectX9_RenderSystem::render(RenderOp renderOp, TexturedVertex* v, int nVertices)
	{
		this->d3dDevice->SetFVF(TEX_FVF);
		this->d3dDevice->DrawPrimitiveUP(dx9_render_ops[renderOp], numPrimitives(renderOp, nVertices), v, sizeof(TexturedVertex));
	}

	void DirectX9_RenderSystem::render(RenderOp renderOp, TexturedVertex* v, int nVertices, Color color)
	{
		unsigned int colorDx9 = D3DCOLOR_ARGB((int)color.a, (int)color.r, (int)color.g, (int)color.b);
		ColoredTexturedVertex* ctv = (nVertices <= VERTICES_BUFFER_COUNT) ? static_ctv : new ColoredTexturedVertex[nVertices];
		ColoredTexturedVertex* p = ctv;
		for_iter (i, 0, nVertices)
		{
			p[i].x = v[i].x;
			p[i].y = v[i].y;
			p[i].z = v[i].z;
			p[i].u = v[i].u;
			p[i].v = v[i].v;
			p[i].color = colorDx9;
		}
		this->d3dDevice->SetFVF(TEX_COLOR_FVF);
		this->d3dDevice->DrawPrimitiveUP(dx9_render_ops[renderOp], numPrimitives(renderOp, nVertices), ctv, sizeof(ColoredTexturedVertex));
		if (nVertices > VERTICES_BUFFER_COUNT)
		{
			delete [] ctv;
		}
	}

	void DirectX9_RenderSystem::render(RenderOp renderOp, ColoredVertex* v, int nVertices)
	{
		if (this->activeTexture != NULL)
		{
			this->setTexture(NULL);
		}
		ColoredVertex* cv = (nVertices <= VERTICES_BUFFER_COUNT) ? static_cv : new ColoredVertex[nVertices];
		ColoredVertex* p = cv;
		for_iter (i, 0, nVertices)
		{
			p[i].x = v[i].x;
			p[i].y = v[i].y;
			p[i].z = v[i].z;
			p[i].color = UINT_RGBA_TO_ARGB(v[i].color);
		}
		this->d3dDevice->SetFVF(COLOR_FVF);
		this->d3dDevice->DrawPrimitiveUP(dx9_render_ops[renderOp], numPrimitives(renderOp, nVertices), cv, sizeof(ColoredVertex));
		if (nVertices > VERTICES_BUFFER_COUNT)
		{
			delete [] cv;
		}
	}

	void DirectX9_RenderSystem::render(RenderOp renderOp, ColoredTexturedVertex* v, int nVertices)
	{
		ColoredTexturedVertex* ctv = (nVertices <= VERTICES_BUFFER_COUNT) ? static_ctv : new ColoredTexturedVertex[nVertices];
		ColoredTexturedVertex* p = ctv;
		for_iter (i, 0, nVertices)
		{
			p[i].x = v[i].x;
			p[i].y = v[i].y;
			p[i].z = v[i].z;
			p[i].u = v[i].u;
			p[i].v = v[i].v;
			p[i].color = UINT_RGBA_TO_ARGB(v[i].color);
		}
		this->d3dDevice->SetFVF(TEX_COLOR_FVF);
		this->d3dDevice->DrawPrimitiveUP(dx9_render_ops[renderOp], numPrimitives(renderOp, nVertices), ctv, sizeof(ColoredTexturedVertex));
		if (nVertices > VERTICES_BUFFER_COUNT)
		{
			delete [] ctv;
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
			this->d3dDevice->SetRenderTarget(0, this->backBuffer);
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
	
	void DirectX9_RenderSystem::beginFrame()
	{
		this->d3dDevice->BeginScene();
	}

	void DirectX9_RenderSystem::presentFrame()
	{
		this->d3dDevice->EndScene();
		HRESULT hr = this->d3dDevice->Present(NULL, NULL, NULL, NULL);
		if (hr == D3DERR_DEVICELOST)
		{
			int i;
			april::log("Direct3D9 Device lost, attempting to restore...");
			this->backBuffer->Release();
			this->backBuffer = NULL;
			while (april::window->isRunning())
			{
				for_iterx (i, 0, 10)
				{
					april::window->checkEvents();
					hthread::sleep(100.0f);
				}
				hr = this->d3dDevice->TestCooperativeLevel();
				if (hr == D3D_OK)
				{
					break;
				}
				if (hr == D3DERR_DEVICENOTRESET)
				{
					april::log("Resetting device...");
					hr = this->d3dDevice->Reset(&d3dpp);
					if (hr == D3D_OK)
					{
						break;
					}
					if (hr == D3DERR_DRIVERINTERNALERROR)
					{
						throw hl_exception("Unable to reset Direct3D device, Driver Internal Error!");
					}
					else if (hr == D3DERR_OUTOFVIDEOMEMORY)
					{
						throw hl_exception("Unable to reset Direct3D device, Out of Video Memory!");
					}
					else
					{
						april::log("Failed to reset device!");
					}
				}
				else if (hr == D3DERR_DRIVERINTERNALERROR)
				{
					throw hl_exception("Unable to reset Direct3D device, Driver Internal Error!");
				}
			}
			this->_setModelviewMatrix(this->modelviewMatrix);
			this->_setProjectionMatrix(this->projectionMatrix);
			this->_configureDevice();
			this->d3dDevice->GetRenderTarget(0, &this->backBuffer); // update backbuffer pointer
			april::log("Direct3D9 Device restored");
		}
		else if (hr == D3DERR_WASSTILLDRAWING)
		{
			for_iter (i, 0, 100)
			{
				hthread::sleep(1.0f);
				hr = this->d3dDevice->Present(NULL, NULL, NULL, NULL);
				if (hr == D3D_OK)
				{
					break;
				}
			}
		}
		this->d3dDevice->BeginScene();
	}

	harray<DisplayMode> DirectX9_RenderSystem::getSupportedDisplayModes()
	{
		// TODO - optimize to enumerate only once and then reuse
		harray<DisplayMode> result;
		unsigned int modeCount = this->d3d->GetAdapterModeCount(D3DADAPTER_DEFAULT, D3DFMT_X8R8G8B8);
		HRESULT hr;
		for_itert (unsigned int, i, 0, modeCount)
		{
			D3DDISPLAYMODE displayMode;
			hr = this->d3d->EnumAdapterModes(D3DADAPTER_DEFAULT, D3DFMT_X8R8G8B8, i, &displayMode);
			if (!FAILED(hr)) 
			{
				DisplayMode mode;
				mode.width = displayMode.Width;
				mode.height = displayMode.Height;
				mode.refreshRate = displayMode.RefreshRate;
				result += mode;
			}
		}
		return result;
	}

}

#endif
