/************************************************************************************\
This source file is part of the Awesome Portable Rendering Interface Library         *
For latest info, see http://libapril.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes (kreso@cateia.com)                                  *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#include <stdio.h>
#include <algorithm>
#include <IL/il.h>
#include "RenderSystem.h"
#include "RenderSystem_GL.h"
#include "ImageSource.h"

April::RenderSystem* rendersys;

namespace April
{	
	void april_writelog(std::string message)
	{
		printf("%s\n",message.c_str());		
	}
	
	void (*g_logFunction)(std::string)=april_writelog;
	
	int hexstr_to_int(std::string s)
	{
		int i;
		sscanf(s.c_str(),"%x",&i);
		return i;
	}
	
	void hexstr_to_argb(std::string& hex,unsigned char* a,unsigned char* r,unsigned char* g,unsigned char* b)
	{
		if (hex.substr(0,2) != "0x") hex="0x"+hex;
		if (hex.size() == 8)
		{
			*r=hexstr_to_int(hex.substr(2,2));
			*g=hexstr_to_int(hex.substr(4,2));
			*b=hexstr_to_int(hex.substr(6,2));
			*a=255;
		}
		else if (hex.size() == 10)
		{
			*r=hexstr_to_int(hex.substr(4,2));
			*g=hexstr_to_int(hex.substr(6,2));
			*b=hexstr_to_int(hex.substr(8,2));
			*a=hexstr_to_int(hex.substr(2,2));
		}
		else throw "Color format must be either 0xAARRGGBB or 0xRRGGBB";
	}
/*****************************************************************************************/	
	void PlainVertex::operator=(const gtypes::Vector3& v)
	{
		this->x=v.x;
		this->y=v.y;
		this->z=v.z;
	}
/*****************************************************************************************/
	Color::Color(float a,float r,float g,float b)
	{
		setColor(a,r,g,b);
	}

	Color::Color(unsigned int color)
	{
		setColor(color);
	}

	Color::Color(std::string hex)
	{
		setColor(hex);
	}

	Color::Color()
	{
		r=g=b=a=255;
	}

	void Color::setColor(float a,float r,float g,float b)
	{
		this->a=(unsigned char)(a*255); this->r=(unsigned char)(r*255);
		this->g=(unsigned char)(g*255); this->b=(unsigned char)(b*255);
	}

	void Color::setColor(unsigned int color)
	{
		this->a=color/256/256/256; this->r=color/256/256%256; this->g=color/256%256; this->b=color%256;
	}

	void Color::setColor(std::string hex)
	{
		// this is going to bite me in the arse on a little endian system...
		hexstr_to_argb(hex,&a,&r,&g,&b);
	}
/*****************************************************************************************/
	Texture::Texture()
	{
		mUnusedTimer=0;
	}

	Texture::~Texture()
	{
		std::vector<Texture*>::iterator it=mDynamicLinks.begin();
		for(;it!=mDynamicLinks.end();it++)
			(*it)->removeDynamicLink(this);
	}
	
	Color Texture::getPixel(int x,int y)
	{
		return Color(0,0,0,0);
	}
	
	Color Texture::getInterpolatedPixel(float x,float y)
	{
		return Color(0,0,0,0);
	}
	
	void Texture::update(float time_increase)
	{
		if (mDynamic && isLoaded())
		{
			float max_time=rendersys->getIdleTextureUnloadTime();
			if (max_time > 0)
			{
				if (mUnusedTimer > max_time) unload();
				mUnusedTimer+=time_increase;
			}
		}
	}
	
	void Texture::addDynamicLink(Texture* lnk)
	{
		if (find(mDynamicLinks.begin(),mDynamicLinks.end(),lnk) != mDynamicLinks.end()) return;
		mDynamicLinks.push_back(lnk);
		lnk->addDynamicLink(this);
	}
	
	void Texture::removeDynamicLink(Texture* lnk)
	{
		std::vector<Texture*>::iterator it=find(mDynamicLinks.begin(),mDynamicLinks.end(),lnk);
		if (it == mDynamicLinks.end()) return;
		mDynamicLinks.erase(it);
	}
	
	void  Texture::_resetUnusedTimer(bool recursive)
	{
		mUnusedTimer=0;
		if (recursive)
		{
			std::vector<Texture*>::iterator it=mDynamicLinks.begin();
			for(;it!=mDynamicLinks.end();it++)
				(*it)->_resetUnusedTimer(0);
		}
	}
