/// @file
/// @author  Kresimir Spes
/// @version 1.31
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <memory.h>

#include <april/RenderSystem.h>
#include <gtypes/Matrix4.h>
#include <hltypes/hltypesUtil.h>

#include "BlurManager.h"

#define TEXTURE_SIZE 1024

namespace april
{
	BlurManager::BlurManager()
	{
		mTex1 = rendersys->createEmptyTexture(TEXTURE_SIZE, TEXTURE_SIZE, AT_XRGB, AT_RENDER_TARGET);
		mTex2 = rendersys->createEmptyTexture(TEXTURE_SIZE, TEXTURE_SIZE, AT_XRGB, AT_RENDER_TARGET);
		mNumLevels = 0;
		mEnable = true;
		mInc = 1.0f;
	}
	
	BlurManager::~BlurManager()
	{
		delete mTex1;
		delete mTex2;
	}

	void BlurManager::begin(int nLevels, float inc)
	{
		if (nLevels != mNumLevels && nLevels > 0)
		{	
			rendersys->setRenderTarget(mTex1);
			mEnable = true;
		}
		else
		{
			mEnable = false;
		}
		mInc = inc;
		mNumLevels = nLevels;
	}

	void BlurManager::end()
	{
		if (mEnable)
		{
			int i;
			float inc;
			april::ColoredTexturedVertex av[6 * 4];
			gtypes::Matrix4 mat = rendersys->getProjectionMatrix();
			rendersys->setOrthoProjection(gvec2(TEXTURE_SIZE, TEXTURE_SIZE));
			float k = 0.5f / TEXTURE_SIZE;
			av[0].x = 0;			av[0].y = 0;			av[0].u = k;		av[0].v = k;
			av[1].x = TEXTURE_SIZE;	av[1].y = 0;			av[1].u = 1 + k;	av[1].v = k;
			av[2].x = 0;			av[2].y = TEXTURE_SIZE;	av[2].u = k;		av[2].v = 1 + k;
			av[3].x = TEXTURE_SIZE;	av[3].y = 0;			av[3].u = 1 + k;	av[3].v = k;
			av[4].x = TEXTURE_SIZE;	av[4].y = TEXTURE_SIZE;	av[4].u = 1 + k;	av[4].v = 1 + k;
			av[5].x = 0;			av[5].y = TEXTURE_SIZE;	av[5].u = k;		av[5].v = 1 + k;
			av[0].color = av[1].color = av[2].color = av[3].color = av[4].color = av[5].color = 0x404040FF;
			memcpy(&av[ 6], &av[0], 6 * sizeof(april::ColoredTexturedVertex));
			memcpy(&av[12], &av[0], 6 * sizeof(april::ColoredTexturedVertex));
			memcpy(&av[18], &av[0], 6 * sizeof(april::ColoredTexturedVertex));
			
			rendersys->setBlendMode(april::ADD);
			for (int j = 0; j < mNumLevels; j++)
			{
				rendersys->setRenderTarget(mTex2);
				rendersys->setTexture(mTex1);
				rendersys->clear();
				hswap(mTex1, mTex2);

				inc = mInc;
				for (i =  0; i <  6; i++) av[i].v -= inc * k;
				for (i =  6; i < 12; i++) av[i].v += inc * k;
				for (i = 12; i < 18; i++) av[i].u -= inc * k;
				for (i = 18; i < 24; i++) av[i].u += inc * k;
				rendersys->render(april::TriangleList, av, 24);
			}
			rendersys->setBlendMode(april::ALPHA_BLEND);
			rendersys->setRenderTarget(0);
			rendersys->setProjectionMatrix(mat);
		}
	}
	
	void BlurManager::draw(int w, int h)
	{
		if (mNumLevels)
		{
			float k = 0.5f / TEXTURE_SIZE;
			rendersys->setTexture(mTex1);
			rendersys->drawTexturedQuad(grect(0.0f, 0.0f, (float)w, (float)h),grect(k, k, 1.0f, 1.0f));
		}
	}
}
