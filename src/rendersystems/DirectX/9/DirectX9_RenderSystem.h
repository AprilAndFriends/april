/// @file
/// @version 4.3
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
	class DirectX9_RenderState;
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

		int getVRam() const;

		Image::Format getNativeTextureFormat(Image::Format format) const;
		unsigned int getNativeColorUInt(const april::Color& color) const;
		Image* takeScreenshot(Image::Format format);
		void presentFrame();

		// TODOa - these need to be refactored
		Texture* getRenderTarget();
		void setRenderTarget(Texture* source);
		void setPixelShader(PixelShader* pixelShader);
		void setVertexShader(VertexShader* vertexShader);

	protected:
		IDirect3D9* d3d;
		IDirect3DDevice9* d3dDevice;
		_D3DPRESENT_PARAMETERS_* d3dpp;
		IDirect3DSurface9* backBuffer;
		HWND childHWnd;
		// TODOa - these need to be refactored
		DirectX9_Texture* renderTarget;

		void _deviceInit();
		bool _deviceCreate(Options options);
		bool _deviceDestroy();
		void _deviceAssignWindow(Window* window);
		void _deviceReset();
		void _deviceSetupCaps();
		void _deviceSetup();
		void _deviceSetupDisplayModes();

		void _tryAssignChildWindow();
		void _tryUnassignChildWindow();

		Texture* _deviceCreateTexture(bool fromResource);
		PixelShader* _deviceCreatePixelShader();
		VertexShader* _deviceCreateVertexShader();

		void _deviceChangeResolution(int w, int h, bool fullscreen);

		void _setDeviceViewport(const grect& rect);
		void _setDeviceModelviewMatrix(const gmat4& matrix);
		void _setDeviceProjectionMatrix(const gmat4& matrix);
		void _setDeviceDepthBuffer(bool enabled, bool writeEnabled);
		void _setDeviceRenderMode(bool useTexture, bool useColor);
		void _setDeviceTexture(Texture* texture);
		void _setDeviceTextureFilter(Texture::Filter textureFilter);
		void _setDeviceTextureAddressMode(Texture::AddressMode textureAddressMode);
		void _setDeviceBlendMode(BlendMode blendMode);
		void _setDeviceColorMode(ColorMode colorMode, float colorModeFactor, bool useTexture, bool useColor, const Color& systemColor);

		void _deviceClear(bool depth);
		void _deviceClear(april::Color color, bool depth);
		void _deviceClearDepth();
		void _deviceRender(RenderOperation renderOperation, PlainVertex* vertices, int count);
		void _deviceRender(RenderOperation renderOperation, TexturedVertex* vertices, int count);
		void _deviceRender(RenderOperation renderOperation, ColoredVertex* vertices, int count);
		void _deviceRender(RenderOperation renderOperation, ColoredTexturedVertex* vertices, int count);

		static D3DPRIMITIVETYPE _dx9RenderOperations[];

	private:
		bool _supportsA8Surface; // this does not seem to be detectable via any type of device caps

	};

}
#endif
#endif
