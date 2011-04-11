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
#include <IL/il.h>

#include "DirectX9_RenderSystem.h"
#include "DirectX9_Texture.h"
#include "ImageSource.h"

namespace april
{
	extern IDirect3DDevice9* d3dDevice;

	DirectX9_Texture::DirectX9_Texture(chstr filename, bool dynamic) : Texture()
	{
		mFilename = filename;
		mDynamic = dynamic;
		mTexture = NULL;
		mSurface = NULL;
		mWidth = 0;
		mHeight = 0;
		if (!mDynamic)
		{
			load();
		}
	}

	DirectX9_Texture::DirectX9_Texture(unsigned char* rgba, int w, int h) : Texture()
	{
		mWidth = w;
		mHeight = h;
		mDynamic = false;
		mFilename = "UserTexture";
		mUnusedTimer = 0;
		mSurface = NULL;

		april::log("Creating user-defined DX9 texture");
		HRESULT hr = d3dDevice->CreateTexture(mWidth, mHeight, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &mTexture, 0);
		if (hr != D3D_OK)
		{
			april::log("Failed to create user-defined DX9 texture!");
			return;
		}
		// write texels
		D3DLOCKED_RECT rect;
		mTexture->LockRect(0, &rect, NULL, D3DLOCK_DISCARD);
		int x;
		unsigned char* p = (unsigned char*)rect.pBits;
		for (int y = 0; y < h; y++)
		{
			for (x = 0; x < w; x++, p += 4, rgba += 4)
			{
				p[0] = rgba[2];
				p[1] = rgba[1];
				p[2] = rgba[0];
				p[3] = rgba[3];
			}
		}
		mTexture->UnlockRect(0);
	}
	
	DirectX9_Texture::DirectX9_Texture(int w, int h, TextureFormat fmt, TextureType type) : Texture()
	{
		mWidth = w;
		mHeight = h;
		mDynamic = false;
		mUnusedTimer = 0;
		mSurface = NULL;
		mFilename = "UserTexture";			
		april::log("creating empty DX9 texture [ " + hstr(w) + "x" + hstr(h) + " ]");
		D3DFORMAT d3dfmt = D3DFMT_X8R8G8B8;
		if (fmt == AT_ARGB)
		{
			d3dfmt = D3DFMT_A8R8G8B8;
		}
		D3DPOOL d3dpool = D3DPOOL_MANAGED;
		DWORD d3dusage = 0;
		if (type == AT_RENDER_TARGET)
		{
			d3dusage = D3DUSAGE_RENDERTARGET;
			d3dpool = D3DPOOL_DEFAULT;
		}

		HRESULT hr = d3dDevice->CreateTexture(mWidth, mHeight, 1, d3dusage, d3dfmt, d3dpool, &mTexture, 0);
		if (hr != D3D_OK)
		{
			april::log("Failed to create user-defined DX9 texture!");
			return;
		}
	}
	
	IDirect3DSurface9* DirectX9_Texture::getSurface()
	{
		if (!mSurface)
		{
			mTexture->GetSurfaceLevel(0, &mSurface);
		}
		return mSurface;
	}

	DirectX9_Texture::~DirectX9_Texture()
	{
		unload();
	}

	bool DirectX9_Texture::load()
	{
		mUnusedTimer = 0;
		if (mTexture)
		{
			return true;
		}
		april::log("loading DX9 texture '" + mFilename + "'");
		ImageSource* img = loadImage(mFilename);
		if (!img)
		{
			april::log("Failed to load texture '" + mFilename + "'!");
			return false;
		}
		mWidth = img->w;
		mHeight = img->h;
		HRESULT hr = d3dDevice->CreateTexture(mWidth, mHeight, 1, 0, (img->bpp == 3) ? D3DFMT_X8R8G8B8 : D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &mTexture, 0);
		if (hr != D3D_OK)
		{
			april::log("Failed to load DX9 texture!");
			delete img;
			return false;
		}
		// write texels
		D3DLOCKED_RECT rect;
		mTexture->LockRect(0, &rect, NULL, D3DLOCK_DISCARD);
		img->copyPixels(rect.pBits, img->bpp == 3 ? AF_BGR : AF_BGRA);
		//memcpy(rect.pBits,img->data,mWidth*mHeight*img->bpp);
		mTexture->UnlockRect(0);

		delete img;
		foreach (Texture*, it, mDynamicLinks)
		{
			if (!(*it)->isLoaded())
			{
				((DirectX9_Texture*)(*it))->load();
			}
		}
		return true;
	}

	bool DirectX9_Texture::isLoaded()
	{
		return (mTexture != NULL || mFilename == "UserTexture");
	}

	void DirectX9_Texture::unload()
	{
		if (mTexture)
		{
			april::log("unloading DX9 texture '" + mFilename + "'");
			mTexture->Release();
			mTexture = NULL;
			if (mSurface)
			{
				mSurface->Release();
				mSurface = NULL;
			}
		}
	}

	int DirectX9_Texture::getSizeInBytes()
	{
		return (mWidth * mHeight * 3);
	}
}

#endif
