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

#include "RenderSystem_DirectX9.h"

#include "ImageSource.h"
#include "Keys.h"
#include <gtypes/Vector2.h>
#include <hltypes/exception.h>
#include <d3d9.h>
#include "dx9Texture.h"
#include <stdio.h>
#include "Timer.h"
#include "Win32Window.h"

#define PLAIN_FVF D3DFVF_XYZ
#define COLORED_FVF D3DFVF_XYZ | D3DFVF_DIFFUSE
#define TEX_FVF D3DFVF_XYZ | D3DFVF_TEX1
#define TEX_COLOR_FVF D3DFVF_XYZ | D3DFVF_TEX1 | D3DFVF_DIFFUSE


namespace April
{
	IDirect3D9* d3d=0;
	IDirect3DDevice9* d3dDevice=0;
	static HWND hWnd;
	DirectX9Texture* active_texture=0;
	gtypes::Vector2 cursorpos;
	bool cursor_visible=1;
	D3DPRESENT_PARAMETERS d3dpp;
	bool window_active=1;
	
#ifdef _DEBUG
	char fpstitle[1024]=" [FPS:0]";
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
	

	unsigned int numPrimitives(RenderOp rop,int nVertices)
	{
		if (rop == TriangleList)  return nVertices/3;
		if (rop == TriangleStrip) return nVertices-2;
		if (rop == TriangleFan)   return nVertices-1;
		if (rop == LineList)      return nVertices/2;
		if (rop == LineStrip)     return nVertices-1;
		if (rop == PointList)     return nVertices;
		return 0;
	}
/**********************************************************************************************/
	DirectX9RenderSystem::DirectX9RenderSystem(Window* window) : //int w,int h,bool fullscreen,chstr title) :
		mTexCoordsEnabled(0), mColorEnabled(0), RenderSystem()
	{
		mWindow = window;
		
		logMessage("Creating DirectX9 Rendersystem");
		
		hWnd = (HWND)getWindow()->getIDFromBackend();
		
		// DIRECT3D
		d3d=Direct3DCreate9(D3D_SDK_VERSION);
		if (!d3d) throw hl_exception("Unable to create Direct3D9 object!");
		
		ZeroMemory(&d3dpp, sizeof(d3dpp));
		d3dpp.Windowed = !window->isFullscreen();
		d3dpp.BackBufferWidth   = getWindow()->getWindowWidth(); //w;
		d3dpp.BackBufferHeight  = getWindow()->getWindowHeight(); //h;
		d3dpp.BackBufferFormat  = D3DFMT_X8R8G8B8;
		d3dpp.PresentationInterval=D3DPRESENT_INTERVAL_ONE;

		d3dpp.SwapEffect = D3DSWAPEFFECT_COPY;
		d3dpp.hDeviceWindow = hWnd;
		HRESULT hr=d3d->CreateDevice(D3DADAPTER_DEFAULT,D3DDEVTYPE_HAL,hWnd,D3DCREATE_SOFTWARE_VERTEXPROCESSING,&d3dpp,&d3dDevice);
		if (hr != D3D_OK) throw hl_exception("Unable to create Direct3D Device!");
		// device config
		configureDevice();
		clear(1,0);
		presentFrame();
		d3dDevice->GetRenderTarget(0,&mBackBuffer);
		mRenderTarget=0;
		d3dDevice->BeginScene();
	}
	
