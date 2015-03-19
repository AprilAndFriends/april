/// @file
/// @version 3.5
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
		virtual bool create(Options options);
		virtual bool destroy();

		virtual void assignWindow(Window* window) = 0;
		virtual void reset();

		HL_DEFINE_GET(hstr, name, Name);
		HL_DEFINE_GET(Options, options, Options);
		harray<Texture*> getTextures();
		HL_DEFINE_GET(grect, viewport, Viewport);
		HL_DEFINE_GET(gmat4, modelviewMatrix, ModelviewMatrix);
		void setModelviewMatrix(gmat4 matrix);
		HL_DEFINE_GET(gmat4, projectionMatrix, ProjectionMatrix);
		void setProjectionMatrix(gmat4 matrix);
		// TODO - move below, these aren't properties
		HL_DEFINE_GET(grect, orthoProjection, OrthoProjection);
		void setOrthoProjection(grect rect);
		void setOrthoProjection(grect rect, float nearZ, float farZ);
		void setOrthoProjection(gvec2 size);
		void setOrthoProjection(gvec2 size, float nearZ, float farZ);
		// TODOa - maybe use int64_t instead of long long
		unsigned long long getVRamConsumption();
		unsigned long long getRamConsumption();
		unsigned long long getAsyncRamConsumption();
		bool hasAsyncTexturesQueued();
		/// @note A timeout value of 0.0 means indefinitely.
		void waitForAsyncTextures(float timeout = 0.0f);

		virtual float getPixelOffset() = 0;
		virtual int getVRam() = 0;

		virtual harray<DisplayMode> getSupportedDisplayModes();
		virtual Caps getCaps();
		virtual void setViewport(grect value);
		HL_DEFINE_IS(depthBufferEnabled, DepthBufferEnabled);
		HL_DEFINE_IS(depthBufferWriteEnabled, DepthBufferWriteEnabled);

		virtual void setTextureBlendMode(BlendMode blendMode) = 0;
		/// @note The parameter factor is only used when the color mode is LERP.
		virtual void setTextureColorMode(ColorMode colorMode, float factor = 1.0f) = 0;
		virtual void setTextureFilter(Texture::Filter textureFilter) = 0;
		virtual void setTextureAddressMode(Texture::AddressMode textureAddressMode) = 0;
		virtual void setTexture(Texture* texture) = 0;

		virtual Texture* getRenderTarget();
		virtual void setRenderTarget(Texture* texture);
		virtual void setVertexShader(VertexShader* vertexShader);
		virtual void setPixelShader(PixelShader* pixelShader);
		virtual void setDepthBuffer(bool enabled, bool writeEnabled = true);

		Texture* createTextureFromResource(chstr filename, Texture::Type type = Texture::TYPE_IMMUTABLE, Texture::LoadMode loadMode = Texture::LOAD_IMMEDIATE);
		/// @note When a format is forced, it's best to use managed (but not necessary).
		Texture* createTextureFromResource(chstr filename, Image::Format format, Texture::Type type = Texture::TYPE_MANAGED, Texture::LoadMode loadMode = Texture::LOAD_IMMEDIATE);
		Texture* createTextureFromFile(chstr filename, Texture::Type type = Texture::TYPE_IMMUTABLE, Texture::LoadMode loadMode = Texture::LOAD_IMMEDIATE);
		/// @note When a format is forced, it's best to use managed (but not necessary).
		Texture* createTextureFromFile(chstr filename, Image::Format format, Texture::Type type = Texture::TYPE_MANAGED, Texture::LoadMode loadMode = Texture::LOAD_IMMEDIATE);
		Texture* createTexture(int w, int h, unsigned char* data, Image::Format format, Texture::Type type = Texture::TYPE_MANAGED);
		Texture* createTexture(int w, int h, Color color, Image::Format format, Texture::Type type = Texture::TYPE_MANAGED);
		virtual PixelShader* createPixelShader();
		virtual PixelShader* createPixelShader(chstr filename);
		virtual VertexShader* createVertexShader();
		virtual VertexShader* createVertexShader(chstr filename);

		void setIdentityTransform();
		void translate(float x, float y, float z = 0.0f);
		void rotate(float angle, float ax = 0.0f, float ay = 0.0f, float az = -1.0f);
		void scale(float s);
		void scale(float sx, float sy, float sz);
		void lookAt(const gvec3& eye, const gvec3& target, const gvec3& up);
		void setPerspective(float fov, float aspect, float nearClip, float farClip);

		virtual void clear(bool useColor = true, bool depth = false) = 0;
		virtual void clear(bool depth, grect rect, Color color = Color::Clear) = 0;
		virtual void render(RenderOperation renderOperation, PlainVertex* v, int nVertices) = 0;
		virtual void render(RenderOperation renderOperation, PlainVertex* v, int nVertices, Color color) = 0;
		virtual void render(RenderOperation renderOperation, TexturedVertex* v, int nVertices) = 0;
		virtual void render(RenderOperation renderOperation, TexturedVertex* v, int nVertices, Color color) = 0;
		virtual void render(RenderOperation renderOperation, ColoredVertex* v, int nVertices) = 0;
		virtual void render(RenderOperation renderOperation, ColoredTexturedVertex* v, int nVertices) = 0;
		
		void drawRect(grect rect, Color color);
		void drawFilledRect(grect rect, Color color);
		void drawTexturedRect(grect rect, grect src);
		void drawTexturedRect(grect rect, grect src, Color color);

		hstr findTextureResource(chstr filename);
		hstr findTextureFile(chstr filename);
		void unloadTextures();
		virtual Image::Format getNativeTextureFormat(Image::Format format) = 0;
		virtual unsigned int getNativeColorUInt(const april::Color& color) = 0;
		virtual Image* takeScreenshot(Image::Format format);
		virtual void presentFrame();

		DEPRECATED_ATTRIBUTE inline int getMaxTextureSize() { return this->getCaps().maxTextureSize; }

	protected:
		hstr name;
		bool created;
		Options options;
		harray<Texture*> textures;
		grect viewport;
		bool depthBufferEnabled;
		bool depthBufferWriteEnabled;
		RenderState* state;
		Texture::Filter textureFilter;
		Texture::AddressMode textureAddressMode;
		gmat4 modelviewMatrix;
		gmat4 projectionMatrix;
		grect orthoProjection;
		Caps caps;
		hmutex texturesMutex;

		Texture* _createTextureFromSource(bool fromResource, chstr filename, Texture::Type type, Texture::LoadMode loadMode, Image::Format format = Image::FORMAT_INVALID);
		virtual Texture* _createTexture(bool fromResource) = 0;

		void _registerTexture(Texture* texture);
		void _unregisterTexture(Texture* texture);

		virtual void _setModelviewMatrix(const gmat4& matrix) = 0;
		virtual void _setProjectionMatrix(const gmat4& matrix) = 0;
		
		virtual void _setupCaps() = 0;
		virtual void _setResolution(int w, int h, bool fullscreen);

		unsigned int _numPrimitives(RenderOperation renderOperation, int nVertices);
		unsigned int _limitPrimitives(RenderOperation renderOperation, int nVertices);

	};

	// global rendersys shortcut variable
	aprilFnExport extern april::RenderSystem* rendersys;
	
}

#endif
