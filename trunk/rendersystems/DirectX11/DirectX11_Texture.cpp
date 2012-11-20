/// @file
/// @author  Boris Mikic
/// @version 2.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#ifdef _DIRECTX11
#include <d3d9.h>
#ifdef USE_IL
#include <IL/il.h>
#endif

#include <hltypes/hlog.h>

#include "april.h"
#include "DirectX11_RenderSystem.h"
#include "DirectX11_Texture.h"
#include "ImageSource.h"

#define APRIL_D3D_DEVICE (((DirectX11_RenderSystem*)april::rendersys)->d3dDevice)
#define _CREATE_RECT(name, x, y, w, h) \
	RECT name; \
	name.left = x; \
	name.top = y; \
	name.right = x + w - 1; \
	name.bottom = y + h - 1;

namespace april
{
	// TODO - refactor
	extern harray<DirectX11_Texture*> gRenderTargets;

	DirectX11_Texture::DirectX11_Texture(chstr filename) : Texture()
	{
		this->filename = filename;
		this->format = FORMAT_ARGB;
		this->width = 0;
		this->height = 0;
		this->bpp = 4;
		this->renderTarget = false;
		this->d3dTexture = nullptr;
		this->d3dView = nullptr;
		this->d3dSampler = nullptr;
		hlog::write(april::logTag, "Creating DX11 texture: " + this->_getInternalName());
	}

	DirectX11_Texture::DirectX11_Texture(int w, int h, unsigned char* rgba) : Texture()
	{
		this->filename = "";
		this->format = FORMAT_ARGB;
		this->width = w;
		this->height = h;
		this->bpp = 4;
		this->renderTarget = false;
		this->d3dTexture = nullptr;
		this->d3dView = nullptr;
		this->d3dSampler = nullptr;
		hlog::write(april::logTag, "Creating user-defined DX11 texture.");
		unsigned char* bgra = new unsigned char[this->width * this->height * this->bpp];
		memcpy(bgra, rgba, this->width * this->height * this->bpp); // so alpha doesn't have to be copied in each iteration
		int offset;
		int i;
		int j;
		for_iterx (j, 0, this->height)
		{
			for_iterx (i, 0, this->width)
			{
				offset = (i + j * this->width) * this->bpp;
				bgra[offset + 2] = rgba[offset + 0];
				//bgra[offset + 1] = rgba[offset + 1]; // not necessary to be executed because of previous memcpy call
				bgra[offset + 0] = rgba[offset + 2];
			}
		}
		this->_createInternalTexture(bgra);
		delete [] bgra;
	}
	
	DirectX11_Texture::DirectX11_Texture(int w, int h, Texture::Format format, Texture::Type type, Color color) : Texture()
	{
		this->filename = "";
		this->format = format;
		this->width = w;
		this->height = h;
		this->renderTarget = false;
		this->d3dTexture = nullptr;
		this->d3dView = nullptr;
		this->d3dSampler = nullptr;
		hlog::writef(april::logTag, "Creating empty DX11 texture [ %dx%d ].", w, h);
		this->bpp = 4;
		if (type == TYPE_RENDER_TARGET)
		{
			this->renderTarget = true;
			gRenderTargets += this;
		}
		unsigned char* data = new unsigned char[this->width * this->height * this->bpp];
		memset(data, 0, this->width * this->height * this->bpp);
		this->_createInternalTexture(data);
		delete [] data;
		if (color != Color::Clear)
		{
			this->fillRect(0, 0, this->width, this->height, color);
		}
	}

