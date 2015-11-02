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

	inline HDC getHDC() { return this->hDC; }

	inline float getPixelOffset() { return 0.0f; }
	inline int getVRam() { return 0; }

	april::Image::Format getNativeTextureFormat(april::Image::Format format);
	unsigned int getNativeColorUInt(const april::Color& color);

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

	void _setDeviceViewport(const grect& rect);
	void _setDeviceModelviewMatrix(const gmat4& matrix);
	void _setDeviceProjectionMatrix(const gmat4& matrix);
	void _setDeviceDepthBuffer(bool enabled, bool writeEnabled);
	void _setDeviceRenderMode(bool useTexture, bool useColor);
	void _setDeviceTexture(april::Texture* texture);
	void _setDeviceTextureFilter(april::Texture::Filter textureFilter);
	void _setDeviceTextureAddressMode(april::Texture::AddressMode textureAddressMode);
	void _setDeviceBlendMode(april::BlendMode blendMode);
	void _setDeviceColorMode(april::ColorMode colorMode, float colorModeFactor, bool useTexture, bool useColor, const april::Color& systemColor);

	void _deviceClear(bool depth);
	void _deviceClear(april::Color color, bool depth);
	void _deviceClearDepth();
	void _deviceRender(april::RenderOperation renderOperation, april::PlainVertex* v, int nVertices);
	void _deviceRender(april::RenderOperation renderOperation, april::TexturedVertex* v, int nVertices);
	void _deviceRender(april::RenderOperation renderOperation, april::ColoredVertex* v, int nVertices);
	void _deviceRender(april::RenderOperation renderOperation, april::ColoredTexturedVertex* v, int nVertices);

	// translation from abstract render ops to gl's render ops
	static int _glRenderOperations[];

};
#endif
