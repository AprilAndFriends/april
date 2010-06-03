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
#include <d3dx9.h>
#include <gtypes/Vector2.h>

#define PLAIN_FVF D3DFVF_XYZ
#define COLORED_FVF D3DFVF_XYZ | D3DFVF_DIFFUSE
#define TEX_FVF D3DFVF_XYZ | D3DFVF_TEX1


namespace April
{
	IDirect3D9* d3d=0;
	IDirect3DDevice9* d3dDevice=0;
	HWND hWnd;
	class DirectX9Texture;
	DirectX9Texture* active_texture=0;

	D3DPRIMITIVETYPE dx9_render_ops[]=
	{
		D3DPT_FORCE_DWORD,
		D3DPT_TRIANGLELIST,  // ROP_TRIANGLE_LIST
		D3DPT_TRIANGLESTRIP, // ROP_TRIANGLE_STRIP
		D3DPT_TRIANGLEFAN,   // ROP_TRIANGLE_FAN
		D3DPT_LINELIST,      // ROP_LINE_LIST
		D3DPT_LINESTRIP,     // ROP_LINE_STRIP
	};

	unsigned int numPrimitives(RenderOp rop,int nVertices)
	{
		if (rop == TriangleList)  return nVertices/3;
		if (rop == TriangleStrip) return nVertices-2;
		if (rop == TriangleFan)   return nVertices-1;
		if (rop == LineList)      return nVertices/2;
		if (rop == LineStrip)     return nVertices-1;
		return 0;
	}

	class DirectX9Texture : public Texture
	{
	public:
		IDirect3DTexture9* mTexture;
		DirectX9Texture(std::string filename,bool dynamic)
		{
			mFilename=filename;
			mDynamic=dynamic;
			mTexture=0;
			if (mDynamic)
			{
				mWidth=mHeight=0;
			}
			else
			{
				load();
			}
		}

		DirectX9Texture(unsigned char* rgba,int w,int h)
		{
			mWidth=w; mHeight=h;
			mDynamic=0;
			mFilename="UserTexture";
		}

		~DirectX9Texture()
		{
			unload();
		}

		bool load()
		{
			mUnusedTimer=0;
			if (mTexture) return 1;
			rendersys->logMessage("loading DX9 texture '"+mFilename+"'");
			HRESULT hr=D3DXCreateTextureFromFile(d3dDevice,mFilename.c_str(),&mTexture);
			if (hr != D3D_OK) rendersys->logMessage("Failed to load DX9 texture!");
			return 1;
		}

		bool isLoaded()
		{
			return mTexture != 0 || mFilename == "UserTexture";
		}

		void unload()
		{
			if (mTexture)
			{
				rendersys->logMessage("unloading DX9 texture '"+mFilename+"'");
				mTexture->Release();
			}
		}