	bool DirectX11_Texture::_createInternalTexture(unsigned char* data)
	{
		// texture
		D3D11_SUBRESOURCE_DATA textureSubresourceData = {0};
		textureSubresourceData.pSysMem = data;
		textureSubresourceData.SysMemPitch = this->width * this->bpp;
		textureSubresourceData.SysMemSlicePitch = 0;
		D3D11_TEXTURE2D_DESC textureDesc = {0};
		textureDesc.Width = this->width;
		textureDesc.Height = this->height;
		textureDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.CPUAccessFlags = 0;
		textureDesc.MiscFlags = 0;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.SampleDesc.Quality = 0;
		textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		switch (this->format)
		{
		case FORMAT_ARGB:
			textureDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
			this->bpp = 4;
			break;
		case FORMAT_RGB:
			textureDesc.Format = DXGI_FORMAT_B8G8R8X8_UNORM;
			this->bpp = 4;
			break;
		case FORMAT_ALPHA:
			textureDesc.Format = DXGI_FORMAT_A8_UNORM;
			this->bpp = 1;
			break;
		default:
			textureDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
			this->bpp = 4;
			break;
		}
		textureSubresourceData.SysMemPitch = this->width * this->bpp;
		if (this->renderTarget) // TODO - may not even be necessary
		{
			textureDesc.Usage = D3D11_USAGE_DEFAULT;
		}
		HRESULT hr = APRIL_D3D_DEVICE->CreateTexture2D(&textureDesc, &textureSubresourceData, &this->d3dTexture);
		if (FAILED(hr))
		{
			hlog::error(april::logTag, "Failed to create DX11 texture!");
			return false;
		}
		// shader resource
        D3D11_SHADER_RESOURCE_VIEW_DESC textureViewDesc;
		memset(&textureViewDesc, 0, sizeof(textureViewDesc));
        textureViewDesc.Format = textureDesc.Format;
        textureViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        textureViewDesc.Texture2D.MipLevels = textureDesc.MipLevels;
        textureViewDesc.Texture2D.MostDetailedMip = 0;
        hr = APRIL_D3D_DEVICE->CreateShaderResourceView(this->d3dTexture.Get(), &textureViewDesc, &this->d3dView);
		if (FAILED(hr))
		{
			hlog::error(april::logTag, "Failed to create DX11 texture view!");
			return false;
		}
		// sampler
		D3D11_SAMPLER_DESC samplerDesc;
		memset(&samplerDesc, 0, sizeof(samplerDesc));
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc.MaxAnisotropy = 0;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.MipLODBias = 0.0f;
		samplerDesc.MinLOD = 0;
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		samplerDesc.BorderColor[0] = 0.0f;
		samplerDesc.BorderColor[1] = 0.0f;
		samplerDesc.BorderColor[2] = 0.0f;
		samplerDesc.BorderColor[3] = 0.0f;
		hr = APRIL_D3D_DEVICE->CreateSamplerState(&samplerDesc, &this->d3dSampler);
		if (FAILED(hr))
		{
			hlog::error(april::logTag, "Failed to create DX11 texture sample!");
			return false;
		}
		return true;
	}
	
	void DirectX11_Texture::restore()
	{
		// TODO
		/*
		if (!this->renderTarget)
		{
			return;
		}
		this->unload();
		D3DFORMAT d3dformat = D3DFMT_X8R8G8B8;
		if (this->bpp == 4)
		{
			d3dformat = D3DFMT_A8R8G8B8;
		}
		D3DPOOL d3dpool = D3DPOOL_DEFAULT;
		DWORD d3dusage = D3DUSAGE_RENDERTARGET;
		HRESULT hr = APRIL_D3D_DEVICE->CreateTexture(this->width, this->height, 1, d3dusage, d3dformat, d3dpool, &this->d3dTexture, NULL);
		if (hr != D3D_OK)
		{
			hlog::error(april::logTag, "Failed to restore user-defined DX11 texture!");
			return;
		}
		*/
	}

	DirectX11_Texture::~DirectX11_Texture()
	{
		this->unload();
		if (this->renderTarget)
		{
			gRenderTargets -= this;
		}
	}

