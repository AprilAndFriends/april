/// @file
/// @version 4.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Defines a DirectX9 render system.

#ifdef _DIRECTX9
#ifndef APRIL_DIRECTX9_RENDER_SYSTEM_H
#define APRIL_DIRECTX9_RENDER_SYSTEM_H

#include <d3d9.h>
#include <d3d9types.h>

#define __HL_INCLUDE_PLATFORM_HEADERS
#include <hltypes/hplatform.h>

#include "DirectX_RenderSystem.h"

namespace april
{
	class DirectX9_PixelShader;
	class DirectX9_Texture;
	class DirectX9_VertexShader;
	class Image;
	class Window;

	class DirectX9_RenderSystem : public DirectX_RenderSystem
	{
	public:
		friend class DirectX9_PixelShader;
		friend class DirectX9_Texture;
		friend class DirectX9_VertexShader;

		DirectX9_RenderSystem();
		~DirectX9_RenderSystem();

		float getPixelOffset();
		int getVRam();

		//void setTextureBlendMode(BlendMode textureBlendMode);
		/// @note The parameter factor is only used when the color mode is LERP.
		//void setTextureColorMode(ColorMode textureColorMode, float factor = 1.0f);
		//void setTextureFilter(Texture::Filter textureFilter);
		//void setTextureAddressMode(Texture::AddressMode textureAddressMode);
		Texture* getRenderTarget();
		void setRenderTarget(Texture* source);
		void setPixelShader(PixelShader* pixelShader);
		void setVertexShader(VertexShader* vertexShader);

		Image::Format getNativeTextureFormat(Image::Format format);
		unsigned int getNativeColorUInt(const april::Color& color);
		Image* takeScreenshot(Image::Format format);
		void presentFrame();

	protected:
		bool textureCoordinatesEnabled;
		bool colorEnabled;
		DirectX9_Texture* activeTexture;
		DirectX9_Texture* renderTarget;
		IDirect3DSurface9* backBuffer;
		IDirect3D9* d3d;
		IDirect3DDevice9* d3dDevice;
		_D3DPRESENT_PARAMETERS_* d3dpp;
		HWND childHWnd;

		void _deviceInit();
		bool _deviceCreate(Options options);
		bool _deviceDestroy();
		void _deviceAssignWindow(Window* window);
		void _deviceReset();
		void _deviceSetupCaps();
		void _deviceSetupDisplayModes();

		void _tryAssignChildWindow();
		void _tryUnassignChildWindow();

		void _configureDevice();

		Texture* _deviceCreateTexture(bool fromResource);
		PixelShader* _deviceCreatePixelShader();
		VertexShader* _deviceCreateVertexShader();

		void _deviceChangeResolution(int w, int h, bool fullscreen);

		void _setDeviceViewport(const grect& rect);
		void _setDeviceModelviewMatrix(const gmat4& matrix);
		void _setDeviceProjectionMatrix(const gmat4& matrix);
		void _setDeviceDepthBuffer(bool enabled, bool writeEnabled);
		void _setDeviceTexture(Texture* texture);
		void _setDeviceTextureFilter(Texture::Filter textureFilter);
		void _setDeviceTextureAddressMode(Texture::AddressMode textureAddressMode);
		void _setDeviceBlendMode(BlendMode blendMode);
		void _setDeviceColorMode(ColorMode colorMode, float colorModeFactor, bool useColor, const Color& systemColor);

		void _deviceClear(bool depth);
		void _deviceClear(april::Color color, bool depth);
		void _deviceClear(april::Color color, grect rect, bool depth);
		void _deviceClearDepth();
		void _deviceRender(RenderOperation renderOperation, PlainVertex* v, int nVertices);
		void _deviceRender(RenderOperation renderOperation, PlainVertex* v, int nVertices, Color color);
		void _deviceRender(RenderOperation renderOperation, TexturedVertex* v, int nVertices);
		void _deviceRender(RenderOperation renderOperation, TexturedVertex* v, int nVertices, Color color);
		void _deviceRender(RenderOperation renderOperation, ColoredVertex* v, int nVertices);
		void _deviceRender(RenderOperation renderOperation, ColoredTexturedVertex* v, int nVertices);

		static D3DPRIMITIVETYPE dx9RenderOperations[];

	private:
		bool _supportsA8Surface; // this does not seem to be detectable via any type of device caps

	};

}
#endif
#endif
