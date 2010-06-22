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
#ifdef _OPENGL
#include "RenderSystem_GL.h"
#else
#include "RenderSystem_DirectX9.h"
#endif
#include "ImageSource.h"

April::RenderSystem* rendersys;

namespace April
{	
	void april_writelog(chstr message)
	{
		printf("%s\n",message.c_str());		
	}
	
	void (*g_logFunction)(chstr)=april_writelog;
	
	int hexstr_to_int(chstr s)
	{
		int i;
		sscanf(s.c_str(),"%x",&i);
		return i;
	}
	
	void hexstr_to_argb(chstr hex,unsigned char* a,unsigned char* r,unsigned char* g,unsigned char* b)
	{
		hstr value = hex;
		if (value(0,2) != "0x") value="0x"+value;
		if (value.size() == 8)
		{
			*r=hexstr_to_int(value(2,2));
			*g=hexstr_to_int(value(4,2));
			*b=hexstr_to_int(value(6,2));
			*a=255;
		}
		else if (value.size() == 10)
		{
			*r=hexstr_to_int(value(4,2));
			*g=hexstr_to_int(value(6,2));
			*b=hexstr_to_int(value(8,2));
			*a=hexstr_to_int(value(2,2));
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

	Color::Color(chstr hex)
	{
		setColor(hex);
	}

	Color::Color()
	{
		a=r=g=b=255;
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

	void Color::setColor(chstr hex)
	{
		// this is going to bite me in the arse on a little endian system...
		hexstr_to_argb(hex,&a,&r,&g,&b);
	}
/*****************************************************************************************/
	Texture::Texture()
	{
		mFilename="";
		mUnusedTimer=0;
	}

	Texture::~Texture()
	{
		for (Texture** it=mDynamicLinks.iter();it;it=mDynamicLinks.next())
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
		if (mDynamicLinks.contains(lnk)) return;
		mDynamicLinks+=lnk;
		lnk->addDynamicLink(this);
	}
	
	void Texture::removeDynamicLink(Texture* lnk)
	{
		if (mDynamicLinks.contains(lnk))
			mDynamicLinks-=lnk;
	}
	
	void  Texture::_resetUnusedTimer(bool recursive)
	{
		mUnusedTimer=0;
		if (recursive)
		{
			for (Texture** it=mDynamicLinks.iter();it;it=mDynamicLinks.next())
				(*it)->_resetUnusedTimer(0);
		}
	}
/*****************************************************************************************/
	RAMTexture::RAMTexture(chstr filename,bool dynamic)
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
		mCharCallback=0;
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
	
	void RenderSystem::logMessage(chstr message,chstr prefix)
	{
		g_logFunction(prefix+message);
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
	void RenderSystem::registerKeyboardCallbacks(void (*key_dn)(unsigned int),
								                 void (*key_up)(unsigned int),
												 void (*char_callback)(unsigned int))
	{
		mKeyDownCallback=key_dn;
		mKeyUpCallback=key_up;
		mCharCallback=char_callback;
	}
	
	Texture* RenderSystem::loadRAMTexture(chstr filename,bool dynamic)
	{
		return new RAMTexture(filename,dynamic);
	}
	
	void RenderSystem::setIdentityTransform()
	{
		mModelviewMatrix.setIdentity();
		_setModelviewMatrix(mModelviewMatrix);
	}
	
	void RenderSystem::translate(float x,float y,float z)
	{
		mModelviewMatrix.translate(x,y,z);
		_setModelviewMatrix(mModelviewMatrix);
	}
	
	void RenderSystem::rotate(float angle,float ax,float ay,float az)
	{
		mModelviewMatrix.rotate(ax,ay,az,angle);
		_setModelviewMatrix(mModelviewMatrix);
	}
	
	void RenderSystem::scale(float s)
	{
		mModelviewMatrix.scale(s);
		_setModelviewMatrix(mModelviewMatrix);
	}
	
	void RenderSystem::scale(float sx,float sy,float sz)
	{
		mModelviewMatrix.scale(sx,sy,sz);
		_setModelviewMatrix(mModelviewMatrix);
	}
	
	void RenderSystem::lookAt(const gtypes::Vector3 &eye, const gtypes::Vector3 &direction, const gtypes::Vector3 &up)
	{
		mModelviewMatrix.lookAt(eye,direction,up);
		_setModelviewMatrix(mModelviewMatrix);
	}
		
	void RenderSystem::setOrthoProjection(float w,float h,float x_offset,float y_offset)
	{
		mProjectionMatrix.ortho(w,h,x_offset,y_offset);
		_setProjectionMatrix(mProjectionMatrix);
	}
	
	void RenderSystem::setPerspective(float fov, float aspect, float near, float far)
	{
		mProjectionMatrix.perspective(fov,aspect,near,far);
		_setProjectionMatrix(mProjectionMatrix);
	}
	
	void RenderSystem::setModelviewMatrix(const gtypes::Matrix4& matrix)
	{
		mModelviewMatrix=matrix;
		_setModelviewMatrix(matrix);
	}
	void RenderSystem::setProjectionMatrix(const gtypes::Matrix4& matrix)
	{
		mProjectionMatrix=matrix;
		_setProjectionMatrix(matrix);
	}
	
	const gtypes::Matrix4& RenderSystem::getModelviewMatrix()
	{
		return mModelviewMatrix;
	}
	
	const gtypes::Matrix4& RenderSystem::getProjectionMatrix()
	{
		return mProjectionMatrix;
	}
/*********************************************************************************/
	void init(chstr rendersystem_name,int w,int h,bool fullscreen,chstr title)
	{
		ilInit();
		#ifdef _OPENGL
			createGLRenderSystem(w,h,fullscreen,title);
		#else
			createDX9RenderSystem(w,h,fullscreen,title);
		#endif
	}
	
	void setLogFunction(void (*fnptr)(chstr))
	{
		g_logFunction=fnptr;
	}
	
	void destroy()
	{
		delete rendersys;
	}

}