		int getSizeInBytes()
		{
			return mWidth*mHeight*3;
		}
	};
/**********************************************************************************************/
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
        case WM_DESTROY:
			PostQuitMessage(0);
			rendersys->terminateMainLoop();
			return 0;
			break;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
} 
/**********************************************************************************************/
	DirectX9RenderSystem::DirectX9RenderSystem(int w,int h,bool fullscreen,std::string title) :
		mTexCoordsEnabled(0), mColorEnabled(0)
	{
		logMessage("Creating DirectX9 Rendersystem");
		mAppRunning=1;
		// WINDOW
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

		RegisterClassEx(&wc);

		hWnd = CreateWindowEx(NULL,
							  "april_d3d_window",    // name of the window class
							  title.c_str(),   // title of the window
							  WS_OVERLAPPEDWINDOW,    // window style
							  300,    // x-position of the window
							  300,    // y-position of the window
							  800,    // width of the window
							  600,    // height of the window
							  NULL,    // we have no parent window, NULL
							  NULL,    // we aren't using menus, NULL
							  hinst,    // application handle
							  NULL);    // used with multiple windows, NULL

		// display the window on the screen
		ShowWindow(hWnd, 1);
		// DIRECT3D
		d3d=Direct3DCreate9(D3D_SDK_VERSION);
		if (!d3d) throw "Unable to create Direct3D9 object!";
		
		D3DPRESENT_PARAMETERS d3dpp;

		ZeroMemory(&d3dpp, sizeof(d3dpp));
		d3dpp.Windowed = 1;
		d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
		d3dpp.hDeviceWindow = hWnd;
		HRESULT hr=d3d->CreateDevice(D3DADAPTER_DEFAULT,D3DDEVTYPE_HAL,hWnd,D3DCREATE_SOFTWARE_VERTEXPROCESSING,&d3dpp,&d3dDevice);
		if (hr != D3D_OK) throw "Unable to create Direct3D Device!";
		// device config
		d3dDevice->SetRenderState(D3DRS_LIGHTING,false);
		d3dDevice->SetRenderState(D3DRS_CULLMODE,D3DCULL_NONE);
		d3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		d3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);


	}
	DirectX9RenderSystem::~DirectX9RenderSystem()
	{
		logMessage("Destroying DirectX9 Rendersystem");
		UnregisterClass("april_d3d_window",GetModuleHandle(0));
		if (d3dDevice) d3dDevice->Release();
		if (d3d) d3d->Release();
		d3d=0; d3dDevice=0;
	}

	std::string DirectX9RenderSystem::getName()
	{
		return "DirectX9";
	}

	Texture* DirectX9RenderSystem::loadTexture(std::string filename,bool dynamic)
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
		rendersys->logMessage("creating user-defined DX9 texture");
		DirectX9Texture* t=new DirectX9Texture(rgba,w,h);
		return t;
	}

	void DirectX9RenderSystem::setTexture(Texture* t)
	{
		active_texture=(DirectX9Texture*) t;
		if (active_texture)
			d3dDevice->SetTexture(0,active_texture->mTexture);
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
	}

	void DirectX9RenderSystem::render(RenderOp renderOp,TexturedVertex* v,int nVertices)
	{
		d3dDevice->SetFVF(TEX_FVF);
		d3dDevice->DrawPrimitiveUP(dx9_render_ops[renderOp],numPrimitives(renderOp,nVertices),v,sizeof(TexturedVertex));
	}

	void DirectX9RenderSystem::render(RenderOp renderOp,TexturedVertex* v,int nVertices,float r,float g,float b,float a)
	{
		
	}

	ColoredVertex static_cv[100];
	void DirectX9RenderSystem::render(RenderOp renderOp,PlainVertex* v,int nVertices)
	{
		ColoredVertex* cv=(nVertices <= 100) ? static_cv : new ColoredVertex[nVertices];
		ColoredVertex* p=cv;
		for (int i=0;i<nVertices;i++,p++,v++)
		{ p->x=v->x; p->y=v->y; p->z=v->z; p->color=0xFFFFFFFF; }
		render(renderOp,cv,nVertices);
		if (nVertices > 100) delete [] cv;
	}

	void DirectX9RenderSystem::render(RenderOp renderOp,PlainVertex* v,int nVertices,float r,float g,float b,float a)
	{
		DWORD color=D3DCOLOR_ARGB((int) (a*255.0f),(int) (r*255.0f),(int) (g*255.0f),(int) (b*255.0f));
		ColoredVertex* cv=(nVertices <= 100) ? static_cv : new ColoredVertex[nVertices];
		ColoredVertex* p=cv;
		for (int i=0;i<nVertices;i++,p++,v++)
		{ p->x=v->x; p->y=v->y; p->z=v->z; p->color=color; }
		render(renderOp,cv,nVertices);
		if (nVertices > 100) delete [] cv;
	}
	
	int DirectX9RenderSystem::getWindowWidth()
	{
		return 800;
	}
	
	int DirectX9RenderSystem::getWindowHeight()
	{
		return 600;
	}

	void DirectX9RenderSystem::render(RenderOp renderOp,ColoredVertex* v,int nVertices)
	{
			if (active_texture) setTexture(0);
		d3dDevice->SetFVF(COLORED_FVF);
		d3dDevice->DrawPrimitiveUP(dx9_render_ops[renderOp],numPrimitives(renderOp,nVertices),v,sizeof(ColoredVertex));
	}

	void DirectX9RenderSystem::setAlphaMultiplier(float value)
	{
		mAlphaMultiplier=value;
	//	glColor4f(1,1,1,value);
	}

	void DirectX9RenderSystem::setWindowTitle(std::string title)
	{

	}
	
	gtypes::Vector2 DirectX9RenderSystem::getCursorPos()
	{
		return gtypes::Vector2(0,0);
	}

	void DirectX9RenderSystem::showSystemCursor(bool b)
	{

	}
	
	bool DirectX9RenderSystem::isSystemCursorShown()
	{

	}

	void DirectX9RenderSystem::presentFrame()
	{
		d3dDevice->Present(NULL, NULL, NULL, NULL);
	}
	
	void DirectX9RenderSystem::terminateMainLoop()
	{
		mAppRunning=0;
	}
	
	void DirectX9RenderSystem::enterMainLoop()
	{
		MSG msg;
		DWORD time=GetTickCount(),t;
		while (mAppRunning)
		{
			if (PeekMessage(&msg,hWnd,0,0,PM_REMOVE))
			{
			//	GetMessage(&msg, NULL, 0, 0));
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			d3dDevice->BeginScene();
			
			t=GetTickCount();
			if (mUpdateCallback) mUpdateCallback((t-time)/1000.0f);
			time=t;
			
			d3dDevice->EndScene();

			presentFrame();
		}
	}


	void createDX9RenderSystem(int w,int h,bool fullscreen,std::string title)
	{
		rendersys=new DirectX9RenderSystem(w,h,fullscreen,title);
	}

}

#endif
