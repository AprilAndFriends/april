/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @version 2.5
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

#include "RenderSystem.h"

struct IDirect3DTexture9;
struct IDirect3DSurface9;

namespace april
{
	class DirectX9_RenderSystem;

	class DirectX9_Texture : public Texture
	{
	public:
		friend class DirectX9_RenderSystem;

		DirectX9_Texture(chstr filename);
		DirectX9_Texture(int w, int h, unsigned char* rgba);
		DirectX9_Texture(int w, int h, Format format, Type type, Color color = Color::Clear);
		~DirectX9_Texture();
		bool load();
		void unload();
		
		bool isLoaded();
		
		void clear();
		Color getPixel(int x, int y);
		void setPixel(int x, int y, Color color);
		void fillRect(int x, int y, int w, int h, Color color);
		void blit(int x, int y, Texture* texture, int sx, int sy, int sw, int sh, unsigned char alpha = 255);
		void blit(int x, int y, unsigned char* data, int dataWidth, int dataHeight, int dataBpp, int sx, int sy, int sw, int sh, unsigned char alpha = 255);
		void stretchBlit(int x, int y, int w, int h, Texture* texture, int sx, int sy, int sw, int sh, unsigned char alpha = 255);
		void stretchBlit(int x, int y, int w, int h, unsigned char* data, int dataWidth, int dataHeight, int dataBpp, int sx, int sy, int sw, int sh, unsigned char alpha = 255);
		void rotateHue(float degrees);
		void saturate(float factor);
		bool copyPixelData(unsigned char** output);
		void insertAsAlphaMap(Texture* source, unsigned char median, int ambiguity);

		void restore(); // TODO - currently only a hack for rendertarget textures

	protected:
		IDirect3DSurface9* d3dSurface;
		IDirect3DTexture9* d3dTexture;
		bool renderTarget;

		enum LOCK_RESULT
		{
			LR_LOCKED,
			LR_RENDERTARGET,
			LR_FAILED
		};

		IDirect3DTexture9* _getTexture() { return this->d3dTexture; }
		IDirect3DSurface9* _getSurface();

		LOCK_RESULT _tryLock(IDirect3DSurface9** buffer, D3DLOCKED_RECT* lockRect, RECT* rect);
		void _unlock(IDirect3DSurface9* buffer, LOCK_RESULT lock, bool update);

	};

}

#endif
#endif
