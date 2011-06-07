/************************************************************************************\
This source file is part of the Awesome Portable Rendering Interface Library         *
For latest info, see http://libapril.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes                                                     *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#ifndef DX9_TEXTURE_H
#ifdef _DIRECTX9
#define DX9_TEXTURE_H

#include "RenderSystem.h"
class IDirect3DTexture9;
class IDirect3DSurface9;

namespace april
{
	class DirectX9Texture : public Texture
	{
		IDirect3DSurface9* mSurface;
	public:
		IDirect3DTexture9* mTexture;
		
		DirectX9Texture(chstr filename,bool dynamic);
		DirectX9Texture(unsigned char* rgba,int w,int h);
		DirectX9Texture(int w,int h,TextureFormat fmt,TextureType type);
		~DirectX9Texture();
		
		IDirect3DSurface9* getSurface();
		bool load();
		bool isLoaded();
		void unload();
		int getSizeInBytes();
	};
}
            
#endif
#endif
