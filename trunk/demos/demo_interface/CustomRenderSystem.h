/// @file
/// @version 3.5
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

class CustomTexture;

class CustomRenderSystem : public april::RenderSystem
{
public:
	friend class CustomTexture;

	CustomRenderSystem();
	~CustomRenderSystem();
	bool create(Options options);
	bool destroy();

	void reset();
	void assignWindow(april::Window* window);

	april::Texture* _createTexture(bool fromResource);

	inline float getPixelOffset() { return 0.0f; }
	inline int getVRam() { return 0; }
	void setViewport(grect value);

	void clear(bool useColor = true, bool depth = false);
	void clear(bool depth, grect rect, april::Color color = april::Color::Clear);

	void setTexture(april::Texture* texture);
	void setTextureBlendMode(april::BlendMode mode);
	/// @note The parameter factor is only used when the color mode is LERP.
	void setTextureColorMode(april::ColorMode textureColorMode, float factor = 1.0f);
	void setTextureFilter(april::Texture::Filter textureFilter);
	void setTextureAddressMode(april::Texture::AddressMode textureAddressMode);

	void render(april::RenderOperation renderOperation, april::PlainVertex* v, int nVertices);
	void render(april::RenderOperation renderOperation, april::PlainVertex* v, int nVertices, april::Color color);
	void render(april::RenderOperation renderOperation, april::TexturedVertex* v, int nVertices);
	void render(april::RenderOperation renderOperation, april::TexturedVertex* v, int nVertices, april::Color color);
	void render(april::RenderOperation renderOperation, april::ColoredVertex* v, int nVertices);
	void render(april::RenderOperation renderOperation, april::ColoredTexturedVertex* v, int nVertices);

	april::Image::Format getNativeTextureFormat(april::Image::Format format);
	unsigned int getNativeColorUInt(const april::Color& color);

	inline HDC getHDC() { return this->hDC; }
	bool _initWin32(april::Window* window);

protected:
	CustomTexture* activeTexture;

	virtual void _setupDefaultParameters();
	virtual void _applyTexture();
	void _setClientState(unsigned int type, bool enabled);
	void _setModelviewMatrix(const gmat4& matrix);
	void _setProjectionMatrix(const gmat4& matrix);

	void _setupCaps();

	void _setVertexPointer(int stride, const void* pointer);

	HWND hWnd;
	HDC hDC;
	HGLRC hRC;

	void _releaseWindow();

};
#endif
