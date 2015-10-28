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
#include "Image.h"
#include "RenderState.h"
#include "Texture.h"

#include "Window.h" // can be removed later

namespace april
{
	class Image;
	class PixelShader;
	class Texture;
	class VertexShader;
	class Window;

	class aprilExport RenderSystem
	{
	public:
		friend class Texture;
		friend class Window;

		struct aprilExport DisplayMode
		{
		public:
			int width;
			int height;
			int refreshRate;

			DisplayMode(int width, int height, int refreshRate);
			~DisplayMode();

			bool operator==(const DisplayMode& other) const;
			bool operator!=(const DisplayMode& other) const;

			hstr toString();

		};
	
		struct aprilExport Options
		{
		public:
			bool depthBuffer;

			Options();
			~Options();

			hstr toString();

		};

		struct aprilExport Caps
		{
		public:
			int maxTextureSize;
			bool npotTexturesLimited;
			bool npotTextures;
			harray<Image::Format> textureFormats;

			Caps();
			~Caps();

		};

		RenderSystem();
		virtual ~RenderSystem();

		void init();
		bool create(Options options);
		bool destroy();
		void assignWindow(Window* window);
		void reset();

		HL_DEFINE_GET(hstr, name, Name);
		HL_DEFINE_GET(Options, options, Options);
		HL_DEFINE_GET(Caps, caps, Caps);
		harray<Texture*> getTextures();
		harray<DisplayMode> getDisplayModes();
		int64_t getVRamConsumption();
		/// @note This is the RAM consumed by only by the textures, not the entire process.
		int64_t getRamConsumption();
		int64_t getAsyncRamConsumption();
		bool hasAsyncTexturesQueued();
		grect getViewport();
		void setViewport(grect value);
		gmat4 getModelviewMatrix();
		void setModelviewMatrix(gmat4 matrix);
		gmat4 getProjectionMatrix();
		void setProjectionMatrix(gmat4 matrix);

		virtual float getPixelOffset() = 0;
		virtual int getVRam() = 0;

		Texture* createTextureFromResource(chstr filename, Texture::Type type = Texture::TYPE_IMMUTABLE, Texture::LoadMode loadMode = Texture::LOAD_IMMEDIATE);
		/// @note When a format is forced, it's best to use managed (but not necessary).
		Texture* createTextureFromResource(chstr filename, Image::Format format, Texture::Type type = Texture::TYPE_MANAGED, Texture::LoadMode loadMode = Texture::LOAD_IMMEDIATE);
		Texture* createTextureFromFile(chstr filename, Texture::Type type = Texture::TYPE_IMMUTABLE, Texture::LoadMode loadMode = Texture::LOAD_IMMEDIATE);
		/// @note When a format is forced, it's best to use managed (but not necessary).
		Texture* createTextureFromFile(chstr filename, Image::Format format, Texture::Type type = Texture::TYPE_MANAGED, Texture::LoadMode loadMode = Texture::LOAD_IMMEDIATE);
		Texture* createTexture(int w, int h, unsigned char* data, Image::Format format, Texture::Type type = Texture::TYPE_MANAGED);
		Texture* createTexture(int w, int h, Color color, Image::Format format, Texture::Type type = Texture::TYPE_MANAGED);
		void destroyTexture(Texture* texture);

		PixelShader* createPixelShaderFromResource(chstr filename);
		PixelShader* createPixelShaderFromFile(chstr filename);
		PixelShader* createPixelShader();
		VertexShader* createVertexShaderFromResource(chstr filename);
		VertexShader* createVertexShaderFromFile(chstr filename);
		VertexShader* createVertexShader();
		void destroyPixelShader(PixelShader* shader);
		void destroyVertexShader(VertexShader* shader);

		void setOrthoProjection(grect rect);
		void setOrthoProjection(grect rect, float nearZ, float farZ);
		void setOrthoProjection(gvec2 size);
		void setOrthoProjection(gvec2 size, float nearZ, float farZ);
		void setDepthBuffer(bool enabled, bool writeEnabled = true);

		void setTexture(Texture* texture);
		void setBlendMode(BlendMode blendMode);
		/// @note The parameter colorModeFactor is only used when the color mode is LERP.
		void setColorMode(ColorMode colorMode, float colorModeFactor = 1.0f);

		virtual Texture* getRenderTarget();
		virtual void setRenderTarget(Texture* texture);
		virtual void setVertexShader(VertexShader* vertexShader);
		virtual void setPixelShader(PixelShader* pixelShader);

		void setIdentityTransform();
		void translate(float x, float y, float z = 0.0f);
		void rotate(float angle, float ax = 0.0f, float ay = 0.0f, float az = -1.0f);
		void scale(float s);
		void scale(float sx, float sy, float sz);
		void lookAt(const gvec3& eye, const gvec3& target, const gvec3& up);
		void setPerspective(float fov, float aspect, float nearClip, float farClip);

		void clear(bool depth = false);
		void clear(april::Color color, bool depth = false);
		void clearDepth();
		/// @note Calling this will effectively set the current texture to NULL.
		void render(RenderOperation renderOperation, PlainVertex* v, int nVertices);
		/// @note Calling this will effectively set the current texture to NULL.
		void render(RenderOperation renderOperation, PlainVertex* v, int nVertices, Color color);
		void render(RenderOperation renderOperation, TexturedVertex* v, int nVertices);
		void render(RenderOperation renderOperation, TexturedVertex* v, int nVertices, Color color);
		/// @note Calling this will effectively set the current texture to NULL.
		void render(RenderOperation renderOperation, ColoredVertex* v, int nVertices);
		void render(RenderOperation renderOperation, ColoredTexturedVertex* v, int nVertices);
		