	void DirectX9RenderSystem::configureDevice()
	{
		// calls on init and device reset
		d3dDevice->SetRenderState(D3DRS_LIGHTING,0);
		d3dDevice->SetRenderState(D3DRS_CULLMODE,D3DCULL_NONE);
		d3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE,1);
		d3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		d3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
		setTextureFilter(mTextureFilter);
	}
	
	DirectX9RenderSystem::~DirectX9RenderSystem()
	{
		logMessage("Destroying DirectX9 Rendersystem");
		if (d3dDevice) d3dDevice->Release();
		if (d3d) d3d->Release();
		d3d=0; d3dDevice=0;
	}

	hstr DirectX9RenderSystem::getName()
	{
		return "DirectX9";
	}
	
	float DirectX9RenderSystem::getPixelOffset()
	{
		return 0.5f;
	}

	Texture* DirectX9RenderSystem::loadTexture(chstr filename,bool dynamic)
	{
		hstr name=findTextureFile(filename);
		if (name=="") return 0;
		if (mDynamicLoading) dynamic=1;
		if (dynamic) rendersys->logMessage("creating dynamic DX9 texture '"+name+"'");
		DirectX9Texture* t=new DirectX9Texture(name,dynamic);
		if (!dynamic)
		{
			if (!t->load())
			{
				delete t;
				return 0;
			}
		}
		return t;
	}

	Texture* DirectX9RenderSystem::createTextureFromMemory(unsigned char* rgba,int w,int h)
	{
		return new DirectX9Texture(rgba,w,h);
	}
	
	Texture* DirectX9RenderSystem::createEmptyTexture(int w,int h,TextureFormat fmt,TextureType type)
	{
		return new DirectX9Texture(w,h,fmt,type);
	}
	
	void DirectX9RenderSystem::setTextureFilter(TextureFilter filter)
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
			logMessage("trying to set unsupported texture filter!");
		mTextureFilter=filter;
	}

	void DirectX9RenderSystem::setTextureWrapping(bool wrap)
	{
		if (wrap)
		{
			d3dDevice->SetSamplerState(0,D3DSAMP_ADDRESSU,D3DTADDRESS_WRAP);
			d3dDevice->SetSamplerState(0,D3DSAMP_ADDRESSV,D3DTADDRESS_WRAP);
		}
		else
		{
			d3dDevice->SetSamplerState(0,D3DSAMP_ADDRESSU,D3DTADDRESS_CLAMP);
			d3dDevice->SetSamplerState(0,D3DSAMP_ADDRESSV,D3DTADDRESS_CLAMP);
		}
		mTextureWrapping=wrap;
	}

	void DirectX9RenderSystem::setTexture(Texture* t)
	{
		active_texture=(DirectX9Texture*) t;
		if (active_texture)
		{
			if (active_texture->mTexture == 0 && active_texture->isDynamic())
				active_texture->load();
			active_texture->_resetUnusedTimer();
			d3dDevice->SetTexture(0,active_texture->mTexture);

			TextureFilter filter=t->getTextureFilter();
			if (filter != mTextureFilter)
				setTextureFilter(filter);

			bool wrapping=t->isTextureWrappingEnabled();
			if (mTextureWrapping != wrapping) setTextureWrapping(wrapping);
		}
		else
			d3dDevice->SetTexture(0,0);
	}

	void DirectX9RenderSystem::clear(bool color,bool depth)
	{
		
		DWORD flags=0;
		if (color) flags |= D3DCLEAR_TARGET;
		if (depth) flags |= D3DCLEAR_ZBUFFER;
		d3dDevice->Clear(0, NULL, flags, D3DCOLOR_XRGB(0,0,0), 1.0f, 0);
	}
	
	ImageSource* DirectX9RenderSystem::grabScreenshot()
    {
		logMessage("grabbing screenshot");
		D3DSURFACE_DESC desc;
		mBackBuffer->GetDesc(&desc);
		if (desc.Format != D3DFMT_X8R8G8B8)
		{
			logMessage("failed to grab screenshot, backbuffer format not supported, expeted X8R8G8B8, got: "+hstr(desc.Format));
			return 0;
		}
		
		IDirect3DSurface9* buffer;
		HRESULT hr = d3dDevice->CreateOffscreenPlainSurface(desc.Width,desc.Height,desc.Format,D3DPOOL_SYSTEMMEM,&buffer,NULL);
		if (hr != D3D_OK)
		{
			logMessage("failed to grab screenshot, CreateOffscreenPlainSurface() call failed");
			return 0;
		}
		hr=d3dDevice->GetRenderTargetData(mBackBuffer,buffer);
		if (hr != D3D_OK)
		{
			logMessage("failed to grab screenshot, GetRenderTargetData() call failed");
			buffer->Release();
			return 0;
		}		
		D3DLOCKED_RECT rect;
		hr = buffer->LockRect(&rect,NULL, D3DLOCK_DONOTWAIT);
		if (hr != D3D_OK)
		{
			logMessage("failed to grab screenshot, surface lock failed");
			buffer->Release();
			return 0;
		}
		
		ImageSource* img=new ImageSource();
		img->w=desc.Width; img->h=desc.Height;img->bpp=3; img->format=AT_RGB;
		img->data=(unsigned char*) malloc(img->w*img->h*4);
		unsigned char *p=img->data,*src=(unsigned char*) rect.pBits;
		int x,y;
		for (y=0;y<img->h;y++)
		{
			for (x=0;x<img->w*4;x+=4,p+=3)
			{
				p[0]=src[x+2];
				p[1]=src[x+1];
				p[2]=src[x];
			}
			src+=rect.Pitch;
		}
		
		
		buffer->UnlockRect();
		buffer->Release();
		
    	return img;
	}
	
	void DirectX9RenderSystem::_setModelviewMatrix(const gtypes::Matrix4& matrix)
	{
		d3dDevice->SetTransform(D3DTS_VIEW,(D3DMATRIX*) matrix.mat);
	}

	void DirectX9RenderSystem::_setProjectionMatrix(const gtypes::Matrix4& matrix)
	{
		d3dDevice->SetTransform(D3DTS_PROJECTION,(D3DMATRIX*) matrix.mat);
	}

	void DirectX9RenderSystem::setBlendMode(BlendMode mode)
	{
		if (mode == ALPHA_BLEND || mode == DEFAULT)
		{
			d3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			d3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		}
		else if (mode == ADD)
		{
			d3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			d3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
		}
	}
	void DirectX9RenderSystem::render(RenderOp renderOp,ColoredTexturedVertex* v,int nVertices)
	{
		d3dDevice->SetFVF(TEX_COLOR_FVF);
		d3dDevice->DrawPrimitiveUP(dx9_render_ops[renderOp],numPrimitives(renderOp,nVertices),v,sizeof(ColoredTexturedVertex));

	}

	void DirectX9RenderSystem::render(RenderOp renderOp,TexturedVertex* v,int nVertices)
	{
		if (mAlphaMultiplier == 1.0f)
		{
			d3dDevice->SetFVF(TEX_FVF);
			d3dDevice->DrawPrimitiveUP(dx9_render_ops[renderOp],numPrimitives(renderOp,nVertices),v,sizeof(TexturedVertex));
		}
		else
			render(renderOp,v,nVertices,1,1,1,1);
	}

	ColoredTexturedVertex static_ctv[100];
	void DirectX9RenderSystem::render(RenderOp renderOp,TexturedVertex* v,int nVertices,float r,float g,float b,float a)
	{
		DWORD color=D3DCOLOR_ARGB((int) (a*mAlphaMultiplier*255.0f),(int) (r*255.0f),(int) (g*255.0f),(int) (b*255.0f));
		ColoredTexturedVertex* cv=(nVertices <= 100) ? static_ctv : new ColoredTexturedVertex[nVertices];
		ColoredTexturedVertex* p=cv;
		for (int i=0;i<nVertices;i++,p++,v++)
		{ p->x=v->x; p->y=v->y; p->z=v->z; p->color=color; p->u=v->u; p->v=v->v; }
		render(renderOp,cv,nVertices);
		if (nVertices > 100) delete [] cv;
	}

	ColoredVertex static_cv[100];
	void DirectX9RenderSystem::render(RenderOp renderOp,PlainVertex* v,int nVertices)
	{
		ColoredVertex* cv=(nVertices <= 100) ? static_cv : new ColoredVertex[nVertices];
		ColoredVertex* p=cv;
		DWORD color=D3DCOLOR_ARGB((int) (mAlphaMultiplier*255),255,255,255);
		for (int i=0;i<nVertices;i++,p++,v++)
		{ p->x=v->x; p->y=v->y; p->z=v->z; p->color=color; }
		render(renderOp,cv,nVertices);
		if (nVertices > 100) delete [] cv;
	}

	void DirectX9RenderSystem::render(RenderOp renderOp,PlainVertex* v,int nVertices,float r,float g,float b,float a)
	{
		DWORD color=D3DCOLOR_ARGB((int) (a*mAlphaMultiplier*255.0f),(int) (r*255.0f),(int) (g*255.0f),(int) (b*255.0f));
		ColoredVertex* cv=(nVertices <= 100) ? static_cv : new ColoredVertex[nVertices];
		ColoredVertex* p=cv;
		for (int i=0;i<nVertices;i++,p++,v++)
		{ p->x=v->x; p->y=v->y; p->z=v->z; p->color=color; }
		render(renderOp,cv,nVertices);
		if (nVertices > 100) delete [] cv;
	}
	
	void DirectX9RenderSystem::setRenderTarget(Texture* source)
	{
		if (mRenderTarget) d3dDevice->EndScene();
		if (!source)
		{
			d3dDevice->SetRenderTarget(0,mBackBuffer);
		}
		else
			d3dDevice->SetRenderTarget(0,((DirectX9Texture*) source)->getSurface());
		if (mRenderTarget) d3dDevice->BeginScene();
		mRenderTarget=(DirectX9Texture*) source;
	}
	/*
	int DirectX9RenderSystem::getWindowWidth()
	{
		RECT rc;
		GetClientRect(hWnd, &rc);
		return rc.right-rc.left;
	}
	
	int DirectX9RenderSystem::getWindowHeight()
	{
		RECT rc;
		GetClientRect(hWnd, &rc);
		return rc.bottom-rc.top;
	}
	*/
	void DirectX9RenderSystem::render(RenderOp renderOp,ColoredVertex* v,int nVertices)
	{
		if (active_texture) setTexture(0);
		d3dDevice->SetFVF(COLORED_FVF);
		if (mAlphaMultiplier)
		{
			d3dDevice->DrawPrimitiveUP(dx9_render_ops[renderOp],numPrimitives(renderOp,nVertices),v,sizeof(ColoredVertex));
		}
		else
		{
			DWORD a=((int) (mAlphaMultiplier*255)) << 24;
			ColoredVertex* cv=(nVertices <= 100) ? static_cv : new ColoredVertex[nVertices];
			ColoredVertex* p=cv;
			for (int i=0;i<nVertices;i++,p++,v++)
			{ p->x=v->x; p->y=v->y; p->z=v->z; p->color=(v->color & 0xFFFFFF) | a; }
			d3dDevice->DrawPrimitiveUP(dx9_render_ops[renderOp],numPrimitives(renderOp,nVertices),cv,sizeof(ColoredVertex));
			if (nVertices > 100) delete [] cv;
		}
	}

	void DirectX9RenderSystem::setAlphaMultiplier(float value)
	{
		mAlphaMultiplier=value;
	}
