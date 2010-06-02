/************************************************************************************\
This source file is part of the Awesome Portable Rendering Interface Library         *
For latest info, see http://libapril.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes (kreso@cateia.com)                                  *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#ifdef _DIRECTX9

#include "RenderSystem_DirectX9.h"
#include "ImageSource.h"
#include <d3dx9.h>
#include <gtypes/Vector2.h>

namespace April
{
	IDirect3D9* d3d=0;
	IDirect3DDevice9* d3dDevice=0;

	unsigned int platformLoadDirectx9Texture(const char* name,int* w,int* h)
	{
		unsigned int texid;
		ImageSource* img=loadImage(name);
		if (!img) return 0;
		*w=img->w; *h=img->h;

		delete img;


		return texid;
	}

	Directx9Texture::Directx9Texture(std::string filename,bool dynamic)
	{
		mFilename=filename;
		mDynamic=dynamic;
		mTexId=0; mWidth=mHeight=0;
	}

	Directx9Texture::Directx9Texture(unsigned char* rgba,int w,int h)
	{
		mWidth=w; mHeight=h;
		mDynamic=0;
		mFilename="UserTexture";
	}

	Directx9Texture::~Directx9Texture()
	{
		unload();
	}

	bool Directx9Texture::load()
	{
		mUnusedTimer=0;
		if (mTexId) return 1;
		rendersys->logMessage("loading DX9 texture '"+mFilename+"'");


		return 1;
	}

	bool Directx9Texture::isLoaded()
	{
		return mTexId != 0;
	}

	void Directx9Texture::unload()
	{
		if (mTexId)
		{
			rendersys->logMessage("unloading DX9 texture '"+mFilename+"'");

			mTexId=0;
		}
	}

	int Directx9Texture::getSizeInBytes()
	{
		return mWidth*mHeight*3;
	}
/**********************************************************************************************/
	DirectX9RenderSystem::DirectX9RenderSystem(int w,int h,bool fullscreen,std::string title) :
		mTexCoordsEnabled(0), mColorEnabled(0)
	{
		logMessage("Creating DirectX9 Rendersystem");
		d3d=Direct3DCreate9(D3D_SDK_VERSION);
		if (!d3d) throw "Unable to create Direct3D9 object!";
		
		
	}
	DirectX9RenderSystem::~DirectX9RenderSystem()
	{
		logMessage("Destroying DirectX9 Rendersystem");
		if (d3dDevice) d3dDevice->Release();
		if (d3d) d3d->Release();
		d3d=0; d3dDevice=0;
	}

	std::string DirectX9RenderSystem::getName()
	{
		return "DirectX9";
	}

	Texture* DirectX9RenderSystem::loadTexture(std::string filename,bool dynamic)
	{
		if (mDynamicLoading) dynamic=1;
		if (dynamic) rendersys->logMessage("creating dynamic DX9 texture '"+filename+"'");
		Directx9Texture* t=new Directx9Texture(filename,dynamic);
		if (!dynamic)
		{
			if (!t->load())
			{
				delete t;
				return 0;
			}
		}
		return t;
	}

	Texture* DirectX9RenderSystem::createTextureFromMemory(unsigned char* rgba,int w,int h)
	{
		
		rendersys->logMessage("creating user-defined DX9 texture");
		Directx9Texture* t=new Directx9Texture(rgba,w,h);
		return t;

	}

	void DirectX9RenderSystem::setTexture(Texture* t)
	{

	}

	void DirectX9RenderSystem::clear(bool color,bool depth)
	{
	}

	void DirectX9RenderSystem::_setModelviewMatrix(const gtypes::Matrix4& matrix)
	{

	}

	void DirectX9RenderSystem::_setProjectionMatrix(const gtypes::Matrix4& matrix)
	{
		
	}

	void DirectX9RenderSystem::setBlendMode(BlendMode mode)
	{
	}

	void DirectX9RenderSystem::render(RenderOp renderOp,TexturedVertex* v,int nVertices)
	{


	}

	void DirectX9RenderSystem::render(RenderOp renderOp,TexturedVertex* v,int nVertices,float r,float g,float b,float a)
	{

	}

	void DirectX9RenderSystem::render(RenderOp renderOp,PlainVertex* v,int nVertices)
	{
	
	}

	void DirectX9RenderSystem::render(RenderOp renderOp,PlainVertex* v,int nVertices,float r,float g,float b,float a)
	{

	}
	
	int DirectX9RenderSystem::getWindowWidth()
	{
		return 800;
	}
	
	int DirectX9RenderSystem::getWindowHeight()
	{
		return 600;
	}

	void DirectX9RenderSystem::render(RenderOp renderOp,ColoredVertex* v,int nVertices)
	{

	}

	void DirectX9RenderSystem::setAlphaMultiplier(float value)
	{
		mAlphaMultiplier=value;
	//	glColor4f(1,1,1,value);
	}

	void DirectX9RenderSystem::setWindowTitle(std::string title)
	{

	}
	
	gtypes::Vector2 DirectX9RenderSystem::getCursorPos()
	{
		return gtypes::Vector2(0,0);
	}

	void DirectX9RenderSystem::showSystemCursor(bool b)
	{

	}
	
	bool DirectX9RenderSystem::isSystemCursorShown()
	{

	}

	void DirectX9RenderSystem::presentFrame()
	{

	}
	
	void DirectX9RenderSystem::terminateMainLoop()
	{

	}
	
	void DirectX9RenderSystem::enterMainLoop()
	{

	}


	void createDX9RenderSystem(int w,int h,bool fullscreen,std::string title)
	{
		rendersys=new DirectX9RenderSystem(w,h,fullscreen,title);
	}

}

#endif