		/// @note Calling this will effectively set the current texture to NULL.
		void drawRect(grect rect, Color color);
		/// @note Calling this will effectively set the current texture to NULL.
		void drawFilledRect(grect rect, Color color);
		void drawTexturedRect(grect rect, grect src);
		void drawTexturedRect(grect rect, grect src, Color color);

		hstr findTextureResource(chstr filename);
		hstr findTextureFile(chstr filename);
		void unloadTextures();
		/// @note A timeout value of 0.0 means indefinitely.
		void waitForAsyncTextures(float timeout = 0.0f);

		virtual Image::Format getNativeTextureFormat(Image::Format format) = 0;
		virtual unsigned int getNativeColorUInt(const april::Color& color) = 0;
		virtual Image* takeScreenshot(Image::Format format);
		virtual void presentFrame();

		DEPRECATED_ATTRIBUTE inline int getMaxTextureSize() { return this->getCaps().maxTextureSize; }
		DEPRECATED_ATTRIBUTE inline harray<DisplayMode> getSupportedDisplayModes() { return this->getDisplayModes(); }
		DEPRECATED_ATTRIBUTE grect getOrthoProjection();
		DEPRECATED_ATTRIBUTE void clear(bool useColor, bool depth);
		DEPRECATED_ATTRIBUTE void clear(bool depth, grect rect, Color color = Color::Clear);
		DEPRECATED_ATTRIBUTE void setTextureFilter(Texture::Filter textureFilter);
		DEPRECATED_ATTRIBUTE void setTextureAddressMode(Texture::AddressMode textureAddressMode);
		DEPRECATED_ATTRIBUTE void setTextureBlendMode(BlendMode blendMode) { this->setBlendMode(blendMode); }
		DEPRECATED_ATTRIBUTE void setTextureColorMode(ColorMode colorMode, float factor = 1.0f) { this->setColorMode(colorMode, factor); }

	protected:
		hstr name;
		bool created;
		Options options;
		Caps caps;
		harray<Texture*> textures;
		harray<DisplayMode> displayModes;
		RenderState* state;
		RenderState* deviceState;
		hmutex texturesMutex;

		void _registerTexture(Texture* texture);
		void _unregisterTexture(Texture* texture);

		Texture* _createTextureFromSource(bool fromResource, chstr filename, Texture::Type type, Texture::LoadMode loadMode, Image::Format format = Image::FORMAT_INVALID);
		PixelShader* _createPixelShaderFromSource(bool fromResource, chstr filename);
		VertexShader* _createVertexShaderFromSource(bool fromResource, chstr filename);

		virtual void _updateDeviceState(bool forceUpdate = false);

		virtual void _deviceInit() = 0;
		virtual bool _deviceCreate(Options options) = 0;
		virtual bool _deviceDestroy() = 0;
		virtual void _deviceAssignWindow(Window* window) = 0;
		virtual void _deviceReset();
		virtual void _deviceSetupCaps() = 0;
		virtual void _deviceSetup() = 0;
		virtual void _deviceSetupDisplayModes();

		virtual Texture* _deviceCreateTexture(bool fromResource) = 0;
		virtual PixelShader* _deviceCreatePixelShader();
		virtual VertexShader* _deviceCreateVertexShader();

		virtual void _deviceChangeResolution(int w, int h, bool fullscreen);

		virtual void _setDeviceViewport(const grect& rect) = 0;
		virtual void _setDeviceModelviewMatrix(const gmat4& matrix) = 0;
		virtual void _setDeviceProjectionMatrix(const gmat4& matrix) = 0;
		virtual void _setDeviceDepthBuffer(bool enabled, bool writeEnabled) = 0;
		virtual void _setDeviceRenderMode(bool useTexture, bool useColor) = 0;
		virtual void _setDeviceTexture(Texture* texture) = 0;
		virtual void _setDeviceTextureFilter(Texture::Filter textureFilter) = 0;
		virtual void _setDeviceTextureAddressMode(Texture::AddressMode textureAddressMode) = 0;
		virtual void _setDeviceBlendMode(BlendMode blendMode) = 0;
		virtual void _setDeviceColorMode(ColorMode colorMode, float colorModeFactor, bool useTexture, bool useColor, const Color& systemColor) = 0;

		virtual void _deviceClear(bool depth) = 0;
		virtual void _deviceClear(april::Color color, bool depth) = 0;
		virtual void _deviceClearDepth() = 0;
		virtual void _deviceRender(RenderOperation renderOperation, PlainVertex* v, int nVertices) = 0;
		virtual void _deviceRender(RenderOperation renderOperation, TexturedVertex* v, int nVertices) = 0;
		virtual void _deviceRender(RenderOperation renderOperation, ColoredVertex* v, int nVertices) = 0;
		virtual void _deviceRender(RenderOperation renderOperation, ColoredTexturedVertex* v, int nVertices) = 0;

		unsigned int _numPrimitives(RenderOperation renderOperation, int nVertices);
		unsigned int _limitPrimitives(RenderOperation renderOperation, int nVertices);

	};

	// global rendersys shortcut variable
	aprilFnExport extern april::RenderSystem* rendersys;
	
}

#endif