/*
	void DirectX9RenderSystem::setWindowTitle(chstr title)
	{
		mTitle=title;
#ifdef _DEBUG
		SetWindowText(hWnd,(title+fpstitle).c_str());
#else
		SetWindowText(hWnd,title.c_str());
#endif
}
	
	gtypes::Vector2 DirectX9RenderSystem::getCursorPos()
	{
		
		return gtypes::Vector2(cursorpos.x,cursorpos.y);
	}

	void DirectX9RenderSystem::showSystemCursor(bool b)
	{
		if (b) SetCursor(LoadCursor(0,IDC_ARROW));
		else   SetCursor(0);
		cursor_visible=b;
	}
	
	bool DirectX9RenderSystem::isSystemCursorShown()
	{
		return cursor_visible;
	}
*/	
	void DirectX9RenderSystem::beginFrame()
	{
		d3dDevice->BeginScene();
	}

	void DirectX9RenderSystem::presentFrame()
	{
		d3dDevice->EndScene();
		HRESULT hr=d3dDevice->Present(NULL, NULL, NULL, NULL);
		if (hr == D3DERR_DEVICELOST)
		{
			int i;
			logMessage("Direct3D9 Device lost, attempting to restore...");
			mBackBuffer->Release(); mBackBuffer=0;
			while (((Win32Window*) mWindow)->isRunning())
			{
				for (i=0;i<10;i++)
				{
					mWindow->doEvents();
					Sleep(100);
				}
				hr=d3dDevice->TestCooperativeLevel();
				if (hr == D3D_OK) break;
				else if (hr == D3DERR_DEVICENOTRESET)
				{
					logMessage("Resetting device...");
					hr=d3dDevice->Reset(&d3dpp);
					if (hr == D3D_OK) break;
					else if (hr == D3DERR_DRIVERINTERNALERROR) throw hl_exception("Unable to reset Direct3D device, Driver Internal Error!");
					else if (hr == D3DERR_OUTOFVIDEOMEMORY)    throw hl_exception("Unable to reset Direct3D device, Out of Video Memory!");
					else
						logMessage("Failed to reset device!");
				}
				else if (hr == D3DERR_DRIVERINTERNALERROR) throw hl_exception("Unable to reset Direct3D device, Driver Internal Error!");
			}
			_setModelviewMatrix(mModelviewMatrix);
			_setProjectionMatrix(mProjectionMatrix);
			configureDevice();
			d3dDevice->GetRenderTarget(0,&mBackBuffer); // update backbuffer pointer
				logMessage("Direct3D9 Device restored");
		}
		else if (hr == D3DERR_WASSTILLDRAWING)
		{
			for (int i=0;i<100;i++)
			{
				Sleep(1);
				hr=d3dDevice->Present(NULL, NULL, NULL, NULL);
				if (hr == D3D_OK) break;
			}
		}
		d3dDevice->BeginScene();
	}

	void createDX9RenderSystem(Window* window) //int w,int h,bool fullscreen,chstr title)
	{
		April::rendersys = ::rendersys = new DirectX9RenderSystem(window); //w,h,fullscreen,title);
	}

}

#endif
