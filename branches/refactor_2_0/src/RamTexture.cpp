/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @author  Ivan Vucica
/// @version 2.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <hltypes/hstring.h>

#include "april.h"
#include "Color.h"
#include "ImageSource.h"
#include "RamTexture.h"
#include "RenderSystem.h"

namespace april
{
	RamTexture::RamTexture(chstr filename, bool dynamic) : Texture()
	{
		this->filename = filename;
		this->dynamic = dynamic;
		this->source = NULL;
		if (!this->dynamic)
		{
			this->load();
		}
	}

	RamTexture::RamTexture(int w, int h) : Texture()
	{
		this->width = w;
		this->height = h;
		this->source = april::createEmptyImage(w, h);
		this->filename = "";
	}

	RamTexture::~RamTexture()
	{
		this->unload();
	}

	bool RamTexture::load()
	{
		if (this->source == NULL)
		{
			april::log("loading RAM texture '" + this->_getInternalName() + "'");
			this->source = april::loadImage(this->filename);
			this->width = this->source->w;
			this->height = this->source->h;
			this->bpp = this->source->bpp;
			return true;
		}
		return false;
	}
	
	void RamTexture::unload()
	{
		if (this->source != NULL)
		{
			april::log("unloading RAM texture '" + this->_getInternalName() + "'");
			delete this->source;
			this->source = NULL;
		}
	}
	
	bool RamTexture::isLoaded()
	{
		return (this->source != NULL);
	}
	
	Color RamTexture::getPixel(int x, int y)
	{
		if (this->source == NULL)
		{
			this->load();
		}
		this->unusedTime = 0.0f;
		return this->source->getPixel(x, y);
	}
	
	void RamTexture::setPixel(int x, int y, Color c)
	{
		if (this->source == NULL)
		{
			this->load();
		}
		this->unusedTime = 0.0f;
		this->source->setPixel(x, y, c);
	}
	
	Color RamTexture::getInterpolatedPixel(float x, float y)
	{
		if (this->source == NULL)
		{
			this->load();
		}
		this->unusedTime = 0.0f;
		return this->source->getInterpolatedPixel(x, y);
	}
	
}