/*****************************************************************************************/
	RAMTexture::RAMTexture(std::string filename,bool dynamic)
	{
		mFilename=filename;
		mBuffer=0;
		if (!dynamic) load();
	}


	RAMTexture::~RAMTexture()
	{
		unload();
	}
	
	void RAMTexture::load()
	{
		if (!mBuffer)
		{
			rendersys->logMessage("loading RAM texture '"+mFilename+"'");
			mBuffer=loadImage(mFilename);
			mWidth=mBuffer->w;
			mHeight=mBuffer->h;
		}
	}
	
	void RAMTexture::unload()
	{
		if (mBuffer)
		{
			rendersys->logMessage("unloading RAM texture '"+mFilename+"'");
			delete mBuffer;
			mBuffer=0;
		}
	}
	
	bool RAMTexture::isLoaded()
	{
		return mBuffer != 0;
	}
	
	Color RAMTexture::getPixel(int x,int y)
	{
		if (!mBuffer) load();
		mUnusedTimer=0;
		return mBuffer->getPixel(x,y);
	}
	
	Color RAMTexture::getInterpolatedPixel(float x,float y)
	{
		if (!mBuffer) load();
		mUnusedTimer=0;
		return mBuffer->getInterpolatedPixel(x,y);
	}
	
	int RAMTexture::getSizeInBytes()
	{
		return 0;
	}
/*****************************************************************************************/
	RenderSystem::RenderSystem()
	{
		mAlphaMultiplier=1.0f;
		mUpdateCallback=0;
		mMouseDownCallback=0;
		mMouseUpCallback=0;
		mMouseMoveCallback=0;
		mKeyDownCallback=0;
		mKeyUpCallback=0;
		mDynamicLoading=0;
		mIdleUnloadTime=0;
	}
	
	RenderSystem::~RenderSystem()
	{
		
	}

	void RenderSystem::drawColoredQuad(float x,float y,float w,float h,float r,float g,float b,float a)
	{
		PlainVertex v[4];
		v[0].x=x;   v[0].y=y;   v[0].z=0;
		v[1].x=x+w; v[1].y=y;   v[1].z=0;
		v[2].x=x;   v[2].y=y+h; v[2].z=0;
		v[3].x=x+w; v[3].y=y+h; v[3].z=0;
		
		render(TriangleStrip,v,4,r,g,b,a);
	}
	
	void RenderSystem::logMessage(std::string message,std::string prefix)
	{
		g_logFunction(prefix+message);
	}
	
	void setLogFunction(void (*fnptr)(std::string))
	{
		g_logFunction=fnptr;
	}
	
	void RenderSystem::registerUpdateCallback(bool (*callback)(float))
	{
		mUpdateCallback=callback;
	}

	void RenderSystem::registerMouseCallbacks(void (*mouse_dn)(float,float,int),
								              void (*mouse_up)(float,float,int),
								              void (*mouse_move)(float,float))
	{
			mMouseDownCallback=mouse_dn;
			mMouseUpCallback=mouse_up;
			mMouseMoveCallback=mouse_move;
			
	}
	void RenderSystem::registerKeyboardCallbacks(void (*key_dn)(unsigned int,unsigned int),
								                 void (*key_up)(unsigned int,unsigned int))
	{
		mKeyDownCallback=key_dn;
		mKeyUpCallback=key_up;
	}
	
	Texture* RenderSystem::loadRAMTexture(std::string filename,bool dynamic)
	{
		return new RAMTexture(filename,dynamic);
	}
/*********************************************************************************/
	void init(std::string rendersystem_name,int w,int h,bool fullscreen,std::string title)
	{
		ilInit();
		createGLRenderSystem(w,h,fullscreen,title);
	}
	
	void destroy()
	{
		destroyGLRenderSystem();
	}

}
