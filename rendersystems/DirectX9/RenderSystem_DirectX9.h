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
	class Directx9Texture : public Texture
	{
	public:
		unsigned int mTexId;
		
		Directx9Texture(std::string filename,bool dynamic);
		Directx9Texture(unsigned char* rgba,int w,int h);
		~Directx9Texture();
		
		bool load();
		void unload();
		bool isLoaded();
		int getSizeInBytes();
	};

	class DirectX9RenderSystem : public RenderSystem
	{
		bool mTexCoordsEnabled,mColorEnabled;
		void _setModelviewMatrix(const gtypes::Matrix4& matrix);
		void _setProjectionMatrix(const gtypes::Matrix4& matrix);
	public:
		DirectX9RenderSystem(int w,int h);
		~DirectX9RenderSystem();
		std::string getName();
		
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
		
		bool triggerUpdate(float time_increase);
		bool triggerKeyEvent(bool down,unsigned int keycode,unsigned int charcode);
		bool triggerMouseEvent(int event,float x,float y,int button);
		
		void enterMainLoop();
		void terminateMainLoop();
	};

	void createDX9RenderSystem(int w,int h,bool fullscreen,std::string title);
}
#endif
#endif
