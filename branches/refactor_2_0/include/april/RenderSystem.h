/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @version 2.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/// 
/// @section DESCRIPTION
/// 
/// Defines a generic render system.

#ifndef APRIL_RENDER_SYSTEM_H
#define APRIL_RENDER_SYSTEM_H

#include <hltypes/harray.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hstring.h>
#include <gtypes/Matrix4.h>
#include <gtypes/Rectangle.h>
#include <gtypes/Vector2.h>
#include <gtypes/Vector3.h>

#include "aprilExport.h"
#include "aprilUtil.h"
#include "Color.h"
#include "Texture.h"

#include "Window.h" // can be removed later

namespace april
{
	class ImageSource;
	class PixelShader;
	class RamTexture;
	class Texture;
	class VertexShader;
	class Window;

	class aprilExport RenderSystem
	{
	public:
		friend class Texture;

		RenderSystem();
		virtual ~RenderSystem();
		virtual bool create(chstr options);
		virtual bool destroy();

		virtual void assignWindow(Window* window) = 0;
		virtual void reset();

		HL_DEFINE_GET(hstr, name, Name);
		HL_DEFINE_GET(harray<Texture*>, textures, Textures);
		HL_DEFINE_GET(gmat4, modelviewMatrix, ModelviewMatrix);
		void setModelviewMatrix(gmat4 matrix);
		HL_DEFINE_GET(gmat4, projectionMatrix, ProjectionMatrix);
		void setProjectionMatrix(gmat4 matrix);
		HL_DEFINE_GET(grect, orthoProjection, OrthoProjection);
		void setOrthoProjection(grect rect);
		void setOrthoProjection(gvec2 size);
		HL_DEFINE_GETSET(float, textureIdleUnloadTime, TextureIdleUnloadTime);
		HL_DEFINE_ISSET(bool, forcedDynamicLoading, ForcedDynamicLoading);

		virtual float getPixelOffset() = 0;
		virtual harray<DisplayMode> getSupportedDisplayModes() = 0;
		virtual grect getViewport() = 0;
		virtual void setViewport(grect value) = 0;

		virtual void setTextureBlendMode(BlendMode blendMode) = 0;
		virtual void setTextureColorMode(ColorMode colorMode, unsigned char alpha = 255) = 0;
		virtual void setTextureFilter(Texture::Filter textureFilter) = 0;
		virtual void setTextureAddressMode(Texture::AddressMode textureAddressMode) = 0;
		virtual void setTexture(Texture* texture) = 0;
		virtual Texture* getRenderTarget() = 0;
		virtual void setRenderTarget(Texture* texture) = 0;
		virtual void setVertexShader(VertexShader* vertexShader) = 0;
		virtual void setPixelShader(PixelShader* pixelShader) = 0;

		virtual void setFullscreen(bool fullscreen) { } // TODO - main part should be in window class
		virtual void setResolution(int w, int h); // TODO - main part should be in window class

		virtual Texture* loadTexture(chstr filename, bool dynamic = false);
		virtual Texture* createTexture(int w, int h, unsigned char* rgba) = 0;
		virtual Texture* createTexture(int w, int h, Texture::Format format = Texture::FORMAT_RGBA, Texture::Type type = Texture::TYPE_NORMAL, Color color = APRIL_COLOR_CLEAR) = 0;
		RamTexture* loadRamTexture(chstr filename, bool dynamic = false);
		virtual PixelShader* createPixelShader() = 0;
		virtual PixelShader* createPixelShader(chstr filename) = 0;
		virtual VertexShader* createVertexShader() = 0;
		virtual VertexShader* createVertexShader(chstr filename) = 0;

		void setIdentityTransform();
		void translate(float x, float y, float z = 0.0f);
		void rotate(float angle, float ax = 0.0f, float ay = 0.0f, float az = -1.0f);
		void scale(float s);
		void scale(float sx, float sy, float sz);
		void lookAt(const gvec3 &eye, const gvec3 &direction, const gvec3 &up);
		void setPerspective(float fov, float aspect, float nearClip, float farClip);

		virtual void clear(bool useColor = true, bool depth = false) = 0;
		virtual void clear(bool depth, grect rect, Color color = APRIL_COLOR_CLEAR) = 0;
		virtual void render(RenderOp renderOp, PlainVertex* v, int nVertices) = 0;
		virtual void render(RenderOp renderOp, PlainVertex* v, int nVertices, Color color) = 0;
		virtual void render(RenderOp renderOp, TexturedVertex* v, int nVertices) = 0;
		virtual void render(RenderOp renderOp, TexturedVertex* v, int nVertices, Color color) = 0;
		virtual void render(RenderOp renderOp, ColoredVertex* v, int nVertices) = 0;
		virtual void render(RenderOp renderOp, ColoredTexturedVertex* v, int nVertices) = 0;
		
