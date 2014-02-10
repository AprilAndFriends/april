/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @author  Ivan Vucica
/// @version 3.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <hltypes/hlog.h>
#include <hltypes/hstring.h>

#include "april.h"
#include "Color.h"
#include "Image.h"
#include "RamTexture.h"
#include "RenderSystem.h"

namespace april
{
	// TODO - this entire class needs to be removed
	RamTexture::RamTexture(chstr filename) : Texture()
	{
		this->filename = filename;
		this->source = NULL;
		hlog::write(april::logTag, "Creating RAM texture.");
	}

	RamTexture::RamTexture(int w, int h) : Texture()
	{
		this->width = w;
		this->height = h;
		this->source = Image::create(w, h);
		this->format = this->source->format;
	}

	RamTexture::~RamTexture()
	{
		this->unload();
	}

	bool RamTexture::load()
	{
		if (this->source == NULL)
		{
			hlog::writef(april::logTag, "Loading RAM texture '%s'.", this->_getInternalName().c_str());
			if (this->filename != "")
			{
				this->source = Image::load(this->filename);
				this->width = this->source->w;
				this->height = this->source->h;
			}
			else
			{
				this->source = Image::create(this->width, this->height);
			}
			this->format = this->source->format;
			return true;
		}
		return false;
	}
	
	void RamTexture::unload()
	{
		if (this->source != NULL)
		{
			hlog::writef(april::logTag, "Unloading RAM texture '%s'.", this->_getInternalName().c_str());
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
		return this->source->getPixel(x, y);
	}
	
	void RamTexture::setPixel(int x, int y, Color color)
	{
		if (this->source == NULL)
		{
			this->load();
		}
		this->source->setPixel(x, y, color);
	}

	bool RamTexture::_createInternalTexture(unsigned char* data, int size)
	{
		return true;
	}

	void RamTexture::_assignFormat()
	{
	}

	bool RamTexture::_uploadDataToGpu(int x, int y, int w, int h)
	{
		return true;
	}
	
}
