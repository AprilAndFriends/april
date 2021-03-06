/// @file
/// @version 4.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifndef CUSTOM_RENDER_SYSTEM_H
#define CUSTOM_RENDER_SYSTEM_H

#include <april/RenderSystem.h>
#include <april/Window.h>
#include <hltypes/hplatform.h>
#include <hltypes/hstring.h>

#define LOG_TAG "demo_interface"

class CustomRenderSystem : public april::RenderSystem
{
public:
	friend class CustomTexture;

	CustomRenderSystem();
	~CustomRenderSystem();

	inline HDC getHDC() const { return this->hDC; }

	inline int getVRam() const { return 0; }

	april::Image::Format getNativeTextureFormat(april::Image::Format format) const;
	unsigned int getNativeColorUInt(const april::Color& color) const;

protected:
	HWND hWnd;
	HDC hDC;
	HGLRC hRC;

	virtual void _releaseWindow();
	virtual bool _initWin32(april::Window* window);

	void _deviceInit();
	bool _deviceCreate(Options options);
	bool _deviceDestroy();
	void _deviceAssignWindow(april::Window* window);
	void _deviceSetupCaps();
	void _deviceSetup();

	april::Texture* _deviceCreateTexture(bool fromResource);

	void _setDeviceViewport(cgrecti rect);
	void _setDeviceModelviewMatrix(const gmat4& matrix);
	void _setDeviceProjectionMatrix(const gmat4& matrix);
	void _setDeviceDepthBuffer(bool enabled, bool writeEnabled);
	void _setDeviceRenderMode(bool useTexture, bool useColor);
	void _setDeviceTexture(april::Texture* texture);
	void _setDeviceTextureFilter(const april::Texture::Filter& textureFilter);
	void _setDeviceTextureAddressMode(const april::Texture::AddressMode& textureAddressMode);
	void _setDeviceBlendMode(const april::BlendMode& blendMode);
	void _setDeviceColorMode(const april::ColorMode& colorMode, float colorModeFactor, bool useTexture, bool useColor, const april::Color& systemColor);

	void _deviceClear(bool depth);
	void _deviceClear(const april::Color& color, bool depth);
	void _deviceClearDepth();
	void _deviceRender(const april::RenderOperation& renderOperation, const april::PlainVertex* v, int nVertices);
	void _deviceRender(const april::RenderOperation& renderOperation, const april::TexturedVertex* v, int nVertices);
	void _deviceRender(const april::RenderOperation& renderOperation, const april::ColoredVertex* v, int nVertices);
	void _deviceRender(const april::RenderOperation& renderOperation, const april::ColoredTexturedVertex* v, int nVertices);

	// translation from abstract render ops to gl's render ops
	static int _glRenderOperations[];

};
#endif
