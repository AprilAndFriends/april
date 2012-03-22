/// @file
/// @author  Boris Mikic
/// @version 2.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include <hltypes/harray.h>

#include "april.h"
#include "RenderSystem.h"
#include "Texture.h"
#include "TextureManager.h"

namespace april
{
	TextureManager::TextureManager()
	{
	}

	TextureManager::~TextureManager()
	{
		while (this->textures.size() > 0)
		{
			delete this->textures[0];
		}
	}

	void TextureManager::registerTexture(Texture* texture)
	{
		this->textures += texture;
	}

	void TextureManager::unregisterTexture(Texture* texture)
	{
		this->textures -= texture;
	}

	void TextureManager::unloadTextures()
	{
		foreach (Texture*, it, this->textures)
		{
			(*it)->unload();
		}
	}
	
}
