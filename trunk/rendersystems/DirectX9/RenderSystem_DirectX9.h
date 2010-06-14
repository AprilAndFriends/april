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

namespace April
{
	class DirectX9RenderSystem : public RenderSystem
	{
		bool mAppRunning;
		bool mTexCoordsEnabled,mColorEnabled;
		void _setModelviewMatrix(const gtypes::Matrix4& matrix);
		void _setProjectionMatrix(const gtypes::Matrix4& matrix);
		std::string mTitle;
	public:
		DirectX9RenderSystem(int w,int h,bool fullscreen,std::string title);
		~DirectX9RenderSystem();
		std::string getName();
		void configureDevice();
		
		// object creation
		Texture* loadTexture(std::string filename,bool dynamic);
		Texture* createTextureFromMemory(unsigned char* rgba,int w,int h);

		// modelview matrix transformation
		void setBlendMode(BlendMode mode);

		// rendering
		void clear(bool color,bool depth);
		void setTexture(Texture* t);
		void render(RenderOp renderOp,TexturedVertex* v,int nVertices);
		void render(RenderOp renderOp,TexturedVertex* v,int nVertices,float r,float g,float b,float a);
		void render(RenderOp renderOp,PlainVertex* v,int nVertices);
		void render(RenderOp renderOp,PlainVertex* v,int nVertices,float r,float g,float b,float a);
		void render(RenderOp renderOp,ColoredVertex* v,int nVertices);

		void setAlphaMultiplier(float value);
		
		void setWindowTitle(std::string title);
		gtypes::Vector2 getCursorPos();
		void showSystemCursor(bool b);
		bool isSystemCursorShown();

		int getWindowWidth();
		int getWindowHeight();
		
		void presentFrame();
		
		void triggerKeyEvent(bool down,unsigned int keycode);
		void triggerCharEvent(unsigned int chr);
	
		void triggerMouseUpEvent(int button);
		void triggerMouseDownEvent(int button);
		void triggerMouseMoveEvent();
		
		void enterMainLoop();
		void terminateMainLoop();
	};

	void createDX9RenderSystem(int w,int h,bool fullscreen,std::string title);
}
#endif
#endif
