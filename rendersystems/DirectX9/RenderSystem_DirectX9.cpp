/************************************************************************************\
This source file is part of the Awesome Portable Rendering Interface Library         *
For latest info, see http://libapril.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes (kreso@cateia.com)                                  *
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

#define PLAIN_FVF D3DFVF_XYZ
#define COLORED_FVF D3DFVF_XYZ | D3DFVF_DIFFUSE
#define TEX_FVF D3DFVF_XYZ | D3DFVF_TEX1
#define TEX_COLOR_FVF D3DFVF_XYZ | D3DFVF_TEX1 | D3DFVF_DIFFUSE

April::Timer globalTimer;

namespace April
{
	IDirect3D9* d3d=0;
	IDirect3DDevice9* d3dDevice=0;
	HWND hWnd;
	DirectX9Texture* active_texture=0;
	gtypes::Vector2 cursorpos;
	bool cursor_visible=1;
	D3DPRESENT_PARAMETERS d3dpp;
	bool window_active=1,wnd_fullscreen=0;
	
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
	};
	
	void doWindowEvents()
	{
		MSG msg;
		if (PeekMessage(&msg,hWnd,0,0,PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	unsigned int numPrimitives(RenderOp rop,int nVertices)
	{
		if (rop == TriangleList)  return nVertices/3;
		if (rop == TriangleStrip) return nVertices-2;
		if (rop == TriangleFan)   return nVertices-1;
		if (rop == LineList)      return nVertices/2;
		if (rop == LineStrip)     return nVertices-1;
		return 0;
	}
/**********************************************************************************************/
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	DirectX9RenderSystem *dx9rs=(DirectX9RenderSystem*) rendersys;
    switch(message)
    {
        case WM_DESTROY:
		case WM_CLOSE:
			if(dx9rs->triggerQuitEvent())
			{
				PostQuitMessage(0);
				rendersys->terminateMainLoop();
			}
			return 0;
			break;
		case WM_KEYDOWN:
#ifdef _DEBUG //2DO - should be removed completely
		    if (wParam == VK_ESCAPE) { rendersys->terminateMainLoop(); return 0; }
#endif
			dx9rs->triggerKeyEvent(1,wParam);
			break;
		case WM_KEYUP: 
			dx9rs->triggerKeyEvent(0,wParam);
			break;
		case WM_CHAR:
			dx9rs->triggerCharEvent(wParam);
			break;
		case WM_LBUTTONDOWN:
			dx9rs->triggerMouseDownEvent(AK_LBUTTON);
			if (!wnd_fullscreen) SetCapture(hWnd);
			break;
		case WM_RBUTTONDOWN:
			dx9rs->triggerMouseDownEvent(AK_RBUTTON);
			if (!wnd_fullscreen) SetCapture(hWnd);
			break;
		case WM_LBUTTONUP:
			dx9rs->triggerMouseUpEvent(AK_LBUTTON);
			if (!wnd_fullscreen) ReleaseCapture();
			break;
		case WM_RBUTTONUP:
			dx9rs->triggerMouseUpEvent(AK_RBUTTON);
			if (!wnd_fullscreen) ReleaseCapture();
			break;
		case WM_MOUSEMOVE:
			dx9rs->triggerMouseMoveEvent();break;
		case WM_SETCURSOR:
			return 1;
		case WM_ACTIVATE:
			if (wParam == WA_ACTIVE || wParam == WA_CLICKACTIVE)
			{
				window_active=1;
				rendersys->logMessage("Window activated");
			}
			else
			{
				window_active=0;
				rendersys->logMessage("Window deactivated");
			}
			break;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}
