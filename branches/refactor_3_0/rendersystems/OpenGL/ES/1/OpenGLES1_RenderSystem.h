/// @file
/// @author  Kresimir Spes
/// @author  Ivan Vucica
/// @author  Boris Mikic
/// @version 2.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/// 
/// @section DESCRIPTION
/// 
/// Defines an OpenGLES1 render system.

#ifdef _OPENGLES1
#ifndef APRIL_OPENGLES1_RENDER_SYSTEM_H
#define APRIL_OPENGLES1_RENDER_SYSTEM_H

#ifdef __APPLE__
#include <TargetConditionals.h>
#endif

#include "OpenGLES_RenderSystem.h"

namespace april
{
	class Image;
	class OpenGLES1_Texture;
	class Window;

	class OpenGLES1_RenderState
	{
	public:
		OpenGLES1_RenderState();
		~OpenGLES1_RenderState();
		
		void reset();

		bool textureCoordinatesEnabled;
		bool colorEnabled;
		unsigned int textureId;
		Texture::Filter textureFilter;
		Texture::AddressMode textureAddressMode;
		Color systemColor;
		bool modelviewMatrixChanged;
		bool projectionMatrixChanged;
		gmat4 modelviewMatrix;
		gmat4 projectionMatrix;
		BlendMode blendMode;
		ColorMode colorMode;
		unsigned char colorModeAlpha;
		unsigned int modeMatrix;
		int strideVertex;
		const void* pointerVertex;
		int strideTexCoord;
		const void* pointerTexCoord;
		int strideColor;
		const void* pointerColor;

	};
	
	class OpenGLES1_RenderSystem : public OpenGLES_RenderSystem
	{
	public:
		friend class OpenGLES1_Texture;

		OpenGLES1_RenderSystem();
		~OpenGLES1_RenderSystem();
		bool create(chstr options);
		bool destroy();

		void assignWindow(Window* window);
		void reset();
		
		float getPixelOffset() { return 0.0f; }
		grect getViewport();
		void setViewport(grect value);
		harray<DisplayMode> getSupportedDisplayModes();

		void setTextureBlendMode(BlendMode textureBlendMode);
		void setTextureColorMode(ColorMode textureColorMode, unsigned char alpha = 255);
		void setTextureFilter(Texture::Filter textureFilter);
		void setTextureAddressMode(Texture::AddressMode textureAddressMode);
		void setTexture(Texture* texture);
		Texture* getRenderTarget();
		void setRenderTarget(Texture* texture);
		void setPixelShader(PixelShader* pixelShader);
		void setVertexShader(VertexShader* vertexShader);

		void setResolution(int w, int h);

		Texture* createTexture(int w, int h, unsigned char* rgba);
		Texture* createTexture(int w, int h, Texture::Format format, Texture::Type type = Texture::TYPE_NORMAL, Color color = Color::Clear);
		PixelShader* createPixelShader();
		PixelShader* createPixelShader(chstr filename);
		VertexShader* createVertexShader();
		VertexShader* createVertexShader(chstr filename);

		void clear(bool useColor = true, bool depth = false);
		void clear(bool depth, grect rect, Color color = Color::Clear);
		void render(RenderOp renderOp, PlainVertex* v, int nVertices);
		void render(RenderOp renderOp, PlainVertex* v, int nVertices, Color color);
		void render(RenderOp renderOp, TexturedVertex* v, int nVertices);
		void render(RenderOp renderOp, TexturedVertex* v, int nVertices, Color color);
		void render(RenderOp renderOp, ColoredVertex* v, int nVertices);
		void render(RenderOp renderOp, ColoredTexturedVertex* v, int nVertices);
		
		void setParam(chstr name, chstr value);
		Image* takeScreenshot(int bpp = 3);

		void setMatrixMode(unsigned int mode);
		void bindTexture(unsigned int textureId);

		// TODO - refactor
		int _getMaxTextureSize();

	protected:
		OpenGLES1_RenderState deviceState;
		OpenGLES1_RenderState state;
		OpenGLES1_Texture* activeTexture;
		hstr options;

		void _setupDefaultParameters();
		Texture* _createTexture(chstr filename);

		void _setVertexPointer(int stride, const void* pointer);
		void _setTexCoordPointer(int stride, const void* pointer);
		void _setColorPointer(int stride, const void* pointer);
		void _setTextureBlendMode(BlendMode mode);
		void _setTextureColorMode(ColorMode mode, unsigned char alpha = 255);
		void _setTextureFilter(Texture::Filter textureFilter);
		void _setTextureAddressMode(Texture::AddressMode textureAddressMode);
		void _setModelviewMatrix(const gmat4& matrix);
		void _setProjectionMatrix(const gmat4& matrix);

		void _applyStateChanges();
		void _setClientState(unsigned int type, bool enabled);
#ifdef _WIN32
		void _releaseWindow();
#endif
		
	};
	
}

#endif
#endif
