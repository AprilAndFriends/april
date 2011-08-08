/************************************************************************************\
This source file is part of the Awesome Portable Rendering Interface Library         *
For latest info, see http://libapril.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes                                                     *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#ifdef _DIRECTX9

#include <d3d9.h>
#include <stdio.h>

#include <gtypes/Vector2.h>
#include <hltypes/exception.h>

#include "DirectX9_RenderSystem.h"
#include "DirectX9_Texture.h"
#include "ImageSource.h"
#include "Keys.h"
#include "Timer.h"
#include "Win32Window.h"

#define PLAIN_FVF (D3DFVF_XYZ)
#define COLOR_FVF (D3DFVF_XYZ | D3DFVF_DIFFUSE)
#define TEX_FVF (D3DFVF_XYZ | D3DFVF_TEX1)
#define TEX_COLOR_FVF (D3DFVF_XYZ | D3DFVF_TEX1 | D3DFVF_DIFFUSE)
#define VERTICES_BUFFER_COUNT 8192
#define UINT_RGBA_TO_ARGB(c) ((((c) >> 8) & 0xFFFFFF) | (((c) & 0xFF) << 24))

namespace april
{
	IDirect3D9* d3d = NULL;
	IDirect3DDevice9* d3dDevice = NULL;
	static HWND hWnd;
	DirectX9_Texture* activeTexture = NULL;
	D3DPRESENT_PARAMETERS d3dpp;
	
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
/************************************************************************************/
	DirectX9_RenderSystem::DirectX9_RenderSystem() :
		mTexCoordsEnabled(0), mColorEnabled(0), RenderSystem()
	{
		// DIRECT3D
		d3d = Direct3DCreate9(D3D_SDK_VERSION);
		if (!d3d)
		{
			throw hl_exception("Unable to create Direct3D9 object!");
		}
	}

	void DirectX9_RenderSystem::assignWindow(Window* window)
	{
		mWindow = window;
		
		hWnd = (HWND)mWindow->getIDFromBackend();
		
		ZeroMemory(&d3dpp, sizeof(d3dpp));
		d3dpp.Windowed = !window->isFullscreen();
		d3dpp.BackBufferWidth = getWindow()->getWidth();
		d3dpp.BackBufferHeight = getWindow()->getHeight();
		d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
		d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;

		d3dpp.SwapEffect = D3DSWAPEFFECT_COPY;
		d3dpp.hDeviceWindow = hWnd;
		HRESULT hr = d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &d3dDevice);
		if (hr != D3D_OK)
		{
			throw hl_exception("Unable to create Direct3D Device!");
		}
		// device config
		configureDevice();
		clear(true, false);
		presentFrame();
		d3dDevice->GetRenderTarget(0, &mBackBuffer);
		mRenderTarget = NULL;
		d3dDevice->BeginScene();
	}
	
	void DirectX9_RenderSystem::configureDevice()
	{
		// calls on init and device reset
		d3dDevice->SetRenderState(D3DRS_LIGHTING, 0);
		d3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
		d3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, 1);
		d3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		d3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
		setTextureFilter(mTextureFilter);
	}
	
	DirectX9_RenderSystem::~DirectX9_RenderSystem()
	{
		april::log("Destroying DirectX9 Rendersystem");
		if (d3dDevice != NULL)
		{
			d3dDevice->Release();
		}
		d3dDevice = NULL;
		if (d3d != NULL)
		{
			d3d->Release();
		}
		d3d = NULL;
	}

	hstr DirectX9_RenderSystem::getName()
	{
		return "DirectX9";
	}
	
	float DirectX9_RenderSystem::getPixelOffset()
	{
		return 0.5f;
	}

	Texture* DirectX9_RenderSystem::loadTexture(chstr filename, bool dynamic)
	{
		hstr name = findTextureFile(filename);
		if (name == "")
		{
			return NULL;
		}
		if (mDynamicLoading)
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

	Texture* DirectX9_RenderSystem::createTextureFromMemory(unsigned char* rgba, int w, int h)
	{
		return new DirectX9_Texture(rgba, w, h);
	}
	
	Texture* DirectX9_RenderSystem::createEmptyTexture(int w, int h, TextureFormat fmt, TextureType type)
	{
		return new DirectX9_Texture(w, h, fmt, type);
	}
	
	void DirectX9_RenderSystem::setTextureFilter(TextureFilter filter)
	{
		if (filter == Linear)
		{
			d3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
			d3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		}
		else if (filter == Nearest)
		{
			d3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
			d3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
		}
		else
		{
			april::log("trying to set unsupported texture filter!");
		}
		mTextureFilter = filter;
	}

	void DirectX9_RenderSystem::setTextureWrapping(bool wrap)
	{
		if (wrap)
		{
			d3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
			d3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
		}
		else
		{
			d3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
			d3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
		}
		mTextureWrapping = wrap;
	}

	void DirectX9_RenderSystem::setTexture(Texture* t)
	{
		activeTexture = (DirectX9_Texture*)t;
		if (activeTexture)
		{
			if (activeTexture->mTexture == NULL && activeTexture->isDynamic())
			{
				activeTexture->load();
			}
			activeTexture->_resetUnusedTimer();
			d3dDevice->SetTexture(0, activeTexture->mTexture);
			TextureFilter filter = t->getTextureFilter();
			if (filter != mTextureFilter)
			{
				setTextureFilter(filter);
			}
			bool wrapping = t->isTextureWrappingEnabled();
			if (mTextureWrapping != wrapping)
			{
				setTextureWrapping(wrapping);
			}
		}
		else
		{
			d3dDevice->SetTexture(0, 0);
		}
	}

	void DirectX9_RenderSystem::setResolution(int w, int h)
	{
		RenderSystem::setResolution(w, h);
		mBackBuffer->Release();
		mBackBuffer = NULL;
		d3dpp.BackBufferWidth = mWindow->getWidth();
		d3dpp.BackBufferHeight = mWindow->getHeight();
		log(hsprintf("Resetting device for %d x %d...", mWindow->getWidth(), mWindow->getHeight()));
		HRESULT hr=d3dDevice->Reset(&d3dpp);
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
		_setModelviewMatrix(mModelviewMatrix);
		_setProjectionMatrix(mProjectionMatrix);
		configureDevice();
		d3dDevice->GetRenderTarget(0, &mBackBuffer); // update backbuffer pointer
		log("Direct3D9 Device restored");
		d3dDevice->BeginScene();
	}

	void DirectX9_RenderSystem::clear(bool color, bool depth)
	{
		DWORD flags = 0;
		if (color)
		{
			flags |= D3DCLEAR_TARGET;
		}
		if (depth)
		{
			flags |= D3DCLEAR_ZBUFFER;
		}
		d3dDevice->Clear(0, NULL, flags, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);
	}
	
	ImageSource* DirectX9_RenderSystem::grabScreenshot(int bpp)
    {
		april::log("grabbing screenshot");
		D3DSURFACE_DESC desc;
		mBackBuffer->GetDesc(&desc);
		if (desc.Format != D3DFMT_X8R8G8B8)
		{
			april::log("failed to grab screenshot, backbuffer format not supported, expeted X8R8G8B8, got: " + hstr(desc.Format));
			return NULL;
		}
		IDirect3DSurface9* buffer;
		HRESULT hr = d3dDevice->CreateOffscreenPlainSurface(desc.Width, desc.Height, desc.Format, D3DPOOL_SYSTEMMEM, &buffer, NULL);
		if (hr != D3D_OK)
		{
			april::log("failed to grab screenshot, CreateOffscreenPlainSurface() call failed");
			return NULL;
		}
		hr = d3dDevice->GetRenderTargetData(mBackBuffer, buffer);
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
		img->format = (bpp == 4 ? AT_RGBA : AT_RGB);
		img->data = (unsigned char*)malloc(img->w * img->h * 4);
		unsigned char* p = img->data;
		unsigned char* src = (unsigned char*)rect.pBits;
		int x;
		for (int y = 0; y < img->h; y++)
		{
			for (x = 0; x < img->w * 4; x += 4, p += bpp)
			{
				p[0] = src[x + 2];
				p[1] = src[x + 1];
				p[2] = src[x];
				if (bpp == 4)
				{
					p[3] = 255;
				}
			}
			src += rect.Pitch;
		}
		buffer->UnlockRect();
		buffer->Release();
    	return img;
	}
	
	void DirectX9_RenderSystem::_setModelviewMatrix(const gmat4& matrix)
	{
		d3dDevice->SetTransform(D3DTS_VIEW, (D3DMATRIX*)matrix.data);
	}

	void DirectX9_RenderSystem::_setProjectionMatrix(const gmat4& matrix)
	{
		d3dDevice->SetTransform(D3DTS_PROJECTION, (D3DMATRIX*)matrix.data);
	}

	void DirectX9_RenderSystem::setBlendMode(BlendMode mode)
	{
		d3dDevice->SetRenderState(D3DRS_ALPHAREF, 224);
		d3dDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_LESS);
		switch (mode)
		{
		case DEFAULT:
		case ALPHA_BLEND:
			d3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			d3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
			break;
		case ADD:
			d3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			d3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
			break;
		case SUBTRACT:
			d3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			d3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR);
			break;
		/*
		case ADD:
			d3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			d3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
			break;
		*/
		}
	}

	void DirectX9_RenderSystem::render(RenderOp renderOp, PlainVertex* v, int nVertices)
	{
		if (activeTexture != NULL)
		{
			setTexture(NULL);
		}
		d3dDevice->SetFVF(PLAIN_FVF);
		d3dDevice->DrawPrimitiveUP(dx9_render_ops[renderOp], numPrimitives(renderOp, nVertices), v, sizeof(PlainVertex));
	}

	void DirectX9_RenderSystem::render(RenderOp renderOp, PlainVertex* v, int nVertices, Color color)
	{
		if (activeTexture != NULL)
		{
			setTexture(NULL);
		}
		unsigned int colorDx9 = D3DCOLOR_ARGB((int)color.a, (int)color.r, (int)color.g, (int)color.b);
		ColoredVertex* cv = (nVertices <= VERTICES_BUFFER_COUNT) ? static_cv : new ColoredVertex[nVertices];
		ColoredVertex* p = cv;
		for (int i = 0; i < nVertices; i++, p++, v++)
		{
			p->x = v->x;
			p->y = v->y;
			p->z = v->z;
			p->color = colorDx9;
		}
		d3dDevice->SetFVF(COLOR_FVF);
		d3dDevice->DrawPrimitiveUP(dx9_render_ops[renderOp], numPrimitives(renderOp, nVertices), cv, sizeof(ColoredVertex));
		if (nVertices > VERTICES_BUFFER_COUNT)
		{
			delete [] cv;
		}
	}
	
	void DirectX9_RenderSystem::render(RenderOp renderOp, TexturedVertex* v, int nVertices)
	{
		d3dDevice->SetFVF(TEX_FVF);
		d3dDevice->DrawPrimitiveUP(dx9_render_ops[renderOp], numPrimitives(renderOp, nVertices), v, sizeof(TexturedVertex));
	}

	void DirectX9_RenderSystem::render(RenderOp renderOp, TexturedVertex* v, int nVertices, Color color)
	{
		unsigned int colorDx9 = D3DCOLOR_ARGB((int)color.a, (int)color.r, (int)color.g, (int)color.b);
		ColoredTexturedVertex* ctv = (nVertices <= VERTICES_BUFFER_COUNT) ? static_ctv : new ColoredTexturedVertex[nVertices];
		ColoredTexturedVertex* p = ctv;
		for (int i = 0; i < nVertices; i++, p++, v++)
		{
			p->x = v->x;
			p->y = v->y;
			p->z = v->z;
			p->u = v->u;
			p->v = v->v;
			p->color = colorDx9;
		}
		d3dDevice->SetFVF(TEX_COLOR_FVF);
		d3dDevice->DrawPrimitiveUP(dx9_render_ops[renderOp], numPrimitives(renderOp, nVertices), ctv, sizeof(ColoredTexturedVertex));
		if (nVertices > VERTICES_BUFFER_COUNT)
		{
			delete [] ctv;
		}
	}

	void DirectX9_RenderSystem::render(RenderOp renderOp, ColoredVertex* v, int nVertices)
	{
		if (activeTexture != NULL)
		{
			setTexture(NULL);
		}
		ColoredVertex* cv = (nVertices <= VERTICES_BUFFER_COUNT) ? static_cv : new ColoredVertex[nVertices];
		ColoredVertex* p = cv;
		for (int i = 0; i < nVertices; i++, p++, v++)
		{
			p->x = v->x;
			p->y = v->y;
			p->z = v->z;
			p->color = UINT_RGBA_TO_ARGB(v->color);
		}
		d3dDevice->SetFVF(COLOR_FVF);
		d3dDevice->DrawPrimitiveUP(dx9_render_ops[renderOp], numPrimitives(renderOp, nVertices), cv, sizeof(ColoredVertex));
		if (nVertices > VERTICES_BUFFER_COUNT)
		{
			delete [] cv;
		}
	}

	void DirectX9_RenderSystem::render(RenderOp renderOp, ColoredTexturedVertex* v, int nVertices)
	{
		ColoredTexturedVertex* ctv = (nVertices <= VERTICES_BUFFER_COUNT) ? static_ctv : new ColoredTexturedVertex[nVertices];
		ColoredTexturedVertex* p = ctv;
		for (int i = 0; i < nVertices; i++, p++, v++)
		{
			p->x = v->x;
			p->y = v->y;
			p->z = v->z;
			p->u = v->u;
			p->v = v->v;
			p->color = UINT_RGBA_TO_ARGB(v->color);
		}
		d3dDevice->SetFVF(TEX_COLOR_FVF);
		d3dDevice->DrawPrimitiveUP(dx9_render_ops[renderOp], numPrimitives(renderOp, nVertices), ctv, sizeof(ColoredTexturedVertex));
		if (nVertices > VERTICES_BUFFER_COUNT)
		{
			delete [] ctv;
		}
	}

	void DirectX9_RenderSystem::setRenderTarget(Texture* source)
	{
		if (mRenderTarget != NULL)
		{
			d3dDevice->EndScene();
		}
		DirectX9_Texture* texture = dynamic_cast<DirectX9_Texture*>(source);
		if (texture == NULL)
		{
			d3dDevice->SetRenderTarget(0, mBackBuffer);
		}
		else
		{
			d3dDevice->SetRenderTarget(0, texture->getSurface());
		}
		mRenderTarget = texture;
		if (mRenderTarget != NULL)
		{
			d3dDevice->BeginScene();
		}
	}
	
	void DirectX9_RenderSystem::beginFrame()
	{
		d3dDevice->BeginScene();
	}

	void DirectX9_RenderSystem::presentFrame()
	{
		d3dDevice->EndScene();
		HRESULT hr = d3dDevice->Present(NULL, NULL, NULL, NULL);
		if (hr == D3DERR_DEVICELOST)
		{
			int i;
			april::log("Direct3D9 Device lost, attempting to restore...");
			mBackBuffer->Release();
			mBackBuffer = 0;
			while (((Win32Window*)mWindow)->isRunning())
			{
				for (i = 0; i < 10; i++)
				{
					mWindow->doEvents();
					Sleep(100);
				}
				hr = d3dDevice->TestCooperativeLevel();
				if (hr == D3D_OK)
				{
					break;
				}
				if (hr == D3DERR_DEVICENOTRESET)
				{
					april::log("Resetting device...");
					hr = d3dDevice->Reset(&d3dpp);
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
			_setModelviewMatrix(mModelviewMatrix);
			_setProjectionMatrix(mProjectionMatrix);
			configureDevice();
			d3dDevice->GetRenderTarget(0, &mBackBuffer); // update backbuffer pointer
			april::log("Direct3D9 Device restored");
		}
		else if (hr == D3DERR_WASSTILLDRAWING)
		{
			for (int i = 0; i < 100; i++)
			{
				Sleep(1);
				hr = d3dDevice->Present(NULL, NULL, NULL, NULL);
				if (hr == D3D_OK)
				{
					break;
				}
			}
		}
		d3dDevice->BeginScene();
	}

	harray<DisplayMode> DirectX9_RenderSystem::getSupportedDisplayModes()
	{
		// TODO - optimize to enumerate only once and then reuse
		harray<DisplayMode> result;
		unsigned int modecount = d3d->GetAdapterModeCount(D3DADAPTER_DEFAULT, D3DFMT_X8R8G8B8);
		HRESULT hr;
		for (unsigned int i = 0; i < modecount; i++)
		{
			D3DDISPLAYMODE displayMode;
			hr = d3d->EnumAdapterModes(D3DADAPTER_DEFAULT, D3DFMT_X8R8G8B8, i, &displayMode);
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

	DirectX9_RenderSystem* DirectX9_RenderSystem::create()
	{
		return new DirectX9_RenderSystem();
	}

}

#endif
