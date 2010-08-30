/************************************************************************************\
This source file is part of the Awesome Portable Rendering Interface Library         *
For latest info, see http://libapril.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes (kreso@cateia.com)                                  *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#ifndef RENDERSYSTEM_DX9_H
#ifdef _DIRECTX9
#define RENDERSYSTEM_DX9_H

#include "RenderSystem.h"

class IDirect3DSurface9;

namespace April
{
	class DirectX9Texture;

	class DirectX9RenderSystem : public RenderSystem
	{
		bool mAppRunning;
		bool mTexCoordsEnabled,mColorEnabled;
		void _setModelviewMatrix(const gtypes::Matrix4& matrix);
		void _setProjectionMatrix(const gtypes::Matrix4& matrix);
		hstr mTitle;
		DirectX9Texture* mRenderTarget;
		IDirect3DSurface9* mBackBuffer;
	public:
		DirectX9RenderSystem(int w,int h,bool fullscreen,chstr title);
		~DirectX9RenderSystem();
		
		void configureDevice();
		
		// object creation
		Texture* loadTexture(chstr filename,bool dynamic);
		Texture* createTextureFromMemory(unsigned char* rgba,int w,int h);
		Texture* createEmptyTexture(int w,int h,TextureFormat fmt,TextureType type);

		void setBlendMode(BlendMode mode);
		void setTextureFilter(TextureFilter filter);
		void setTextureWrapping(bool wrap);
		// caps
		float getPixelOffset();
		hstr getName();
		// rendering
		void clear(bool color,bool depth);
		void setTexture(Texture* t);
		void render(RenderOp renderOp,ColoredTexturedVertex* v,int nVertices);
		void render(RenderOp renderOp,TexturedVertex* v,int nVertices);
		void render(RenderOp renderOp,TexturedVertex* v,int nVertices,float r,float g,float b,float a);
		void render(RenderOp renderOp,PlainVertex* v,int nVertices);
		void render(RenderOp renderOp,PlainVertex* v,int nVertices,float r,float g,float b,float a);
		void render(RenderOp renderOp,ColoredVertex* v,int nVertices);

		void setRenderTarget(Texture* source);

		void setAlphaMultiplier(float value);
		
		void setWindowTitle(chstr title);
		gtypes::Vector2 getCursorPos();
		void showSystemCursor(bool b);
		bool isSystemCursorShown();

		int getWindowWidth();
		int getWindowHeight();
		
		void beginFrame();
		void presentFrame();
		
		void triggerKeyEvent(bool down,unsigned int keycode);
		void triggerCharEvent(unsigned int chr);
	
		void triggerMouseUpEvent(int button);
		void triggerMouseDownEvent(int button);
		void triggerMouseMoveEvent();
		bool triggerQuitEvent();
		
		void enterMainLoop();
		void terminateMainLoop();
	};

	void createDX9RenderSystem(int w,int h,bool fullscreen,chstr title);
}
#endif
#endif