	bool DirectX11_Texture::load()
	{
		if (this->isLoaded())
		{
			return true;
		}
		hlog::write(april::logTag, "Loading DX11 texture: " + this->_getInternalName());
		ImageSource* image = NULL;
		if (this->filename != "")
		{
			image = april::loadImage(this->filename);
			if (image == NULL)
			{
				hlog::error(april::logTag, "Failed to load texture: " + this->_getInternalName());
				return false;
			}
			this->width = image->w;
			this->height = image->h;
			this->bpp = image->bpp;
		}
		if (image == NULL)
		{
			hlog::error(april::logTag, "Image source does not exist!");
			return false;
		}
		switch (image->format)
		{
		case AF_RGBA:
		case AF_BGRA:
			this->format = FORMAT_ARGB;
			break;
		case AF_RGB:
		case AF_BGR:
			this->format = FORMAT_ARGB;
			break;
		case AF_GRAYSCALE:
			this->format = FORMAT_ALPHA;
			break;
		case AF_PALETTE:
			this->format = FORMAT_ARGB; // TODO - should be changed
			break;
		default:
			this->format = FORMAT_ARGB;
			break;
		}
		if (image->format == AF_RGBA || image->format == AF_RGB)
		{
			unsigned char* data = new unsigned char[image->w * image->h * 4];
			if (image->format == AF_RGBA)
			{
				image->copyPixels(data, AF_BGRA);
			}
			else
			{
				image->copyPixels(data, AF_BGR);
			}
			this->bpp = image->bpp = 4;
			delete image->data;
			image->data = data;
		}
		bool result = this->_createInternalTexture(image->data);
		delete image;
		return result;
	}

	void DirectX11_Texture::unload()
	{
		if (this->d3dTexture != nullptr)
		{
			hlog::write(april::logTag, "Unloading DX11 texture: " + this->_getInternalName());
			_HL_TRY_RELEASE_COMPTR(this->d3dTexture);
			_HL_TRY_RELEASE_COMPTR(this->d3dView);
			_HL_TRY_RELEASE_COMPTR(this->d3dSampler);
		}
	}

	bool DirectX11_Texture::isLoaded()
	{
		return (this->d3dTexture != nullptr);
	}

	void DirectX11_Texture::clear()
	{
		// TODO
		/*
		D3DLOCKED_RECT lockRect;
		IDirect3DSurface9* buffer = NULL;
		LOCK_RESULT result = this->_tryLock(&buffer, &lockRect, NULL);
		if (result == LR_FAILED)
		{
			return;
		}
		memset(lockRect.pBits, 0, this->getByteSize());
		this->_unlock(buffer, result, true);
		*/
	}

	Color DirectX11_Texture::getPixel(int x, int y)
	{
		Color color = Color::Clear;
		// TODO
		/*
		D3DLOCKED_RECT lockRect;
		_CREATE_RECT(rect, x, y, 1, 1);
		IDirect3DSurface9* buffer = NULL;
		LOCK_RESULT result = this->_tryLock(&buffer, &lockRect, &rect);
		if (result == LR_FAILED)
		{
			return color;
		}
		unsigned char* p = (unsigned char*)lockRect.pBits;
		if (this->bpp == 4)
		{
			color.r = p[2];
			color.g = p[1];
			color.b = p[0];
			color.a = p[3];
		}
		else if (this->bpp == 3)
		{
			color.r = p[2];
			color.g = p[1];
			color.b = p[0];
			color.a = 255;
		}
		else if (this->bpp == 1)
		{
			color.r = 255;
			color.g = 255;
			color.b = 255;
			color.a = p[0];
		}
		else
		{
			hlog::error(april::logTag, "Unsupported format for getPixel()!");
		}
		this->_unlock(buffer, result, false);
		*/
		return color;
	}

	void DirectX11_Texture::setPixel(int x, int y, Color color)
	{
		// TODO
		/*
		x = hclamp(x, 0, this->width - 1);
		y = hclamp(y, 0, this->height - 1);
		D3DLOCKED_RECT lockRect;
		_CREATE_RECT(rect, x, y, 1, 1);
		IDirect3DSurface9* buffer = NULL;
		LOCK_RESULT result = this->_tryLock(&buffer, &lockRect, &rect);
		if (result == LR_FAILED)
		{
			return;
		}
		unsigned char* p = (unsigned char*)lockRect.pBits;
		if (this->bpp == 4)
		{
			p[2] = color.r;
			p[1] = color.g;
			p[0] = color.b;
			p[3] = color.a;
		}
		else if (this->bpp == 3)
		{
			p[2] = color.r;
			p[1] = color.g;
			p[0] = color.b;
		}
		else if (this->bpp == 1)
		{
			p[0] = (color.r + color.g + color.b) / 3;
		}
		else
		{
			hlog::error(april::logTag, "Unsupported format for setPixel()!");
		}
		this->_unlock(buffer, result, true);
		*/
	}

