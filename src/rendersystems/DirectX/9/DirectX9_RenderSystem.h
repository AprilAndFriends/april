/// @file
/// @version 5.2
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

		// TODOa - these need to be refactored
		void setPixelShader(PixelShader* pixelShader);
		void setVertexShader(VertexShader* vertexShader);

	protected:
		IDirect3D9* d3d;
		IDirect3DDevice9* d3dDevice;
		_D3DPRESENT_PARAMETERS_* d3dpp;
		IDirect3DSurface9* backBuffer;
		HWND childHWnd;

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

		void _deviceChangeResolution(int width, int height, bool fullscreen);

		void _setDeviceViewport(cgrecti rect);
		void _setDeviceModelviewMatrix(const gmat4& matrix);
		void _setDeviceProjectionMatrix(const gmat4& matrix);
		void _setDeviceDepthBuffer(bool enabled, bool writeEnabled);
		void _setDeviceRenderMode(bool useTexture, bool useColor);
		void _setDeviceTexture(Texture* texture);
		void _setDeviceTextureFilter(const Texture::Filter& textureFilter);
		void _setDeviceTextureAddressMode(const Texture::AddressMode& textureAddressMode);
		void _setDeviceBlendMode(const BlendMode& blendMode);
		void _setDeviceColorMode(const ColorMode& colorMode, float colorModeFactor, bool useTexture, bool useColor, const Color& systemColor);
		void _setDeviceRenderTarget(Texture* texture);

		void _deviceClear(bool depth);
		void _deviceClear(const Color& color, bool depth);
		void _deviceClearDepth();
		void _deviceRender(const RenderOperation& renderOperation, const PlainVertex* vertices, int count);
		void _deviceRender(const RenderOperation& renderOperation, const TexturedVertex* vertices, int count);
		void _deviceRender(const RenderOperation& renderOperation, const ColoredVertex* vertices, int count);
		void _deviceRender(const RenderOperation& renderOperation, const ColoredTexturedVertex* vertices, int count);
		void _devicePresentFrame(bool systemEnabled);
		void _deviceCopyRenderTargetData(Texture* source, Texture* destination);
		void _deviceTakeScreenshot(Image::Format format, bool backBufferOnly);

		static D3DPRIMITIVETYPE _dx9RenderOperations[];

	private:
		bool _supportsA8Surface; // this does not seem to be detectable via any type of device caps

	};

}
#endif
#endif
