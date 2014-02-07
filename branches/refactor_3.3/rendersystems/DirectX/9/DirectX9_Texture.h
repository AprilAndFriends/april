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
		bool load();
		void unload();
		
		bool isLoaded();
		
		void clear();
		Color getPixel(int x, int y);
		void setPixel(int x, int y, Color color);
		void fillRect(int x, int y, int w, int h, Color color);
		void write(int x, int y, int w, int h, unsigned char* data, Image::Format format);
		bool copyPixelData(unsigned char** output, Image::Format format);
		void blit(int x, int y, Texture* texture, int sx, int sy, int sw, int sh, unsigned char alpha = 255);
		void blit(int x, int y, unsigned char* data, int dataWidth, int dataHeight, int dataBpp, int sx, int sy, int sw, int sh, unsigned char alpha = 255);
		void stretchBlit(int x, int y, int w, int h, Texture* texture, int sx, int sy, int sw, int sh, unsigned char alpha = 255);
		void stretchBlit(int x, int y, int w, int h, unsigned char* data, int dataWidth, int dataHeight, int dataBpp, int sx, int sy, int sw, int sh, unsigned char alpha = 255);
		void rotateHue(float degrees);
		void saturate(float factor);
		void insertAsAlphaMap(Texture* source, unsigned char median, int ambiguity);

		void restore(); // TODOaa - currently only a hack for rendertarget textures

	protected:
		IDirect3DSurface9* d3dSurface;
		IDirect3DTexture9* d3dTexture;
		D3DFORMAT d3dFormat;
		bool renderTarget;

		enum LOCK_RESULT
		{
			LR_LOCKED,
			LR_RENDERTARGET,
			LR_FAILED
		};

		bool _create(chstr filename, Type type);
		bool _create(int w, int h, unsigned char* data, Image::Format format, Type type);
		bool _create(int w, int h, Color color, Image::Format format, Type type);
		void _assignFormat();

		IDirect3DSurface9* _getSurface();

		LOCK_RESULT _tryLock(IDirect3DSurface9** buffer, D3DLOCKED_RECT* lockRect, RECT* rect);
		void _unlock(IDirect3DSurface9* buffer, LOCK_RESULT lock, bool update);
		bool _uploadDataToGpu(int x, int y, int w, int h);

	};

}

#endif
#endif