		void drawRect(grect rect, Color color);
		void drawFilledRect(grect rect, Color color);
		void drawTexturedRect(grect rect, grect src);
		void drawTexturedRect(grect rect, grect src, Color color);

		void unloadTextures();
		virtual void setParam(chstr name, chstr value) { }
		virtual hstr getParam(chstr name) { return ""; }
        virtual ImageSource* takeScreenshot(int bpp = 3) = 0;
		virtual void presentFrame();

		DEPRECATED_ATTRIBUTE Window* getWindow() { return april::window; }
		DEPRECATED_ATTRIBUTE void setOrthoProjection(float w, float h, float x_offset = 0.0f, float y_offset = 0.0f) { this->setOrthoProjection(grect(x_offset, y_offset, w, h)); }
		DEPRECATED_ATTRIBUTE bool isFullscreen() { return april::window->isFullscreen(); }
		DEPRECATED_ATTRIBUTE float getIdleTextureUnloadTime() { return this->getTextureIdleUnloadTime(); }
		DEPRECATED_ATTRIBUTE void setIdleTextureUnloadTime(float value) { this->setTextureIdleUnloadTime(value); }
		DEPRECATED_ATTRIBUTE bool isDynamicLoadingForced() { return this->isForcedDynamicLoading(); }
		DEPRECATED_ATTRIBUTE void forceDynamicLoading(bool value) { this->setForcedDynamicLoading(value); }
		DEPRECATED_ATTRIBUTE void restore() { this->reset(); }
		DEPRECATED_ATTRIBUTE void clear(bool useColor, bool depth, grect rect, Color color = APRIL_COLOR_CLEAR) { this->clear(depth, rect, color); }
		DEPRECATED_ATTRIBUTE ImageSource* gradScreenshot(int bpp = 3) { return this->takeScreenshot(bpp); }
		DEPRECATED_ATTRIBUTE Texture* createTextureFromMemory(unsigned char* rgba, int w, int h) { return this->createTexture(w, h, rgba); }
		DEPRECATED_ATTRIBUTE Texture* createEmptyTexture(int w, int h, Texture::Format format = Texture::FORMAT_RGBA, Texture::Type type = Texture::TYPE_NORMAL) { return this->createTexture(w, h, format, type); }
		DEPRECATED_ATTRIBUTE Texture* createBlankTexture(int w, int h, Texture::Format format = Texture::FORMAT_RGBA, Texture::Type type = Texture::TYPE_NORMAL) { return this->createTexture(w, h, format, type, Color(APRIL_COLOR_WHITE, 0)); }
		DEPRECATED_ATTRIBUTE void setTextureWrapping(bool value) { this->setTextureAddressMode(value ? Texture::ADDRESS_WRAP : Texture::ADDRESS_CLAMP); }
		DEPRECATED_ATTRIBUTE RamTexture* loadRAMTexture(chstr filename, bool dynamic = false) { return this->loadRamTexture(filename, dynamic); }
		DEPRECATED_ATTRIBUTE void drawQuad(grect rect, Color color) { this->drawRect(rect, color); }
		DEPRECATED_ATTRIBUTE void drawColoredQuad(grect rect, Color color) { this->drawFilledRect(rect, color); }
		DEPRECATED_ATTRIBUTE void drawTexturedQuad(grect rect, grect src) { this->drawTexturedRect(rect, src); }
		DEPRECATED_ATTRIBUTE void drawTexturedQuad(grect rect, grect src, Color color) { this->drawTexturedRect(rect, src, color); }
		DEPRECATED_ATTRIBUTE void setBlendMode(BlendMode blendMode) { this->setTextureBlendMode(blendMode); }
		DEPRECATED_ATTRIBUTE void setColorMode(ColorMode colorMode, unsigned char alpha = 255) { this->setTextureColorMode(colorMode, alpha); }

	protected:
		hstr name;
		bool created;
		harray<Texture*> textures;
		float textureIdleUnloadTime;
		Texture::Filter textureFilter;
		Texture::AddressMode textureAddressMode;
		gmat4 modelviewMatrix;
		gmat4 projectionMatrix;
		grect orthoProjection;

		bool forcedDynamicLoading;
		
		virtual Texture* _createTexture(chstr filename, bool dynamic = false) = 0;

		void _registerTexture(Texture* texture);
		void _unregisterTexture(Texture* texture);
		hstr _findTextureFilename(chstr filename);

		virtual void _setModelviewMatrix(const gmat4& matrix) = 0;
		virtual void _setProjectionMatrix(const gmat4& matrix) = 0;
		
	};

	// global rendersys shortcut variable
	aprilFnExport extern april::RenderSystem* rendersys;
	
}

#endif
