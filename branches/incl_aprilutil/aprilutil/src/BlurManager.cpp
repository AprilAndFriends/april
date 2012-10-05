/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @version 2.0
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
		this->textures[0] = rendersys->createTexture(TEXTURE_SIZE, TEXTURE_SIZE, april::Texture::FORMAT_RGB, april::Texture::TYPE_RENDER_TARGET);
		this->textures[1] = rendersys->createTexture(TEXTURE_SIZE, TEXTURE_SIZE, april::Texture::FORMAT_RGB, april::Texture::TYPE_RENDER_TARGET);
		this->numLevels = 0;
		this->enabled = true;
		this->increment = 1.0f;
	}
	
	BlurManager::~BlurManager()
	{
		delete this->textures[0];
		delete this->textures[1];
	}

	void BlurManager::begin(int nLevels, float increment)
	{
		if (nLevels != this->numLevels && nLevels > 0)
		{	
			april::rendersys->setRenderTarget(this->textures[0]);
			this->enabled = true;
		}
		else
		{
			this->enabled = false;
		}
		this->increment = increment;
		this->numLevels = nLevels;
	}

	void BlurManager::end()
	{
		if (this->enabled)
		{
			int i;
			float increment;
			april::ColoredTexturedVertex av[6 * 4];
			gtypes::Matrix4 mat = rendersys->getProjectionMatrix();
			april::rendersys->setOrthoProjection(gvec2(TEXTURE_SIZE, TEXTURE_SIZE));
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
			
			april::rendersys->setTextureBlendMode(april::ADD);
			for_iter (j, 0, this->numLevels)
			{
				april::rendersys->setRenderTarget(this->textures[1]);
				april::rendersys->setTexture(this->textures[0]);
				april::rendersys->clear();
				hswap(this->textures[0], this->textures[1]);

				increment = this->increment;
				for_iterx (i, 0, 6)
				{
					av[i].v -= increment * k;
				}
				for_iterx (i, 6, 12)
				{
					av[i].v += increment * k;
				}
				for_iterx (i, 12, 18)
				{
					av[i].u -= increment * k;
				}
				for_iterx (i, 18, 24)
				{
					av[i].u += increment * k;
				}
				april::rendersys->render(april::TriangleList, av, 24);
			}
			april::rendersys->setTextureBlendMode(april::ALPHA_BLEND);
			april::rendersys->setRenderTarget(0);
			april::rendersys->setProjectionMatrix(mat);
		}
	}
	
	void BlurManager::draw(int w, int h)
	{
		if (this->numLevels)
		{
			float k = 0.5f / TEXTURE_SIZE;
			april::rendersys->setTexture(this->textures[0]);
			april::rendersys->drawTexturedRect(grect(0.0f, 0.0f, (float)w, (float)h), grect(k, k, 1.0f, 1.0f));
		}
	}
}
