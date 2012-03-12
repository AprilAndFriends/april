/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @version 1.51
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _DIRECTX9

#include <d3d9.h>
#include <IL/il.h>

#include "april.h"
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
		mBpp = 4;
		mDynamic = false;
		mFilename = "";
		mUnusedTimer = 0;
		mSurface = NULL;

		april::log("Creating user-defined DX9 texture");
		HRESULT hr = d3dDevice->CreateTexture(mWidth, mHeight, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &mTexture, NULL);
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
		for_iter (y, 0, h)
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
		mFilename = "";			
		april::log("creating empty DX9 texture [ " + hstr(w) + "x" + hstr(h) + " ]");
		D3DFORMAT d3dfmt = D3DFMT_X8R8G8B8;
		mBpp = 3;
		if (fmt == AT_ARGB)
		{
			d3dfmt = D3DFMT_A8R8G8B8;
			mBpp = 4;
		}
		D3DPOOL d3dpool = D3DPOOL_MANAGED;
		DWORD d3dusage = 0;
		if (type == AT_RENDER_TARGET)
		{
			d3dusage = D3DUSAGE_RENDERTARGET;
			d3dpool = D3DPOOL_DEFAULT;
		}

		HRESULT hr = d3dDevice->CreateTexture(mWidth, mHeight, 1, d3dusage, d3dfmt, d3dpool, &mTexture, NULL);
		if (hr != D3D_OK)
		{
			april::log("Failed to create user-defined DX9 texture!");
			return;
		}
	}
	
	DirectX9_Texture::~DirectX9_Texture()
	{
		unload();
	}

	Color DirectX9_Texture::getPixel(int x, int y)
	{
		Color color;
		D3DLOCKED_RECT lockRect;
		RECT rect;
		rect.left = x;
		rect.top = y;
		rect.right = x;
		rect.bottom = y;
		IDirect3DSurface9* buffer = NULL;
		LOCK_RESULT result = _tryLock(&buffer, &lockRect, &rect);
		if (result == LR_FAILED)
		{
			return color;
		}
		unsigned char* p = (unsigned char*)lockRect.pBits;
		color.r = p[2];
		color.g = p[1];
		color.b = p[0];
		color.a = p[3];
		_unlock(buffer, result, false);
		return color;
	}

	void DirectX9_Texture::setPixel(int x, int y, Color color)
	{
		x = hclamp(x, 0, this->mWidth - 1);
		y = hclamp(y, 0, this->mHeight - 1);
		D3DLOCKED_RECT lockRect;
		RECT rect;
		rect.left = x;
		rect.right = x;
		rect.top = y;
		rect.bottom = y;
		IDirect3DSurface9* buffer = NULL;
		LOCK_RESULT result = _tryLock(&buffer, &lockRect, &rect);
		if (result == LR_FAILED)
		{
			return;
		}
		unsigned char* p = (unsigned char*)lockRect.pBits;
		p[2] = color.r;
		p[1] = color.g;
		p[0] = color.b;
		p[3] = (mBpp == 4 ? color.a : 255);
		_unlock(buffer, result, true);
	}

	void DirectX9_Texture::fillRect(int x, int y, int w, int h, Color color)
	{
		x = hclamp(x, 0, this->mWidth - 1);
		y = hclamp(y, 0, this->mHeight - 1);
		w = hclamp(w, 1, this->mWidth - x);
		h = hclamp(h, 1, this->mHeight - y);
		if (w == 1 && h == 1)
		{
			this->setPixel(x, y, color);
			return;
		}
		D3DLOCKED_RECT lockRect;
		RECT rect;
		rect.left = x;
		rect.right = x + w - 1;
		rect.top = y;
		rect.bottom = y + h - 1;
		IDirect3DSurface9* buffer = NULL;
		LOCK_RESULT result = _tryLock(&buffer, &lockRect, &rect);
		if (result == LR_FAILED)
		{
			return;
		}
		unsigned char* p = (unsigned char*)lockRect.pBits;
		int i;
		int offset;
		if (mBpp == 4)
		{
			for_iter (j, 0, h)
			{
				for_iterx (i, 0, w)
				{
					offset = (j * mWidth + i) * 4;
					p[offset + 2] = color.r;
					p[offset + 1] = color.g;
					p[offset + 0] = color.b;
					p[offset + 3] = color.a;
				}
			}
		}
		else
		{
			for_iter (j, 0, h)
			{
				for_iterx (i, 0, w)
				{
					offset = (i + j * mWidth) * 4;
					p[offset + 2] = color.r;
					p[offset + 1] = color.g;
					p[offset + 0] = color.b;
				}
			}
		}
		_unlock(buffer, result, true);
	}

	void DirectX9_Texture::blit(int x, int y, Texture* texture, int sx, int sy, int sw, int sh, unsigned char alpha)
	{
		DirectX9_Texture* source = (DirectX9_Texture*)texture;
		x = hclamp(x, 0, mWidth - 1);
		y = hclamp(y, 0, mHeight - 1);
		sx = hclamp(sx, 0, source->mWidth - 1);
		sy = hclamp(sy, 0, source->mHeight - 1);
		sw = hmin(sw, hmin(mWidth - x, source->mWidth - sx));
		sh = hmin(sh, hmin(mHeight - y, source->mHeight - sy));
		if (sw == 1 && sh == 1)
		{
			this->setPixel(x, y, source->getPixel(sx, sy));
			return;
		}
		D3DLOCKED_RECT lockRect;
		RECT rect;
		rect.left = sx;
		rect.right = sx + sw - 1;
		rect.top = sy;
		rect.bottom = sy + sh - 1;
		IDirect3DSurface9* buffer = NULL;
		LOCK_RESULT result = source->_tryLock(&buffer, &lockRect, &rect);
		if (result == LR_FAILED)
		{
			return;
		}
		blit(x, y, (unsigned char*)lockRect.pBits, source->mWidth, source->mHeight, source->mBpp, sx, sy, sw, sh, alpha);
		_unlock(buffer, result, false);
	}

	void DirectX9_Texture::blit(int x, int y, unsigned char* data, int dataWidth, int dataHeight, int dataBpp, int sx, int sy, int sw, int sh, unsigned char alpha)
	{
		x = hclamp(x, 0, mWidth - 1);
		y = hclamp(y, 0, mHeight - 1);
		sx = hclamp(sx, 0, dataWidth - 1);
		sy = hclamp(sy, 0, dataHeight - 1);
		sw = hmin(sw, hmin(mWidth - x, dataWidth - sx));
		sh = hmin(sh, hmin(mHeight - y, dataHeight - sy));
		D3DLOCKED_RECT lockRect;
		RECT rect;
		rect.left = x;
		rect.right = x + sw;
		rect.top = y;
		rect.bottom = y + sh;
		IDirect3DSurface9* buffer = NULL;
		LOCK_RESULT result = _tryLock(&buffer, &lockRect, &rect);
		if (result == LR_FAILED)
		{
			return;
		}
		unsigned char* thisData = (unsigned char*)lockRect.pBits;
		unsigned char* srcData = data;
		unsigned char* c;
		unsigned char* sc;
		unsigned char a0;
		unsigned char a1;
		int i;
		// the following iteration blocks are very similar, but for performance reasons they
		// have been duplicated instead of putting everything into one block with if branches
		if (mBpp == 4 && dataBpp == 4)
		{
			for_iter (j, 0, sh)
			{
				for_iterx (i, 0, sw)
				{
					c = &thisData[(i + j * mWidth) * 4];
					sc = &srcData[(i + j * dataWidth) * 4];
					if (c[3] > 0)
					{
						a0 = sc[3] * alpha / 255;
						a1 = 255 - a0;
						c[2] = (sc[2] * a0 + c[2] * a1) / 255;
						c[1] = (sc[1] * a0 + c[1] * a1) / 255;
						c[0] = (sc[0] * a0 + c[0] * a1) / 255;
						c[3] = a0 + c[3] * a1 / 255;
					}
					else
					{
						c[2] = sc[2];
						c[1] = sc[1];
						c[0] = sc[0];
						c[3] = sc[3] * alpha / 255;
					}
				}
			}
		}
		else if (dataBpp == 4)
		{
			for_iter (j, 0, sh)
			{
				for_iterx (i, 0, sw)
				{
					c = &thisData[(i + j * mWidth) * 4];
					sc = &srcData[(i + j * dataWidth) * 4];
					a0 = sc[3] * alpha / 255;
					a1 = 255 - a0;
					c[2] = (sc[2] * a0 + c[2] * a1) / 255;
					c[1] = (sc[1] * a0 + c[1] * a1) / 255;
					c[0] = (sc[0] * a0 + c[0] * a1) / 255;
				}
			}
		}
		else if (alpha < 255)
		{
			a0 = alpha;
			a1 = 255 - a0;
			for_iter (j, 0, sh)
			{
				for_iterx (i, 0, sw)
				{
					c = &thisData[(i + j * mWidth) * 4];
					sc = &srcData[(i + j * dataWidth) * 4];
					c[2] = (sc[2] * a0 + c[2] * a1) / 255;
					c[1] = (sc[1] * a0 + c[1] * a1) / 255;
					c[0] = (sc[0] * a0 + c[0] * a1) / 255;
				}
			}
		}
		else
		{
			for_iter (j, 0, sh)
			{
				for_iterx (i, 0, sw)
				{
					c = &thisData[(i + j * mWidth) * 4];
					sc = &srcData[(i + j * dataWidth) * 4];
					c[2] = sc[2];
					c[1] = sc[1];
					c[0] = sc[0];
				}
			}
		}
		_unlock(buffer, result, true);
	}

	void DirectX9_Texture::stretchBlit(int x, int y, int w, int h, Texture* texture, int sx, int sy, int sw, int sh, unsigned char alpha)
	{
		DirectX9_Texture* source = (DirectX9_Texture*)texture;
		x = hclamp(x, 0, this->mWidth - 1);
		y = hclamp(y, 0, this->mHeight - 1);
		w = hmin(w, this->mWidth - x);
		h = hmin(h, this->mHeight - y);
		sx = hclamp(sx, 0, source->mWidth - 1);
		sy = hclamp(sy, 0, source->mHeight - 1);
		sw = hmin(sw, source->mWidth - sx);
		sh = hmin(sh, source->mHeight - sy);
		D3DLOCKED_RECT lockRect;
		HRESULT result = source->getTexture()->LockRect(0, &lockRect, NULL, D3DLOCK_DISCARD);
		if (result == D3D_OK)
		{
			stretchBlit(x, y, w, h, (unsigned char*)lockRect.pBits, source->mWidth, source->mHeight, source->mBpp, sx, sy, sw, sh, alpha);
			source->getTexture()->UnlockRect(0);
		}
	}

	void DirectX9_Texture::stretchBlit(int x, int y, int w, int h, unsigned char* data, int dataWidth, int dataHeight, int dataBpp, int sx, int sy, int sw, int sh, unsigned char alpha)
	{
		x = hclamp(x, 0, mWidth - 1);
		y = hclamp(y, 0, mHeight - 1);
		w = hmin(w, mWidth - x);
		h = hmin(h, mHeight - y);
		sx = hclamp(sx, 0, dataWidth - 1);
		sy = hclamp(sy, 0, dataHeight - 1);
		sw = hmin(sw, dataWidth - sx);
		sh = hmin(sh, dataHeight - sy);
		D3DLOCKED_RECT lockRect;
		RECT rect;
		rect.left = x;
		rect.right = x + w - 1;
		rect.top = y;
		rect.bottom = y + h - 1;
		IDirect3DSurface9* buffer = NULL;
		LOCK_RESULT result = _tryLock(&buffer, &lockRect, &rect);
		if (result == LR_FAILED)
		{
			return;
		}
		unsigned char* thisData = (unsigned char*)lockRect.pBits;
		unsigned char* srcData = data;
		float fw = (float)sw / w;
		float fh = (float)sh / h;
		unsigned char* c;
		unsigned char* sc;
		int a0;
		int a1;
		unsigned char color[4] = {0};
		unsigned char* ctl;
		unsigned char* ctr;
		unsigned char* cbl;
		unsigned char* cbr;
		float cx;
		float cy;
		float rx0;
		float ry0;
		float rx1;
		float ry1;
		int x0;
		int y0;
		int x1;
		int y1;
		int i;
		// the following iteration blocks are very similar, but for performance reasons they
		// have been duplicated instead of putting everything into one block with if branches
		if (mBpp == 4 && dataBpp == 4)
		{
			for_iter (j, 0, h)
			{
				for_iterx (i, 0, w)
				{
					c = &thisData[(i + j * mWidth) * 4];
					cx = sx + i * fw;
					cy = sy + j * fh;
					x0 = (int)cx;
					y0 = (int)cy;
					x1 = hmin((int)cx + 1, dataWidth - 1);
					y1 = hmin((int)cy + 1, dataHeight - 1);
					rx0 = cx - x0;
					ry0 = cy - y0;
					rx1 = 1.0f - rx0;
					ry1 = 1.0f - ry0;
					if (rx0 != 0.0f || ry0 != 0.0f)
					{
						ctl = &srcData[(x0 + y0 * dataWidth) * 4];
						ctr = &srcData[(x1 + y0 * dataWidth) * 4];
						cbl = &srcData[(x0 + y1 * dataWidth) * 4];
						cbr = &srcData[(x1 + y1 * dataWidth) * 4];
						color[2] = (unsigned char)(((ctl[2] * ry1 + cbl[2] * ry0) * rx1 + (ctr[2] * ry1 + cbr[2] * ry0) * rx0));
						color[1] = (unsigned char)(((ctl[1] * ry1 + cbl[1] * ry0) * rx1 + (ctr[1] * ry1 + cbr[1] * ry0) * rx0));
						color[0] = (unsigned char)(((ctl[0] * ry1 + cbl[0] * ry0) * rx1 + (ctr[0] * ry1 + cbr[0] * ry0) * rx0));
						color[3] = (unsigned char)(((ctl[3] * ry1 + cbl[3] * ry0) * rx1 + (ctr[3] * ry1 + cbr[3] * ry0) * rx0));
						sc = color;
					}
					else if (rx0 != 0.0f)
					{
						ctl = &srcData[(x0 + y0 * dataWidth) * 4];
						ctr = &srcData[(x1 + y0 * dataWidth) * 4];
						color[2] = (unsigned char)((ctl[2] * rx1 + ctr[2] * rx0));
						color[1] = (unsigned char)((ctl[1] * rx1 + ctr[1] * rx0));
						color[0] = (unsigned char)((ctl[0] * rx1 + ctr[0] * rx0));
						color[3] = (unsigned char)((ctl[3] * rx1 + ctr[3] * rx0));
						sc = color;
					}
					else if (ry0 != 0.0f)
					{
						ctl = &srcData[(x0 + y0 * dataWidth) * 4];
						cbl = &srcData[(x0 + y1 * dataWidth) * 4];
						color[2] = (unsigned char)((ctl[2] * ry1 + cbl[2] * ry0));
						color[1] = (unsigned char)((ctl[1] * ry1 + cbl[1] * ry0));
						color[0] = (unsigned char)((ctl[0] * ry1 + cbl[0] * ry0));
						color[3] = (unsigned char)((ctl[3] * ry1 + cbl[3] * ry0));
						sc = color;
					}
					else
					{
						sc = &srcData[(x0 + y0 * dataWidth) * 4];
					}
					if (c[3] > 0)
					{
						a0 = sc[3] * (int)alpha / 255;
						a1 = 255 - a0;
						c[2] = (unsigned char)((sc[2] * a0 + c[2] * a1) / 255);
						c[1] = (unsigned char)((sc[1] * a0 + c[1] * a1) / 255);
						c[0] = (unsigned char)((sc[0] * a0 + c[0] * a1) / 255);
						c[3] = (unsigned char)(a0 + c[3] * a1 / 255);
					}
					else
					{
						c[2] = sc[2];
						c[1] = sc[1];
						c[0] = sc[0];
						c[3] = sc[3] * (int)alpha / 255;
					}
				}
			}
		}
		else if (dataBpp == 4)
		{
			for_iter (j, 0, h)
			{
				for_iterx (i, 0, w)
				{
					c = &thisData[(i + j * mWidth) * 4];
					cx = sx + i * fw;
					cy = sy + j * fh;
					x0 = (int)cx;
					y0 = (int)cy;
					x1 = hmin((int)cx + 1, dataWidth - 1);
					y1 = hmin((int)cy + 1, dataHeight - 1);
					rx0 = cx - x0;
					ry0 = cy - y0;
					rx1 = 1.0f - rx0;
					ry1 = 1.0f - ry0;
					if (rx0 != 0.0f || ry0 != 0.0f)
					{
						ctl = &srcData[(x0 + y0 * dataWidth) * 4];
						ctr = &srcData[(x1 + y0 * dataWidth) * 4];
						cbl = &srcData[(x0 + y1 * dataWidth) * 4];
						cbr = &srcData[(x1 + y1 * dataWidth) * 4];
						color[2] = (unsigned char)(((ctl[2] * ry1 + cbl[2] * ry0) * rx1 + (ctr[2] * ry1 + cbr[2] * ry0) * rx0));
						color[1] = (unsigned char)(((ctl[1] * ry1 + cbl[1] * ry0) * rx1 + (ctr[1] * ry1 + cbr[1] * ry0) * rx0));
						color[0] = (unsigned char)(((ctl[0] * ry1 + cbl[0] * ry0) * rx1 + (ctr[0] * ry1 + cbr[0] * ry0) * rx0));
						color[3] = (unsigned char)(((ctl[3] * ry1 + cbl[3] * ry0) * rx1 + (ctr[3] * ry1 + cbr[3] * ry0) * rx0));
						sc = color;
					}
					else if (rx0 != 0.0f)
					{
						ctl = &srcData[(x0 + y0 * dataWidth) * 4];
						ctr = &srcData[(x1 + y0 * dataWidth) * 4];
						color[2] = (unsigned char)((ctl[2] * rx1 + ctr[2] * rx0));
						color[1] = (unsigned char)((ctl[1] * rx1 + ctr[1] * rx0));
						color[0] = (unsigned char)((ctl[0] * rx1 + ctr[0] * rx0));
						color[3] = (unsigned char)((ctl[3] * rx1 + ctr[3] * rx0));
						sc = color;
					}
					else if (ry0 != 0.0f)
					{
						ctl = &srcData[(x0 + y0 * dataWidth) * 4];
						cbl = &srcData[(x0 + y1 * dataWidth) * 4];
						color[2] = (unsigned char)((ctl[2] * ry1 + cbl[2] * ry0));
						color[1] = (unsigned char)((ctl[1] * ry1 + cbl[1] * ry0));
						color[0] = (unsigned char)((ctl[0] * ry1 + cbl[0] * ry0));
						color[3] = (unsigned char)((ctl[3] * ry1 + cbl[3] * ry0));
						sc = color;
					}
					else
					{
						sc = &srcData[(x0 + y0 * dataWidth) * 4];
					}
					a0 = sc[3] * (int)alpha / 255;
					a1 = 255 - a0;
					c[2] = (unsigned char)((sc[2] * a0 + c[2] * a1) / 255);
					c[1] = (unsigned char)((sc[1] * a0 + c[1] * a1) / 255);
					c[0] = (unsigned char)((sc[0] * a0 + c[0] * a1) / 255);
				}
			}
		}
		else if (alpha < 255)
		{
			a0 = alpha;
			a1 = 255 - a0;
			for_iter (j, 0, h)
			{
				for_iterx (i, 0, w)
				{
					c = &thisData[(i + j * mWidth) * 4];
					cx = sx + i * fw;
					cy = sy + j * fh;
					x0 = (int)cx;
					y0 = (int)cy;
					x1 = hmin((int)cx + 1, dataWidth - 1);
					y1 = hmin((int)cy + 1, dataHeight - 1);
					rx0 = cx - x0;
					ry0 = cy - y0;
					rx1 = 1.0f - rx0;
					ry1 = 1.0f - ry0;
					if (rx0 != 0.0f || ry0 != 0.0f)
					{
						ctl = &srcData[(x0 + y0 * dataWidth) * 4];
						ctr = &srcData[(x1 + y0 * dataWidth) * 4];
						cbl = &srcData[(x0 + y1 * dataWidth) * 4];
						cbr = &srcData[(x1 + y1 * dataWidth) * 4];
						color[2] = (unsigned char)(((ctl[2] * ry1 + cbl[2] * ry0) * rx1 + (ctr[2] * ry1 + cbr[2] * ry0) * rx0));
						color[1] = (unsigned char)(((ctl[1] * ry1 + cbl[1] * ry0) * rx1 + (ctr[1] * ry1 + cbr[1] * ry0) * rx0));
						color[0] = (unsigned char)(((ctl[0] * ry1 + cbl[0] * ry0) * rx1 + (ctr[0] * ry1 + cbr[0] * ry0) * rx0));
						sc = color;
					}
					else if (rx0 != 0.0f)
					{
						ctl = &srcData[(x0 + y0 * dataWidth) * 4];
						ctr = &srcData[(x1 + y0 * dataWidth) * 4];
						color[2] = (unsigned char)((ctl[2] * rx1 + ctr[2] * rx0));
						color[1] = (unsigned char)((ctl[1] * rx1 + ctr[1] * rx0));
						color[0] = (unsigned char)((ctl[0] * rx1 + ctr[0] * rx0));
						sc = color;
					}
					else if (ry0 != 0.0f)
					{
						ctl = &srcData[(x0 + y0 * dataWidth) * 4];
						cbl = &srcData[(x0 + y1 * dataWidth) * 4];
						color[2] = (unsigned char)((ctl[2] * ry1 + cbl[2] * ry0));
						color[1] = (unsigned char)((ctl[1] * ry1 + cbl[1] * ry0));
						color[0] = (unsigned char)((ctl[0] * ry1 + cbl[0] * ry0));
						sc = color;
					}
					else
					{
						sc = &srcData[(x0 + y0 * dataWidth) * 4];
					}
					c[2] = (unsigned char)((sc[2] * a0 + c[2] * a1) / 255);
					c[1] = (unsigned char)((sc[1] * a0 + c[1] * a1) / 255);
					c[0] = (unsigned char)((sc[0] * a0 + c[0] * a1) / 255);
				}
			}
		}
		else
		{
			for_iter (j, 0, h)
			{
				for_iterx (i, 0, w)
				{
					c = &thisData[(i + j * mWidth) * 4];
					cx = sx + i * fw;
					cy = sy + j * fh;
					x0 = (int)cx;
					y0 = (int)cy;
					x1 = hmin((int)cx + 1, dataWidth - 1);
					y1 = hmin((int)cy + 1, dataHeight - 1);
					rx0 = cx - x0;
					ry0 = cy - y0;
					rx1 = 1.0f - rx0;
					ry1 = 1.0f - ry0;
					if (rx0 != 0.0f || ry0 != 0.0f)
					{
						ctl = &srcData[(x0 + y0 * dataWidth) * 4];
						ctr = &srcData[(x1 + y0 * dataWidth) * 4];
						cbl = &srcData[(x0 + y1 * dataWidth) * 4];
						cbr = &srcData[(x1 + y1 * dataWidth) * 4];
						color[2] = (unsigned char)(((ctl[2] * ry1 + cbl[2] * ry0) * rx1 + (ctr[2] * ry1 + cbr[2] * ry0) * rx0));
						color[1] = (unsigned char)(((ctl[1] * ry1 + cbl[1] * ry0) * rx1 + (ctr[1] * ry1 + cbr[1] * ry0) * rx0));
						color[0] = (unsigned char)(((ctl[0] * ry1 + cbl[0] * ry0) * rx1 + (ctr[0] * ry1 + cbr[0] * ry0) * rx0));
						sc = color;
					}
					else if (rx0 != 0.0f)
					{
						ctl = &srcData[(x0 + y0 * dataWidth) * 4];
						ctr = &srcData[(x1 + y0 * dataWidth) * 4];
						color[2] = (unsigned char)((ctl[2] * rx1 + ctr[2] * rx0));
						color[1] = (unsigned char)((ctl[1] * rx1 + ctr[1] * rx0));
						color[0] = (unsigned char)((ctl[0] * rx1 + ctr[0] * rx0));
						sc = color;
					}
					else if (ry0 != 0.0f)
					{
						ctl = &srcData[(x0 + y0 * dataWidth) * 4];
						cbl = &srcData[(x0 + y1 * dataWidth) * 4];
						color[2] = (unsigned char)((ctl[2] * ry1 + cbl[2] * ry0));
						color[1] = (unsigned char)((ctl[1] * ry1 + cbl[1] * ry0));
						color[0] = (unsigned char)((ctl[0] * ry1 + cbl[0] * ry0));
						sc = color;
					}
					else
					{
						sc = &srcData[(x0 + y0 * dataWidth) * 4];
					}
					c[2] = sc[2];
					c[1] = sc[1];
					c[0] = sc[0];
				}
			}
		}
		_unlock(buffer, result, true);
	}

	void DirectX9_Texture::clear()
	{
		D3DLOCKED_RECT lockRect;
		IDirect3DSurface9* buffer = NULL;
		LOCK_RESULT result = _tryLock(&buffer, &lockRect, NULL);
		if (result == LR_FAILED)
		{
			return;
		}
		memset(lockRect.pBits, 0, getWidth() * getHeight() * 4 * sizeof(unsigned char));
		_unlock(buffer, result, true);
	}

	void rgb2hsl(unsigned char r, unsigned char g, unsigned char b, float* h, float* s, float* l)
	{
		int min = hmin(hmin(r, g), b);
		int max = hmax(hmax(r, g), b);
		int delta = max - min;
		*l = (max + min) / 510.0f;
		*s = 0.0f;
		if (*l > 0.0f && *l < 1.0f)
		{
			*s = (delta / 255.0f) / (*l < 0.5f ? (2 * *l) : (2 - 2 * *l));
		}
		*h = 0.0f;
		if (delta > 0)
		{
			if (max == r)
			{
				*h += (g - b) / (float)delta;
			}
			if (max == g)
			{
				*h += 2 + (b - r) / (float)delta;
			}
			if (max == b)
			{
				*h += 4 + (r - g) / (float)delta;
			}
			*h /= 6;
		}
    }

	float _color_hue2rgb(float m1, float m2, float h)
	{ 
		h = (h < 0) ? h + 1 : ((h > 1) ? h - 1 : h);
		if (h * 6 < 1)
		{
			return m1 + (m2 - m1) * h * 6;
		}
		if (h * 2 < 1)
		{
			return m2;
		}
		if (h * 3 < 2)
		{
			return m1 + (m2 - m1) * (0.6666667f - h) * 6;
		}
		return m1;
	}

	void hsl2rgb(float h, float s, float l, unsigned char* r, unsigned char* g, unsigned char* b)
	{
		float m2 = (l <= 0.5f) ? l * (s + 1) : l + s - l * s;
		float m1 = l * 2 - m2;
		*r = (unsigned char)hroundf(255.0f * _color_hue2rgb(m1, m2, h + 0.3333333f));
		*g = (unsigned char)hroundf(255.0f * _color_hue2rgb(m1, m2, h));
		*b = (unsigned char)hroundf(255.0f * _color_hue2rgb(m1, m2, h - 0.3333333f));
	}

	void DirectX9_Texture::rotateHue(float degrees)
	{
		if (degrees == 0.0f)
		{
			return;
		}
		D3DLOCKED_RECT lockRect;
		IDirect3DSurface9* buffer = NULL;
		LOCK_RESULT result = _tryLock(&buffer, &lockRect, NULL);
		if (result == LR_FAILED)
		{
			return;
		}
		int size = getWidth() * getHeight() * 4;
		float range = hmodf(degrees, 360.0f) / 360.0f;
		float h;
		float s;
		float l;
		unsigned char* data = (unsigned char*)lockRect.pBits;
		for_iter_step(i, 0, size, mBpp)
		{
			rgb2hsl(data[i + 2], data[i + 1], data[i], &h, &s, &l);
			h += range;
			if (h > 1.0f)
			{
				h -= 1.0f;
			}
			hsl2rgb(h, s, l, &data[i + 2], &data[i + 1], &data[i]);
		}
		_unlock(buffer, result, true);
	}

	void DirectX9_Texture::saturate(float factor)
	{
		D3DLOCKED_RECT lockRect;
		IDirect3DSurface9* buffer = NULL;
		LOCK_RESULT result = _tryLock(&buffer, &lockRect, NULL);
		if (result == LR_FAILED)
		{
			return;
		}
		int size = getWidth() * getHeight() * 4;
		float h;
		float s;
		float l;
		unsigned char* data = (unsigned char*)lockRect.pBits;
		for_iter_step(i, 0, size, mBpp)
		{
			rgb2hsl(data[i + 2], data[i + 1], data[i], &h, &s, &l);
			s *= factor;
			hsl2rgb(h, s, l, &data[i + 2], &data[i + 1], &data[i]);
		}
		_unlock(buffer, result, true);
	}

	IDirect3DSurface9* DirectX9_Texture::getSurface()
	{
		if (mSurface == NULL)
		{
			mTexture->GetSurfaceLevel(0, &mSurface);
		}
		return mSurface;
	}

	bool DirectX9_Texture::load()
	{
		mUnusedTimer = 0;
		if (mTexture)
		{
			return true;
		}
		april::log("loading DX9 texture '" + _getInternalName() + "'");
		ImageSource* img = loadImage(mFilename);
		if (!img)
		{
			april::log("Failed to load texture '" + _getInternalName() + "'!");
			return false;
		}
		mWidth = img->w;
		mHeight = img->h;
		mBpp = (img->bpp == 3 ? 3 : 4);
		HRESULT hr = d3dDevice->CreateTexture(mWidth, mHeight, 1, 0, (img->bpp == 3) ? D3DFMT_X8R8G8B8 : D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &mTexture, NULL);
		if (hr != D3D_OK)
		{
			april::log("Failed to load DX9 texture!");
			delete img;
			return false;
		}
		// write texels
		D3DLOCKED_RECT rect;
		mTexture->LockRect(0, &rect, NULL, D3DLOCK_DISCARD);
		if (img->bpp == 4)
		{
			img->copyPixels(rect.pBits, AF_BGRA);
		}
		else if (img->bpp == 3)
		{
			img->copyPixels(rect.pBits, AF_BGR);
		}
		else
		{
			ImageSource* tempImg = april::createEmptyImage(img->w, img->h);
			tempImg->copyImage(img, 4);
			tempImg->copyPixels(rect.pBits, AF_BGRA);
			delete tempImg;
		}
		mTexture->UnlockRect(0);
		delete img;
		foreach (Texture*, it, mDynamicLinks)
		{
			if (!(*it)->isLoaded())
			{
				(*it)->load();
			}
		}
		return true;
	}

	bool DirectX9_Texture::isLoaded()
	{
		return (mTexture != NULL);
	}

	void DirectX9_Texture::unload()
	{
		if (mTexture != NULL)
		{
			april::log("unloading DX9 texture '" + _getInternalName() + "'");
			mTexture->Release();
			mTexture = NULL;
			if (mSurface != NULL)
			{
				mSurface->Release();
				mSurface = NULL;
			}
		}
	}

	int DirectX9_Texture::getSizeInBytes()
	{
		return (mWidth * mHeight * mBpp);
	}

	bool DirectX9_Texture::isValid()
	{
		return (mTexture != NULL);
	}

	void DirectX9_Texture::insertAsAlphaMap(Texture* texture, unsigned char median, int ambiguity)
	{
		if (mWidth != texture->getWidth() || mHeight != texture->getHeight() || mBpp != 4)
		{
			return;
		}
		DirectX9_Texture* source = (DirectX9_Texture*)texture;
		D3DLOCKED_RECT lockRect;
		IDirect3DSurface9* buffer = NULL;
		LOCK_RESULT result = _tryLock(&buffer, &lockRect, NULL);
		if (result == LR_FAILED)
		{
			return;
		}
		D3DLOCKED_RECT srcLockRect;
		IDirect3DSurface9* srcBuffer = NULL;
		LOCK_RESULT srcResult = source->_tryLock(&srcBuffer, &srcLockRect, NULL);
		if (srcResult == LR_FAILED)
		{
			_unlock(buffer, result, false);
			return;
		}
		unsigned char* thisData = (unsigned char*)lockRect.pBits;
		unsigned char* srcData = (unsigned char*)srcLockRect.pBits;
		unsigned char* c;
		unsigned char* sc;
		int i;
		int alpha;
		int min = (int)median - ambiguity / 2;
		int max = (int)median + ambiguity / 2;
		for_iter (j, 0, mHeight)
		{
			for_iterx (i, 0, mWidth)
			{
				c = &thisData[(i + j * mWidth) * 4];
				sc = &srcData[(i + j * mWidth) * 4];
				alpha = (sc[0] + sc[1] + sc[2]) / 3;
				if (alpha < min)
				{
					c[3] = 255;
				}
				else if (alpha >= max)
				{
					c[3] = 0;
				}
				else
				{
					c[3] = (max - alpha) * 255 / ambiguity;
				}
			}
		}
		source->_unlock(srcBuffer, srcResult, false);
		_unlock(buffer, result, true);
	}

	DirectX9_Texture::LOCK_RESULT DirectX9_Texture::_tryLock(IDirect3DSurface9** buffer, D3DLOCKED_RECT* lockRect, RECT* rect)
	{
		HRESULT result = mTexture->LockRect(0, lockRect, rect, D3DLOCK_DISCARD);
		if (result == D3D_OK)
		{
			return LR_LOCKED;
		}
		// could be a render target
		result = d3dDevice->CreateOffscreenPlainSurface(mWidth, mHeight,
			(mBpp == 4 ? D3DFMT_A8R8G8B8 : D3DFMT_X8R8G8B8), D3DPOOL_SYSTEMMEM, buffer, NULL);
		if (result != D3D_OK)
		{
			april::log("failed to get pixel data, CreateOffscreenPlainSurface() call failed");
			return LR_FAILED;
		}
		result = d3dDevice->GetRenderTargetData(getSurface(), *buffer);
		if (result != D3D_OK)
		{
			april::log("failed to get pixel data, GetRenderTargetData() call failed");
			return LR_FAILED;
		}
		result = (*buffer)->LockRect(lockRect, rect, D3DLOCK_DISCARD);
		if (result != D3D_OK)
		{
			april::log("failed to get pixel data, surface lock failed");
			return LR_FAILED;
		}
		return LR_RENDERTARGET;
	}

	void DirectX9_Texture::_unlock(IDirect3DSurface9* buffer, DirectX9_Texture::LOCK_RESULT lock, bool update)
	{
		switch (lock)
		{
		case LR_LOCKED:
			mTexture->UnlockRect(0);
			break;
		case LR_RENDERTARGET:
			buffer->UnlockRect();
			if (update)
			{
				d3dDevice->UpdateSurface(buffer, NULL, getSurface(), NULL);
			}
			buffer->Release();
			break;
		}
	}

}

#endif
