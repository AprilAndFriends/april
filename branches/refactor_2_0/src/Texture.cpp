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

#include <hltypes/harray.h>
#include <hltypes/hltypesUtil.h>
#include <hltypes/hstring.h>

#include "april.h"
#include "Color.h"
#include "ImageSource.h"
#include "Texture.h"
#include "TextureManager.h"
#include "RenderSystem.h"

namespace april
{
	Texture::Texture()
	{
		this->dynamic = false;
		this->filename = "";
		this->width = 0;
		this->height = 0;
		this->bpp = 3;
		this->unusedTimer = 0.0f;
		this->textureFilter = Linear;
		this->textureWrapping = true;
		april::rendersys->getTextureManager()->registerTexture(this);
	}

	Texture::~Texture()
	{
		foreach (Texture*, it, this->dynamicLinks)
		{
			(*it)->removeDynamicLink(this);
		}
		april::rendersys->getTextureManager()->unregisterTexture(this);
	}

	hstr Texture::_getInternalName()
	{
		return (this->filename != "" ? this->filename : "UserTexture");
	}
	
	void Texture::fillRect(grect rect, Color color)
	{
		this->fillRect((int)rect.x, (int)rect.y, (int)rect.w, (int)rect.h, color);
	}
	
	Color Texture::getInterpolatedPixel(float x, float y)
	{
		Color result;
		int x0 = (int)x;
		int y0 = (int)x;
		int x1 = x0 + 1;
		int y1 = y0 + 1;
		float rx0 = x - x0;
		float ry0 = y - y0;
		float rx1 = 1.0f - rx0;
		float ry1 = 1.0f - ry0;
		if (rx0 != 0.0f && ry0 != 0.0f)
		{
			Color tl = this->getPixel(x0, y0);
			Color tr = this->getPixel(x1, y0);
			Color bl = this->getPixel(x0, y1);
			Color br = this->getPixel(x1, y1);
			Color l = tl * ry1 + bl * ry0;
			Color r = tr * ry1 + br * ry0;
			result = l * rx1 + r * rx0;
			//result = (tl * ry1 + bl * ry0) * rx1 + (tr * ry1 + br * ry0) * rx0; // causes a weird conversion from "float" to "unsigned int" warning
		}
		else if (rx0 != 0.0f)
		{
			Color tl = this->getPixel(x0, y0);
			Color tr = this->getPixel(x1, y0);
			result = tl * rx1 + tr * rx0;
		}
		else if (ry0 != 0.0f)
		{
			Color tl = this->getPixel(x0, y0);
			Color bl = this->getPixel(x0, y1);
			result = tl * ry1 + bl * ry0;
		}
		else
		{
			result = this->getPixel(x0, y0);
		}
		return result;
	}
	
	void Texture::update(float k)
	{
		if (this->dynamic && this->isLoaded())
		{
			float max_time = april::rendersys->getIdleTextureUnloadTime();
			if (max_time > 0)
			{
				if (this->unusedTimer > max_time)
				{
					this->unload();
				}
				this->unusedTimer += k;
			}
		}
	}
	
	void Texture::addDynamicLink(Texture* link)
	{
		if (!this->dynamicLinks.contains(link))
		{
			this->dynamicLinks += link;
			link->addDynamicLink(this);
		}
	}
	
	void Texture::removeDynamicLink(Texture* link)
	{
		if (this->dynamicLinks.contains(link))
		{
			this->dynamicLinks -= link;
		}
	}
	
	void  Texture::_resetUnusedTimer(bool recursive)
	{
		this->unusedTimer = 0;
		if (recursive)
		{
			foreach (Texture*, it, this->dynamicLinks)
			{
				(*it)->_resetUnusedTimer(0);
			}
		}
	}

	void Texture::insertAsAlphaMap(Texture* texture, unsigned char median, int ambiguity)
	{
	}

/************************************************************************************/
	RAMTexture::RAMTexture(chstr filename, bool dynamic) : Texture()
	{
		this->filename = filename;
		this->buffer = NULL;
		if (!dynamic)
		{
			this->load();
		}
	}

	RAMTexture::RAMTexture(int w, int h) : Texture()
	{
		this->width = w;
		this->height = h;
		this->buffer = createEmptyImage(w, h);
		this->filename = "";
	}

	RAMTexture::~RAMTexture()
	{
		unload();
	}

	bool RAMTexture::load()
	{
		if (this->buffer == NULL)
		{
			april::log("loading RAM texture '" + this->_getInternalName() + "'");
			this->buffer = loadImage(this->filename);
			this->width = this->buffer->w;
			this->height = this->buffer->h;
			return true;
		}
		return false;
	}
	
	void RAMTexture::unload()
	{
		if (this->buffer != NULL)
		{
			april::log("unloading RAM texture '" + this->_getInternalName() + "'");
			delete this->buffer;
			this->buffer = NULL;
		}
	}
	
	bool RAMTexture::isLoaded()
	{
		return (this->buffer != NULL);
	}
	
	Color RAMTexture::getPixel(int x, int y)
	{
		if (this->buffer == NULL)
		{
			this->load();
		}
		this->unusedTimer = 0;
		return this->buffer->getPixel(x, y);
	}
	
	void RAMTexture::setPixel(int x, int y, Color c)
	{
		if (this->buffer == NULL)
		{
			this->load();
		}
		this->unusedTimer = 0;
		this->buffer->setPixel(x, y, c);
	}
	
	Color RAMTexture::getInterpolatedPixel(float x, float y)
	{
		if (this->buffer == NULL)
		{
			this->load();
		}
		this->unusedTimer = 0;
		return this->buffer->getInterpolatedPixel(x, y);
	}
	
	int RAMTexture::getSizeInBytes()
	{
		return (this->width * this->height * 3); // TODO - should use BPP instead of 3?
	}

	bool RAMTexture::isValid()
	{
		return (this->buffer != NULL);
	}

}
