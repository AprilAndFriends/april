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
#ifndef APRIL_DIRECTX9_TEXTURE_H
#define APRIL_DIRECTX9_TEXTURE_H

#include "RenderSystem.h"

struct IDirect3DTexture9;
struct IDirect3DSurface9;

namespace april
{
	class DirectX9_Texture : public Texture
	{
	public:
		IDirect3DTexture9* mTexture;
		
		DirectX9_Texture(chstr filename, bool dynamic);
		DirectX9_Texture(unsigned char* rgba, int w, int h);
		DirectX9_Texture(int w, int h, TextureFormat fmt, TextureType type);
		~DirectX9_Texture();
		
		IDirect3DSurface9* getSurface();
		IDirect3DTexture9* getTexture() { return mTexture; }

		Color getPixel(int x, int y);
		void setPixel(int x, int y, Color color);
		void fillRect(int x, int y, int w, int h, Color color);
		void blit(int x, int y, Texture* texture, int sx, int sy, int sw, int sh, unsigned char alpha = 255);
		void blit(int x, int y, unsigned char* data, int dataWidth, int dataHeight, int dataBpp, int sx, int sy, int sw, int sh, unsigned char alpha = 255);
		void stretchBlit(int x, int y, int w, int h, Texture* texture, int sx, int sy, int sw, int sh, unsigned char alpha = 255);
		void stretchBlit(int x, int y, int w, int h, unsigned char* data, int dataWidth, int dataHeight, int dataBpp, int sx, int sy, int sw, int sh, unsigned char alpha = 255);
		void clear();

		bool load();
		bool isLoaded();
		void unload();
		int getSizeInBytes();
		
	protected:
		IDirect3DSurface9* mSurface;
		
	};
}

#endif
#endif