	void DirectX11_Texture::fillRect(int x, int y, int w, int h, Color color)
	{
		// TODO
		/*
		x = hclamp(x, 0, this->width - 1);
		y = hclamp(y, 0, this->height - 1);
		w = hclamp(w, 1, this->width - x);
		h = hclamp(h, 1, this->height - y);
		if (w == 1 && h == 1)
		{
			this->setPixel(x, y, color);
			return;
		}
		D3DLOCKED_RECT lockRect;
		_CREATE_RECT(rect, x, y, w, h);
		IDirect3DSurface9* buffer = NULL;
		LOCK_RESULT result = this->_tryLock(&buffer, &lockRect, &rect);
		if (result == LR_FAILED)
		{
			return;
		}
		unsigned char* p = (unsigned char*)lockRect.pBits;
		int i;
		int j;
		int offset;
		if (this->bpp == 4)
		{
			for_iterx (j, 0, h)
			{
				for_iterx (i, 0, w)
				{
					offset = (j * this->width + i) * this->bpp;
					p[offset + 2] = color.r;
					p[offset + 1] = color.g;
					p[offset + 0] = color.b;
					p[offset + 3] = color.a;
				}
			}
		}
		else if (this->bpp == 3)
		{
			for_iterx (j, 0, h)
			{
				for_iterx (i, 0, w)
				{
					offset = (i + j * this->width) * this->bpp;
					p[offset + 2] = color.r;
					p[offset + 1] = color.g;
					p[offset + 0] = color.b;
				}
			}
		}
		else if (this->bpp == 1)
		{
			for_iterx (j, 0, h)
			{
				for_iterx (i, 0, w)
				{
					offset = (i + j * this->width) * this->bpp;
					p[offset] = (color.r + color.g + color.b) / 3;
				}
			}
		}
		else
		{
			hlog::error(april::logTag, "Unsupported format for setPixel()!");
		}
		this->_unlock(buffer, result, true);
		*/
	}

	void DirectX11_Texture::blit(int x, int y, Texture* texture, int sx, int sy, int sw, int sh, unsigned char alpha)
	{
		// TODO
		/*
		DirectX11_Texture* source = (DirectX11_Texture*)texture;
		x = hclamp(x, 0, this->width - 1);
		y = hclamp(y, 0, this->height - 1);
		sx = hclamp(sx, 0, source->width - 1);
		sy = hclamp(sy, 0, source->height - 1);
		sw = hmin(sw, hmin(this->width - x, source->width - sx));
		sh = hmin(sh, hmin(this->height - y, source->height - sy));
		if (sw == 1 && sh == 1)
		{
			this->setPixel(x, y, source->getPixel(sx, sy));
			return;
		}
		D3DLOCKED_RECT lockRect;
		_CREATE_RECT(rect, sx, sy, sw, sh);
		IDirect3DSurface9* buffer = NULL;
		LOCK_RESULT result = source->_tryLock(&buffer, &lockRect, &rect);
		if (result == LR_FAILED)
		{
			return;
		}
		this->blit(x, y, (unsigned char*)lockRect.pBits, source->width, source->height, source->bpp, sx, sy, sw, sh, alpha);
		source->_unlock(buffer, result, false);
		*/
	}

