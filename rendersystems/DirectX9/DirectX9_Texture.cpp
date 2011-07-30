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
		mBpp = 4;
		mDynamic = false;
		mFilename = "UserTexture";
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
	
	Color DirectX9_Texture::getPixel(int x, int y)
	{
		Color result;
		D3DLOCKED_RECT lockRect;
		RECT rect;
		rect.left = x;
		rect.right = x;
		rect.top = y;
		rect.bottom = y;
		mTexture->LockRect(0, &lockRect, &rect, (D3DLOCK_DISCARD | D3DLOCK_READONLY | D3DLOCK_NO_DIRTY_UPDATE));
		unsigned char* p = (unsigned char*)lockRect.pBits;
		result.r = p[2];
		result.g = p[1];
		result.b = p[0];
		result.a = p[3];
		mTexture->UnlockRect(0);
		return result;
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
		HRESULT result = mTexture->LockRect(0, &lockRect, &rect, D3DLOCK_DISCARD);
		if (result == D3D_OK)
		{
			unsigned char* p = (unsigned char*)lockRect.pBits;
			p[2] = color.r;
			p[1] = color.g;
			p[0] = color.b;
			p[3] = (mBpp == 4 ? color.a : 255);
			mTexture->UnlockRect(0);
		}
		else
		{
			// TODO - throw error here?
		}
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
		HRESULT result = mTexture->LockRect(0, &lockRect, &rect, D3DLOCK_DISCARD);
		if (result == D3D_OK)
		{
			unsigned char* p = (unsigned char*)lockRect.pBits;
			int i;
			int offset;
			if (mBpp == 4)
			{
				for (int j = 0; j < h; j++)
				{
					for (i = 0; i < w; i++)
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
				for (int j = 0; j < h; j++)
				{
					for (i = 0; i < w; i++)
					{
						offset = (i + j * mWidth) * 4;
						p[offset + 2] = color.r;
						p[offset + 1] = color.g;
						p[offset + 0] = color.b;
					}
				}
			}
			mTexture->UnlockRect(0);
		}
		else
		{
			// TODO - throw error here?
		}
	}

	void DirectX9_Texture::blit(int x, int y, Texture* texture, int sx, int sy, int sw, int sh, unsigned char alpha)
	{
		DirectX9_Texture* other = (DirectX9_Texture*)texture;
		x = hclamp(x, 0, mWidth - 1);
		y = hclamp(y, 0, mHeight - 1);
		sx = hclamp(sx, 0, other->mWidth - 1);
		sy = hclamp(sy, 0, other->mHeight - 1);
		sw = hmin(sw, hmin(mWidth - x, other->mWidth - sx));
		sh = hmin(sh, hmin(mHeight - y, other->mHeight - sy));
		if (sw == 1 && sh == 1)
		{
			this->setPixel(x, y, other->getPixel(sx, sy));
			return;
		}
		D3DLOCKED_RECT lockRect;
		RECT rect;
		rect.left = sx;
		rect.right = sx + sw - 1;
		rect.top = sy;
		rect.bottom = sy + sh - 1;
		HRESULT result = other->getTexture()->LockRect(0, &lockRect, &rect, (D3DLOCK_DISCARD | D3DLOCK_READONLY | D3DLOCK_NO_DIRTY_UPDATE));
		if (result == D3D_OK)
		{
			blit(x, y, (unsigned char*)lockRect.pBits, other->mWidth, other->mHeight, other->mBpp, sx, sy, sw, sh, alpha);
			other->getTexture()->UnlockRect(0);
		}
		mTexture->UnlockRect(0);
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
		HRESULT result = mTexture->LockRect(0, &lockRect, &rect, D3DLOCK_DISCARD);
		if (result == D3D_OK)
		{
			unsigned char* thisData = (unsigned char*)lockRect.pBits;
			unsigned char* otherData = data;
			unsigned char* c;
			unsigned char* sc;
			unsigned char a;
			int i;
			// the following iteration blocks are very similar, but for performance reasons they
			// have been duplicated instead of putting everything into one block with if branches
			if (mBpp == 4 && dataBpp == 4)
			{
				for (int j = 0; j < sh; j++)
				{
					for (i = 0; i < sw; i++)
					{
						c = &thisData[(i + j * mWidth) * 4];
						sc = &otherData[(i + j * dataWidth) * 4];
						if (c[3] > 0)
						{
							a = sc[3] * alpha / 255;
							c[2] = (sc[2] * a + (255 - a) * c[2]) / 255;
							c[1] = (sc[1] * a + (255 - a) * c[1]) / 255;
							c[0] = (sc[0] * a + (255 - a) * c[0]) / 255;
							c[3] = hmax(c[3], a);
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
				for (int j = 0; j < sh; j++)
				{
					for (i = 0; i < sw; i++)
					{
						c = &thisData[(i + j * mWidth) * 4];
						sc = &otherData[(i + j * dataWidth) * 4];
						a = sc[3] * alpha / 255;
						c[2] = (sc[2] * a + (255 - a) * c[2]) / 255;
						c[1] = (sc[1] * a + (255 - a) * c[1]) / 255;
						c[0] = (sc[0] * a + (255 - a) * c[0]) / 255;
					}
				}
			}
			else if (alpha < 255)
			{
				a = alpha;
				for (int j = 0; j < sh; j++)
				{
					for (i = 0; i < sw; i++)
					{
						c = &thisData[(i + j * mWidth) * 4];
						sc = &otherData[(i + j * dataWidth) * 4];
						c[2] = (sc[2] * a + (255 - a) * c[2]) / 255;
						c[1] = (sc[1] * a + (255 - a) * c[1]) / 255;
						c[0] = (sc[0] * a + (255 - a) * c[0]) / 255;
					}
				}
			}
			else
			{
				for (int j = 0; j < sh; j++)
				{
					for (i = 0; i < sw; i++)
					{
						c = &thisData[(i + j * mWidth) * 4];
						sc = &otherData[(i + j * dataWidth) * 4];
						c[2] = sc[2];
						c[1] = sc[1];
						c[0] = sc[0];
					}
				}
			}
			mTexture->UnlockRect(0);
		}
	}

	void DirectX9_Texture::stretchBlit(int x, int y, int w, int h, Texture* texture, int sx, int sy, int sw, int sh, unsigned char alpha)
	{
		DirectX9_Texture* other = (DirectX9_Texture*)texture;
		x = hclamp(x, 0, this->mWidth - 1);
		y = hclamp(y, 0, this->mHeight - 1);
		w = hmin(w, this->mWidth - x);
		h = hmin(h, this->mHeight - y);
		sx = hclamp(sx, 0, other->mWidth - 1);
		sy = hclamp(sy, 0, other->mHeight - 1);
		sw = hmin(sw, other->mWidth - sx);
		sh = hmin(sh, other->mHeight - sy);
		D3DLOCKED_RECT lockRect;
		HRESULT result = other->getTexture()->LockRect(0, &lockRect, NULL, D3DLOCK_DISCARD);
		if (result == D3D_OK)
		{
			stretchBlit(x, y, w, h, (unsigned char*)lockRect.pBits, other->mWidth, other->mHeight, other->mBpp, sx, sy, sw, sh, alpha);
			other->getTexture()->UnlockRect(0);
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
		HRESULT result = mTexture->LockRect(0, &lockRect, &rect, D3DLOCK_DISCARD);
		if (result == D3D_OK)
		{
			unsigned char* thisData = (unsigned char*)lockRect.pBits;
			unsigned char* otherData = data;
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
				for (int j = 0; j < h; j++)
				{
					for (i = 0; i < w; i++)
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
							ctl = &otherData[(x0 + y0 * dataWidth) * 4];
							ctr = &otherData[(x1 + y0 * dataWidth) * 4];
							cbl = &otherData[(x0 + y1 * dataWidth) * 4];
							cbr = &otherData[(x1 + y1 * dataWidth) * 4];
							color[2] = (unsigned char)(((ctl[2] * ry1 + cbl[2] * ry0) * rx1 + (ctr[2] * ry1 + cbr[2] * ry0) * rx0));
							color[1] = (unsigned char)(((ctl[1] * ry1 + cbl[1] * ry0) * rx1 + (ctr[1] * ry1 + cbr[1] * ry0) * rx0));
							color[0] = (unsigned char)(((ctl[0] * ry1 + cbl[0] * ry0) * rx1 + (ctr[0] * ry1 + cbr[0] * ry0) * rx0));
							color[3] = (unsigned char)(((ctl[3] * ry1 + cbl[3] * ry0) * rx1 + (ctr[3] * ry1 + cbr[3] * ry0) * rx0));
							sc = color;
						}
						else if (rx0 != 0.0f)
						{
							ctl = &otherData[(x0 + y0 * dataWidth) * 4];
							ctr = &otherData[(x1 + y0 * dataWidth) * 4];
							color[2] = (unsigned char)((ctl[2] * rx1 + ctr[2] * rx0));
							color[1] = (unsigned char)((ctl[1] * rx1 + ctr[1] * rx0));
							color[0] = (unsigned char)((ctl[0] * rx1 + ctr[0] * rx0));
							color[3] = (unsigned char)((ctl[3] * rx1 + ctr[3] * rx0));
							sc = color;
						}
						else if (ry0 != 0.0f)
						{
							ctl = &otherData[(x0 + y0 * dataWidth) * 4];
							cbl = &otherData[(x0 + y1 * dataWidth) * 4];
							color[2] = (unsigned char)((ctl[2] * ry1 + cbl[2] * ry0));
							color[1] = (unsigned char)((ctl[1] * ry1 + cbl[1] * ry0));
							color[0] = (unsigned char)((ctl[0] * ry1 + cbl[0] * ry0));
							color[3] = (unsigned char)((ctl[3] * ry1 + cbl[3] * ry0));
							sc = color;
						}
						else
						{
							sc = &otherData[(x0 + y0 * dataWidth) * 4];
						}
						if (c[3] > 0)
						{
							a0 = sc[3] * (int)alpha / 255;
							a1 = 255 - a0;
							c[2] = (unsigned char)((sc[2] * a0 + c[2] * a1) / 255);
							c[1] = (unsigned char)((sc[1] * a0 + c[1] * a1) / 255);
							c[0] = (unsigned char)((sc[0] * a0 + c[0] * a1) / 255);
							c[3] = (unsigned char)hmax((int)c[3], a0);
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
				for (int j = 0; j < h; j++)
				{
					for (i = 0; i < w; i++)
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
							ctl = &otherData[(x0 + y0 * dataWidth) * 4];
							ctr = &otherData[(x1 + y0 * dataWidth) * 4];
							cbl = &otherData[(x0 + y1 * dataWidth) * 4];
							cbr = &otherData[(x1 + y1 * dataWidth) * 4];
							color[2] = (unsigned char)(((ctl[2] * ry1 + cbl[2] * ry0) * rx1 + (ctr[2] * ry1 + cbr[2] * ry0) * rx0));
							color[1] = (unsigned char)(((ctl[1] * ry1 + cbl[1] * ry0) * rx1 + (ctr[1] * ry1 + cbr[1] * ry0) * rx0));
							color[0] = (unsigned char)(((ctl[0] * ry1 + cbl[0] * ry0) * rx1 + (ctr[0] * ry1 + cbr[0] * ry0) * rx0));
							color[3] = (unsigned char)(((ctl[3] * ry1 + cbl[3] * ry0) * rx1 + (ctr[3] * ry1 + cbr[3] * ry0) * rx0));
							sc = color;
						}
						else if (rx0 != 0.0f)
						{
							ctl = &otherData[(x0 + y0 * dataWidth) * 4];
							ctr = &otherData[(x1 + y0 * dataWidth) * 4];
							color[2] = (unsigned char)((ctl[2] * rx1 + ctr[2] * rx0));
							color[1] = (unsigned char)((ctl[1] * rx1 + ctr[1] * rx0));
							color[0] = (unsigned char)((ctl[0] * rx1 + ctr[0] * rx0));
							color[3] = (unsigned char)((ctl[3] * rx1 + ctr[3] * rx0));
							sc = color;
						}
						else if (ry0 != 0.0f)
						{
							ctl = &otherData[(x0 + y0 * dataWidth) * 4];
							cbl = &otherData[(x0 + y1 * dataWidth) * 4];
							color[2] = (unsigned char)((ctl[2] * ry1 + cbl[2] * ry0));
							color[1] = (unsigned char)((ctl[1] * ry1 + cbl[1] * ry0));
							color[0] = (unsigned char)((ctl[0] * ry1 + cbl[0] * ry0));
							color[3] = (unsigned char)((ctl[3] * ry1 + cbl[3] * ry0));
							sc = color;
						}
						else
						{
							sc = &otherData[(x0 + y0 * dataWidth) * 4];
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
				for (int j = 0; j < h; j++)
				{
					for (i = 0; i < w; i++)
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
							ctl = &otherData[(x0 + y0 * dataWidth) * 4];
							ctr = &otherData[(x1 + y0 * dataWidth) * 4];
							cbl = &otherData[(x0 + y1 * dataWidth) * 4];
							cbr = &otherData[(x1 + y1 * dataWidth) * 4];
							color[2] = (unsigned char)(((ctl[2] * ry1 + cbl[2] * ry0) * rx1 + (ctr[2] * ry1 + cbr[2] * ry0) * rx0));
							color[1] = (unsigned char)(((ctl[1] * ry1 + cbl[1] * ry0) * rx1 + (ctr[1] * ry1 + cbr[1] * ry0) * rx0));
							color[0] = (unsigned char)(((ctl[0] * ry1 + cbl[0] * ry0) * rx1 + (ctr[0] * ry1 + cbr[0] * ry0) * rx0));
							sc = color;
						}
						else if (rx0 != 0.0f)
						{
							ctl = &otherData[(x0 + y0 * dataWidth) * 4];
							ctr = &otherData[(x1 + y0 * dataWidth) * 4];
							color[2] = (unsigned char)((ctl[2] * rx1 + ctr[2] * rx0));
							color[1] = (unsigned char)((ctl[1] * rx1 + ctr[1] * rx0));
							color[0] = (unsigned char)((ctl[0] * rx1 + ctr[0] * rx0));
							sc = color;
						}
						else if (ry0 != 0.0f)
						{
							ctl = &otherData[(x0 + y0 * dataWidth) * 4];
							cbl = &otherData[(x0 + y1 * dataWidth) * 4];
							color[2] = (unsigned char)((ctl[2] * ry1 + cbl[2] * ry0));
							color[1] = (unsigned char)((ctl[1] * ry1 + cbl[1] * ry0));
							color[0] = (unsigned char)((ctl[0] * ry1 + cbl[0] * ry0));
							sc = color;
						}
						else
						{
							sc = &otherData[(x0 + y0 * dataWidth) * 4];
						}
						c[2] = (unsigned char)((sc[2] * a0 + c[2] * a1) / 255);
						c[1] = (unsigned char)((sc[1] * a0 + c[1] * a1) / 255);
						c[0] = (unsigned char)((sc[0] * a0 + c[0] * a1) / 255);
					}
				}
			}
			else
			{
				for (int j = 0; j < h; j++)
				{
					for (i = 0; i < w; i++)
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
							ctl = &otherData[(x0 + y0 * dataWidth) * 4];
							ctr = &otherData[(x1 + y0 * dataWidth) * 4];
							cbl = &otherData[(x0 + y1 * dataWidth) * 4];
							cbr = &otherData[(x1 + y1 * dataWidth) * 4];
							color[2] = (unsigned char)(((ctl[2] * ry1 + cbl[2] * ry0) * rx1 + (ctr[2] * ry1 + cbr[2] * ry0) * rx0));
							color[1] = (unsigned char)(((ctl[1] * ry1 + cbl[1] * ry0) * rx1 + (ctr[1] * ry1 + cbr[1] * ry0) * rx0));
							color[0] = (unsigned char)(((ctl[0] * ry1 + cbl[0] * ry0) * rx1 + (ctr[0] * ry1 + cbr[0] * ry0) * rx0));
							sc = color;
						}
						else if (rx0 != 0.0f)
						{
							ctl = &otherData[(x0 + y0 * dataWidth) * 4];
							ctr = &otherData[(x1 + y0 * dataWidth) * 4];
							color[2] = (unsigned char)((ctl[2] * rx1 + ctr[2] * rx0));
							color[1] = (unsigned char)((ctl[1] * rx1 + ctr[1] * rx0));
							color[0] = (unsigned char)((ctl[0] * rx1 + ctr[0] * rx0));
							sc = color;
						}
						else if (ry0 != 0.0f)
						{
							ctl = &otherData[(x0 + y0 * dataWidth) * 4];
							cbl = &otherData[(x0 + y1 * dataWidth) * 4];
							color[2] = (unsigned char)((ctl[2] * ry1 + cbl[2] * ry0));
							color[1] = (unsigned char)((ctl[1] * ry1 + cbl[1] * ry0));
							color[0] = (unsigned char)((ctl[0] * ry1 + cbl[0] * ry0));
							sc = color;
						}
						else
						{
							sc = &otherData[(x0 + y0 * dataWidth) * 4];
						}
						c[2] = sc[2];
						c[1] = sc[1];
						c[0] = sc[0];
					}
				}
			}
			mTexture->UnlockRect(0);
		}
	}

	void DirectX9_Texture::clear()
	{
		D3DLOCKED_RECT lockRect;
		HRESULT result = mTexture->LockRect(0, &lockRect, NULL, D3DLOCK_DISCARD);
		if (result == D3D_OK)
		{
			memset(lockRect.pBits, 0, getWidth() * getHeight() * 4 * sizeof(unsigned char));
			mTexture->UnlockRect(0);
		}
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
		D3DLOCKED_RECT lockRect;
		HRESULT result = mTexture->LockRect(0, &lockRect, NULL, D3DLOCK_DISCARD);
		if (result == D3D_OK)
		{
			int size = getWidth() * getHeight() * 4;
			float range = (degrees >= 0.0f ? fmod(degrees, 360.0f) : 360.0f - fmod(-degrees, 360.0f)) / 360.0f;
			float h;
			float s;
			float l;

			unsigned char* data = (unsigned char*)lockRect.pBits;
			for (int i = 0; i < size; i += mBpp)
			{
				rgb2hsl(data[i + 2], data[i + 1], data[i], &h, &s, &l);
				h += range;
				if (h > 1.0f)
				{
					h -= 1.0f;
				}
				hsl2rgb(h, s, l, &data[i + 2], &data[i + 1], &data[i]);
			}
			mTexture->UnlockRect(0);
		}
	}

	IDirect3DSurface9* DirectX9_Texture::getSurface()
	{
		if (mSurface == NULL)
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
		if (mTexture != NULL)
		{
			april::log("unloading DX9 texture '" + mFilename + "'");
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
}

#endif
