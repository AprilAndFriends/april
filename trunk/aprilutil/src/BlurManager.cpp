/************************************************************************************\
This source file is part of the APRIL Utility library                                *
For latest info, see http://libapril.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes (kreso@cateia.com)                                  *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#include <april/RenderSystem.h>
#include <gtypes/Matrix4.h>
#include "BlurManager.h"
#include <memory.h>

namespace April
{
	BlurManager::BlurManager()
	{
		mTex1=rendersys->createEmptyTexture(1024,1024,AT_XRGB,AT_RENDER_TARGET);
		mTex2=rendersys->createEmptyTexture(1024,1024,AT_XRGB,AT_RENDER_TARGET);
		mNumLevels=0;
		mEnable=1;
		mInc=1;
	}
	
	BlurManager::~BlurManager()
	{
		delete mTex1;
		delete mTex2;
	}

	void BlurManager::begin(int nLevels,float inc)
	{
		if (nLevels != mNumLevels && nLevels > 0)
		{	
			rendersys->setRenderTarget(mTex1);
			mEnable=1;
		}
		else
		{
			mEnable=0;
		}
		mInc=inc;
		mNumLevels=nLevels;
	}

	void BlurManager::end()
	{
		if (mEnable)
		{	
			int i,j;
			float inc;
			April::Texture* temp;
			April::ColoredTexturedVertex av[6*4];
			gtypes::Matrix4 mat=rendersys->getProjectionMatrix();
			rendersys->setOrthoProjection(1024,1024);
			float k=0.5/1024.0f;
			av[0].x=0;    av[0].y=0;     av[0].u=k;   av[0].v=k;
			av[1].x=1024; av[1].y=0;     av[1].u=1+k; av[1].v=k;
			av[2].x=0;    av[2].y=1024;  av[2].u=k;   av[2].v=1+k;
			av[3].x=1024; av[3].y=0;     av[3].u=1+k; av[3].v=k;
			av[4].x=1024; av[4].y=1024;  av[4].u=1+k; av[4].v=1+k;
			av[5].x=0;    av[5].y=1024;  av[5].u=k;   av[5].v=1+k;
			av[0].color=av[1].color=av[2].color=av[3].color=av[4].color=av[5].color=0xFF404040;
			memcpy(&av[ 6],&av[0],6*sizeof(April::ColoredTexturedVertex));
			memcpy(&av[12],&av[0],6*sizeof(April::ColoredTexturedVertex));
			memcpy(&av[18],&av[0],6*sizeof(April::ColoredTexturedVertex));
			
			rendersys->setBlendMode(April::ADD);
			for (int j=0;j<mNumLevels;j++)
			{
				rendersys->setRenderTarget(mTex2);
				rendersys->setTexture(mTex1);
				rendersys->clear();
				temp=mTex1; mTex1=mTex2; mTex2=temp;

				inc=mInc;
				for (i= 0;i< 6;i++) av[i].v-=inc*k;
				for (i= 6;i<12;i++) av[i].v+=inc*k;
				for (i=12;i<18;i++) av[i].u-=inc*k;
				for (i=18;i<24;i++) av[i].u+=inc*k;
				rendersys->render(April::TriangleList,av,24);
			}
			rendersys->setBlendMode(April::ALPHA_BLEND);
			rendersys->setRenderTarget(0);
			rendersys->setProjectionMatrix(mat);
		}
	}
	
	void BlurManager::draw(int w,int h)
	{
		if (mNumLevels)
		{
			float k=0.5/1024.0f;
			rendersys->setTexture(mTex1);
			rendersys->drawTexturedQuad(0,0,w,h,k,k,1,1);
		}
	}
}