	void DirectX11_Texture::blit(int x, int y, unsigned char* data, int dataWidth, int dataHeight, int dataBpp, int sx, int sy, int sw, int sh, unsigned char alpha)
	{
		// TODO
		/*
		x = hclamp(x, 0, this->width - 1);
		y = hclamp(y, 0, this->height - 1);
		sx = hclamp(sx, 0, dataWidth - 1);
		sy = hclamp(sy, 0, dataHeight - 1);
		sw = hmin(sw, hmin(this->width - x, dataWidth - sx));
		sh = hmin(sh, hmin(this->height - y, dataHeight - sy));
		D3DLOCKED_RECT lockRect;
		_CREATE_RECT(rect, x, y, sw, sh);
		IDirect3DSurface9* buffer = NULL;
		LOCK_RESULT result = this->_tryLock(&buffer, &lockRect, &rect);
		if (result == LR_FAILED)
		{
			return;
		}
		this->_blit((unsigned char*)lockRect.pBits, x, y, data, dataWidth, dataHeight, dataBpp, sx, sy, sw, sh, alpha);
		this->_unlock(buffer, result, true);
		*/
	}

	void DirectX11_Texture::stretchBlit(int x, int y, int w, int h, Texture* texture, int sx, int sy, int sw, int sh, unsigned char alpha)
	{
		// TODO
		/*
		DirectX11_Texture* source = (DirectX11_Texture*)texture;
		x = hclamp(x, 0, this->width - 1);
		y = hclamp(y, 0, this->height - 1);
		w = hmin(w, this->width - x);
		h = hmin(h, this->height - y);
		sx = hclamp(sx, 0, source->width - 1);
		sy = hclamp(sy, 0, source->height - 1);
		sw = hmin(sw, source->width - sx);
		sh = hmin(sh, source->height - sy);
		D3DLOCKED_RECT lockRect;
		HRESULT result = source->_getTexture()->LockRect(0, &lockRect, NULL, D3DLOCK_DISCARD);
		if (result != D3D_OK)
		{
			return;
		}
		this->stretchBlit(x, y, w, h, (unsigned char*)lockRect.pBits, source->width, source->height, source->bpp, sx, sy, sw, sh, alpha);
		source->_getTexture()->UnlockRect(0);
		*/
	}

	void DirectX11_Texture::stretchBlit(int x, int y, int w, int h, unsigned char* data, int dataWidth, int dataHeight, int dataBpp, int sx, int sy, int sw, int sh, unsigned char alpha)
	{
		// TODO
		/*
		x = hclamp(x, 0, this->width - 1);
		y = hclamp(y, 0, this->height - 1);
		w = hmin(w, this->width - x);
		h = hmin(h, this->height - y);
		sx = hclamp(sx, 0, dataWidth - 1);
		sy = hclamp(sy, 0, dataHeight - 1);
		sw = hmin(sw, dataWidth - sx);
		sh = hmin(sh, dataHeight - sy);
		D3DLOCKED_RECT lockRect;
		_CREATE_RECT(rect, x, y, w, h);
		IDirect3DSurface9* buffer = NULL;
		LOCK_RESULT result = this->_tryLock(&buffer, &lockRect, &rect);
		if (result == LR_FAILED)
		{
			return;
		}
		this->_stretchBlit((unsigned char*)lockRect.pBits, x, y, w, h, data, dataWidth, dataHeight, dataBpp, sx, sy, sw, sh, alpha);
		this->_unlock(buffer, result, true);
		*/
	}

	void DirectX11_Texture::rotateHue(float degrees)
	{
		// TODO
		/*
		if (degrees == 0.0f)
		{
			return;
		}
		D3DLOCKED_RECT lockRect;
		IDirect3DSurface9* buffer = NULL;
		LOCK_RESULT result = this->_tryLock(&buffer, &lockRect, NULL);
		if (result == LR_FAILED)
		{
			return;
		}
		int size = this->getByteSize();
		float range = hmodf(degrees, 360.0f) / 360.0f;
		float h;
		float s;
		float l;
		unsigned char* data = (unsigned char*)lockRect.pBits;
		for_iter_step (i, 0, size, this->bpp)
		{
			april::rgbToHsl(data[i + 2], data[i + 1], data[i], &h, &s, &l);
			april::hslToRgb(hmodf(h + range, 1.0f), s, l, &data[i + 2], &data[i + 1], &data[i]);
		}
		this->_unlock(buffer, result, true);
		*/
	}

