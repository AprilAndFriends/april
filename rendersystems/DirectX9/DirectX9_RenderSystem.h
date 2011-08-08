/************************************************************************************\
This source file is part of the Awesome Portable Rendering Interface Library         *
For latest info, see http://libapril.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes                                                     *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#ifdef _DIRECTX9
#ifndef APRIL_DIRECTX9_RENDERSYSTEM_H
#define APRIL_DIRECTX9_RENDERSYSTEM_H

#include "RenderSystem.h"

struct IDirect3DSurface9;

namespace april
{
	class DirectX9_Texture;
	class Window;

	class DirectX9_RenderSystem : public RenderSystem
	{
	public:
		DirectX9_RenderSystem();
		~DirectX9_RenderSystem();

		void assignWindow(Window* window);
		
		void configureDevice();
		
		// object creation
		Texture* loadTexture(chstr filename,bool dynamic);
		Texture* createTextureFromMemory(unsigned char* rgba, int w, int h);
		Texture* createEmptyTexture(int w, int h, TextureFormat fmt, TextureType type);

		void setBlendMode(BlendMode mode);
		void setTextureFilter(TextureFilter filter);
		void setTextureWrapping(bool wrap);
		void setResolution(int w, int h);
		// caps
		float getPixelOffset();
		hstr getName();
		// rendering
		void clear(bool useColor = true, bool depth = false);
		void clear(bool useColor, bool depth, grect rect, Color color = APRIL_COLOR_CLEAR);
		ImageSource* grabScreenshot(int bpp = 3);
		void setTexture(Texture* t);
		void render(RenderOp renderOp, PlainVertex* v, int nVertices);
		void render(RenderOp renderOp, PlainVertex* v, int nVertices, Color color);
		void render(RenderOp renderOp, TexturedVertex* v, int nVertices);
		void render(RenderOp renderOp, TexturedVertex* v, int nVertices, Color color);
		void render(RenderOp renderOp, ColoredVertex* v, int nVertices);
		void render(RenderOp renderOp, ColoredTexturedVertex* v, int nVertices);

		void setRenderTarget(Texture* source);

		void beginFrame();
		
		void presentFrame();
		harray<DisplayMode> getSupportedDisplayModes();

		static DirectX9_RenderSystem* create();
		
	protected:
		bool mTexCoordsEnabled;
		bool mColorEnabled;
		void _setModelviewMatrix(const gmat4& matrix);
		void _setProjectionMatrix(const gmat4& matrix);
		hstr mTitle;
		DirectX9_Texture* mRenderTarget;
		IDirect3DSurface9* mBackBuffer;
		
	};

}
#endif
#endif
