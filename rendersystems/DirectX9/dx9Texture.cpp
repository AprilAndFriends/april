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
#include "dx9Texture.h"
#include "ImageSource.h"
#include <d3d9.h>
#include <IL/il.h>

namespace April
{
	extern IDirect3DDevice9* d3dDevice;

	DirectX9Texture::DirectX9Texture(chstr filename,bool dynamic)
	{
		mFilename=filename;
		mDynamic=dynamic;
		mTexture=0;
		mSurface=0;
		if (mDynamic)
		{
			mWidth=mHeight=0;
		}
		else
		{
			load();
		}
	}

	DirectX9Texture::DirectX9Texture(unsigned char* rgba,int w,int h)
	{
		mWidth=w; mHeight=h;
		mDynamic=0;
		mFilename="UserTexture";
		mUnusedTimer=0;
		mSurface=0;

		rendersys->logMessage("Creating user-defined DX9 texture");
		HRESULT hr=d3dDevice->CreateTexture(mWidth,mHeight,1,0,D3DFMT_A8R8G8B8,D3DPOOL_MANAGED,&mTexture,0);
		if (hr != D3D_OK) { rendersys->logMessage("Failed to create user-defined DX9 texture!"); return; }
		// write texels
		D3DLOCKED_RECT rect;
		mTexture->LockRect(0,&rect,NULL,D3DLOCK_DISCARD);
		int x,y;
		unsigned char* p=(unsigned char*) rect.pBits;
		for (y=0;y<h;y++)
		{
			for (x=0;x<w;x++,p+=4,rgba+=4)
			{
				p[0]=rgba[2];
				p[1]=rgba[1];
				p[2]=rgba[0];
				p[3]=rgba[3];
			}
		}
		mTexture->UnlockRect(0);
	}
	
	DirectX9Texture::DirectX9Texture(int w,int h,TextureFormat fmt,TextureType type)
	{
		mWidth=w; mHeight=h;
		mDynamic=0;
		mUnusedTimer=0;
		mSurface=0;
		mFilename="UserTexture";			
		rendersys->logMessage("creating empty DX9 texture [ "+hstr(w)+"x"+hstr(h)+" ]");
		D3DFORMAT d3dfmt=D3DFMT_X8R8G8B8;
		if (fmt == AT_A8R8G8B8) d3dfmt=D3DFMT_A8R8G8B8;
		D3DPOOL d3dpool=D3DPOOL_MANAGED;
		DWORD d3dusage=0;
		if (type == AT_RENDER_TARGET)
		{
			d3dusage=D3DUSAGE_RENDERTARGET;
			d3dpool=D3DPOOL_DEFAULT;
		}

		HRESULT hr=d3dDevice->CreateTexture(mWidth,mHeight,1,d3dusage,d3dfmt,d3dpool,&mTexture,0);
		if (hr != D3D_OK) { rendersys->logMessage("Failed to create user-defined DX9 texture!"); return; }
	}
	
	IDirect3DSurface9* DirectX9Texture::getSurface()
	{
		if (!mSurface) mTexture->GetSurfaceLevel(0,&mSurface);
		return mSurface;
	}

	DirectX9Texture::~DirectX9Texture()
	{
		unload();
	}

	bool DirectX9Texture::load()
	{
		mUnusedTimer=0;
		if (mTexture) return 1;
		rendersys->logMessage("loading DX9 texture '"+mFilename+"'");
		ImageSource* img=loadImage(mFilename);
		if (!img) { rendersys->logMessage("Failed to load texture '"+mFilename+"'!"); return 0; }
		mWidth=img->w; mHeight=img->h;
		HRESULT hr=d3dDevice->CreateTexture(mWidth,mHeight,1,0,(img->bpp == 3) ? D3DFMT_X8R8G8B8 : D3DFMT_A8R8G8B8,D3DPOOL_MANAGED,&mTexture,0);
		if (hr != D3D_OK) { rendersys->logMessage("Failed to load DX9 texture!"); delete img; return 0; }
		// write texels
		D3DLOCKED_RECT rect;
		mTexture->LockRect(0,&rect,NULL,D3DLOCK_DISCARD);
		img->copyPixels(rect.pBits,IL_BGRA);
		//memcpy(rect.pBits,img->data,mWidth*mHeight*img->bpp);
		mTexture->UnlockRect(0);

		delete img;
		return 1;
	}

	bool DirectX9Texture::isLoaded()
	{
		return mTexture != 0 || mFilename == "UserTexture";
	}

	void DirectX9Texture::unload()
	{
		if (mTexture)
		{
			rendersys->logMessage("unloading DX9 texture '"+mFilename+"'");
			mTexture->Release();
			mTexture=0;
			if (mSurface) { mSurface->Release(); mSurface=0; }
		}
	}

	int DirectX9Texture::getSizeInBytes()
	{
		return mWidth*mHeight*3;
	}
}

#endif