	void DirectX11_Texture::saturate(float factor)
	{
		// TODO
		/*
		D3DLOCKED_RECT lockRect;
		IDirect3DSurface9* buffer = NULL;
		LOCK_RESULT result = this->_tryLock(&buffer, &lockRect, NULL);
		if (result == LR_FAILED)
		{
			return;
		}
		int size = this->getByteSize();
		float h;
		float s;
		float l;
		unsigned char* data = (unsigned char*)lockRect.pBits;
		for_iter_step (i, 0, size, this->bpp)
		{
			april::rgbToHsl(data[i + 2], data[i + 1], data[i], &h, &s, &l);
			april::hslToRgb(h, hmin(s * factor, 1.0f), l, &data[i + 2], &data[i + 1], &data[i]);
		}
		this->_unlock(buffer, result, true);
		*/
	}

	bool DirectX11_Texture::copyPixelData(unsigned char** output)
	{
		// TODO
		/*
		D3DLOCKED_RECT lockRect;
		IDirect3DSurface9* buffer = NULL;
		LOCK_RESULT result = this->_tryLock(&buffer, &lockRect, NULL);
		if (result == LR_FAILED)
		{
			return false;
		}
		unsigned char* p = (unsigned char*)lockRect.pBits;
		int i;
		int j;
		int offset;
		*output = new unsigned char[this->width * this->height * 4];
		if (this->bpp == 4)
		{
			memcpy(*output, p, this->getByteSize());
			for_iterx (j, 0, this->height)
			{
				for_iterx (i, 0, this->width)
				{
					offset = (j * this->width + i) * 4;
					(*output)[offset + 0] = p[offset + 2];
					//(*output)[offset + 1] = p[offset + 1]; // not necessary to be executed because of previous memcpy call
					(*output)[offset + 2] = p[offset + 0];
				}
			}
		}
		else if (this->bpp == 3)
		{
			for_iterx (j, 0, this->height)
			{
				for_iterx (i, 0, this->width)
				{
					offset = (j * this->width + i) * 4;
					(*output)[offset + 0] = p[offset + 2];
					(*output)[offset + 1] = p[offset + 1];
					(*output)[offset + 2] = p[offset + 0];
				}
			}
		}
		else if (this->bpp == 1)
		{
			memcpy(*output, p, this->getByteSize());
		}
		this->_unlock(buffer, result, false);
		return true;
		*/
		return false;
	}

	void DirectX11_Texture::insertAsAlphaMap(Texture* texture, unsigned char median, int ambiguity)
	{
		// TODO
		/*
		if (this->width != texture->getWidth() || this->height != texture->getHeight() || this->bpp != 4)
		{
			return;
		}
		DirectX11_Texture* source = (DirectX11_Texture*)texture;
		D3DLOCKED_RECT lockRect;
		IDirect3DSurface9* buffer = NULL;
		LOCK_RESULT result = this->_tryLock(&buffer, &lockRect, NULL);
		if (result == LR_FAILED)
		{
			return;
		}
		D3DLOCKED_RECT srcLockRect;
		IDirect3DSurface9* srcBuffer = NULL;
		LOCK_RESULT srcResult = source->_tryLock(&srcBuffer, &srcLockRect, NULL);
		if (srcResult == LR_FAILED)
		{
			this->_unlock(buffer, result, false);
			return;
		}
		unsigned char* thisData = (unsigned char*)lockRect.pBits;
		unsigned char* srcData = (unsigned char*)srcLockRect.pBits;
		unsigned char* c;
		unsigned char* sc;
		int i;
		int j;
		int alpha;
		int min = (int)median - ambiguity / 2;
		int max = (int)median + ambiguity / 2;
		for_iterx (j, 0, this->height)
		{
			for_iterx (i, 0, this->width)
			{
				c = &thisData[(i + j * this->width) * 4];
				sc = &srcData[(i + j * this->width) * 4];
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
		this->_unlock(buffer, result, true);
		source->_unlock(srcBuffer, srcResult, false);
		*/
	}

}

#endif