/**********************************************************************************************/
	DirectX9RenderSystem::DirectX9RenderSystem(int w,int h,bool fullscreen,chstr title) :
		mTexCoordsEnabled(0), mColorEnabled(0), RenderSystem()
	{
		logMessage("Creating DirectX9 Rendersystem");
		mAppRunning=1;
		wnd_fullscreen=fullscreen;
		// WINDOW
		mTitle=title;
		WNDCLASSEX wc;
		ZeroMemory(&wc, sizeof(WNDCLASSEX));

		HINSTANCE hinst=GetModuleHandle(0);
		wc.cbSize = sizeof(WNDCLASSEX);
		wc.style = CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc = WindowProc;
		wc.hInstance = hinst;
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
		wc.lpszClassName = "april_d3d_window";
		wc.hIcon=(HICON) LoadImage(0,"game.ico",IMAGE_ICON,0,0,LR_LOADFROMFILE);
		wc.hbrBackground=(HBRUSH)GetStockObject(BLACK_BRUSH);

		RegisterClassEx(&wc);
		float x=(fullscreen) ? 0 : (GetSystemMetrics(SM_CXSCREEN)-w)/2,
			  y=(fullscreen) ? 0 : (GetSystemMetrics(SM_CYSCREEN)-h)/2;
		
		DWORD style=(fullscreen) ? WS_EX_TOPMOST|WS_POPUP : WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX;
		hWnd = CreateWindowEx(NULL,"april_d3d_window",title.c_str(),style,x,y,w,h,NULL,NULL,hinst,NULL);

		if (!fullscreen)
		{
			RECT rcClient, rcWindow;
			POINT ptDiff;
			GetClientRect(hWnd, &rcClient);
			GetWindowRect(hWnd, &rcWindow);
			ptDiff.x = (rcWindow.right - rcWindow.left) - rcClient.right;
			ptDiff.y = (rcWindow.bottom - rcWindow.top) - rcClient.bottom;
			MoveWindow(hWnd,rcWindow.left, rcWindow.top, w + ptDiff.x, h + ptDiff.y, TRUE);
		}
 
		// display the window on the screen
		ShowWindow(hWnd, 1);
		UpdateWindow(hWnd);
		
		SetCursor(LoadCursor(0,IDC_ARROW));
		// DIRECT3D
		d3d=Direct3DCreate9(D3D_SDK_VERSION);
		if (!d3d) throw hl_exception("Unable to create Direct3D9 object!");
		
		ZeroMemory(&d3dpp, sizeof(d3dpp));
		d3dpp.Windowed = !fullscreen;
		d3dpp.BackBufferWidth   = w;
		d3dpp.BackBufferHeight  = h;
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
		UnregisterClass("april_d3d_window",GetModuleHandle(0));
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
		if (mDynamicLoading) dynamic=1;
		if (dynamic) rendersys->logMessage("creating dynamic DX9 texture '"+filename+"'");
		DirectX9Texture* t=new DirectX9Texture(filename,dynamic);
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
			while (mAppRunning)
			{
				for (i=0;i<10;i++)
				{
					doWindowEvents();
					Sleep(100);
				}
				hr=d3dDevice->TestCooperativeLevel();
				if (hr == D3D_OK) break;
				else if (hr == D3DERR_DEVICENOTRESET)
				{
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
	
	void DirectX9RenderSystem::terminateMainLoop()
	{
		mAppRunning=0;
	}
	
	bool DirectX9RenderSystem::triggerQuitEvent()
	{
		if(mQuitCallback)
		{
			return mQuitCallback(true);
		}
		return true;
	}
	
	void DirectX9RenderSystem::triggerKeyEvent(bool down,unsigned int keycode)
	{
		if (down)
		{ if (mKeyDownCallback) mKeyDownCallback(keycode); }
		else
		{ if (mKeyUpCallback) mKeyUpCallback(keycode); }
	}
	
	void DirectX9RenderSystem::triggerCharEvent(unsigned int chr)
	{
		if (mCharCallback) mCharCallback(chr);
	}
	
	void DirectX9RenderSystem::triggerMouseUpEvent(int button)
	{
		if (mMouseUpCallback) mMouseUpCallback(cursorpos.x,cursorpos.y,button);
	}
	
	void DirectX9RenderSystem::triggerMouseDownEvent(int button)
	{
		if (mMouseDownCallback) mMouseDownCallback(cursorpos.x,cursorpos.y,button);
	}
	
	void DirectX9RenderSystem::triggerMouseMoveEvent()
	{
		if (mMouseMoveCallback) mMouseMoveCallback(cursorpos.x,cursorpos.y);
	}
	
	void DirectX9RenderSystem::enterMainLoop()
	{
		DWORD time=globalTimer.getTime(),t;
		bool cvisible=cursor_visible;
#ifdef _DEBUG
		static DWORD fpsTimer=globalTimer.getTime(),fps=0;
#endif
		POINT w32_cursorpos;
		float k;
		while (mAppRunning)
		{
			// mouse position
			GetCursorPos(&w32_cursorpos);
			ScreenToClient(hWnd,&w32_cursorpos);
			cursorpos.set(w32_cursorpos.x,w32_cursorpos.y);
			if (!cursor_visible && !wnd_fullscreen)
			{
				if (cursorpos.y < 0)
				{
					if (!cvisible) { cvisible=1; SetCursor(LoadCursor(0,IDC_ARROW)); }
				}
				else
				{
					if (cvisible) { cvisible=0; SetCursor(0); }
				}
			}
			doWindowEvents();
			t=globalTimer.getTime();

			if (t == time) continue; // don't redraw frames which won't change
			k=(t-time)/1000.0f;
			if (k > 0.5f) k=0.05f; // prevent jumps. from eg, waiting on device reset or super low framerate
			time=t;
			if (!window_active)
			{
				k=0;
				for (int i=0;i<5;i++)
				{
					doWindowEvents();
					Sleep(40);
				}
			}
			// rendering
			//d3dDevice->BeginScene();
			if (mUpdateCallback) mUpdateCallback(k);
			
#ifdef _DEBUG
			if (time-fpsTimer > 1000)
			{
				sprintf(fpstitle," [FPS: %d]",fps);
				setWindowTitle(mTitle);
				fps=0;
				fpsTimer=time;
			}
			else fps++;
#endif			
			//d3dDevice->EndScene();


			presentFrame();
		}
	}


	void createDX9RenderSystem(int w,int h,bool fullscreen,chstr title)
	{
		rendersys=new DirectX9RenderSystem(w,h,fullscreen,title);
	}

}

#endif
