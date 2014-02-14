/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @version 3.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/// 
/// @section DESCRIPTION
/// 
/// Defines a DirectX9 specific texture.

#ifdef _DIRECTX9
#ifndef APRIL_DIRECTX9_TEXTURE_H
#define APRIL_DIRECTX9_TEXTURE_H

#include "DirectX_Texture.h"

struct IDirect3DTexture9;
struct IDirect3DSurface9;

namespace april
{
	class DirectX9_RenderSystem;

	class DirectX9_Texture : public DirectX_Texture
	{
	public:
		friend class DirectX9_RenderSystem;

		DirectX9_Texture();
		~DirectX9_Texture();
		void unload();
		
		bool isLoaded();
		
		bool copyPixelData(unsigned char** output, Image::Format format);
		bool write(int sx, int sy, int sw, int sh, int dx, int dy, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat);
		bool write(int sx, int sy, int sw, int sh, int dx, int dy, Texture* texture);
		bool writeStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat);
		bool writeStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, Texture* texture);
		bool blit(int sx, int sy, int sw, int sh, int dx, int dy, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat, unsigned char alpha = 255);
		bool blit(int sx, int sy, int sw, int sh, int dx, int dy, Texture* texture, unsigned char alpha = 255);
		bool blitStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, unsigned char* srcData, int srcWidth, int srcHeight, Image::Format srcFormat, unsigned char alpha = 255);
		bool blitStretch(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, Texture* texture, unsigned char alpha = 255);
		bool rotateHue(int x, int y, int w, int h, float degrees);
		bool saturate(int x, int y, int w, int h, float factor);
		bool invert(int x, int y, int w, int h);
		bool insertAlphaMap(unsigned char* srcData, Image::Format srcFormat, unsigned char median, int ambiguity);
		bool insertAlphaMap(Texture* texture, unsigned char median, int ambiguity);

	protected:
		IDirect3DSurface9* d3dSurface;
		IDirect3DTexture9* d3dTexture;
		D3DFORMAT d3dFormat;
		D3DPOOL d3dPool;
		DWORD d3dUsage;
		bool renderTarget;

		bool _createInternalTexture(unsigned char* data, int size, Type type);
		void _assignFormat();

		IDirect3DSurface9* _getSurface();

		Lock _tryLockSystem(int x, int y, int w, int h);
		bool _unlockSystem(Lock& lock);

	};

}

#endif
#endif
