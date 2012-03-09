/// @file
/// @author  Boris Mikic
/// @version 1.5
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
		while (mTextures.size() > 0)
		{
			delete mTextures[0];
		}
	}

	void TextureManager::registerTexture(Texture* texture)
	{
		mTextures += texture;
	}

	void TextureManager::unregisterTexture(Texture* texture)
	{
		mTextures -= texture;
	}

	void TextureManager::unloadTextures()
	{
#ifdef _DEBUG
		april::log("unloading all textures");
#endif
		foreach (Texture*, it, mTextures)
		{
			(*it)->unload();
		}
	}
	
}
