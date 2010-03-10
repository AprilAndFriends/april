/************************************************************************************
This source file is part of the Awesome Portable Rendering Interface Library
For latest info, see http://libatres.sourceforge.net/
*************************************************************************************
Copyright (c) 2010 Kresimir Spes (kreso@cateia.com)

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License (LGPL) as published by the
Free Software Foundation; either version 2 of the License, or (at your option)
any later version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.
*************************************************************************************/
#include <stdio.h>
#include "RenderSystem.h"
#include "RenderSystem_GL.h"
#include "ImageSource.h"
#include <IL/ilut.h>

April::RenderSystem* rendersys;

namespace April
{
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
	Color::Color(float r,float g,float b,float a)
	{
		this->r=r*255; this->g=g*255; this->b=b*255; this->a=a*255;
	}

	Color::Color()
	{
		r=g=b=a=1;
	}

	void Color::setHex(std::string hex)
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
	
	void RenderSystem::logMessage(std::string message)
	{
		printf("%s\n",message.c_str());
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
	//	ilutRenderer(ILUT_OPENGL);
		createGLRenderSystem(w,h,fullscreen,title);
	}
	
	void destroy()
	{
		destroyGLRenderSystem();
	}

}