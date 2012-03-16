/// @file
/// @author  Boris Mikic
/// @version 1.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/// 
/// @section DESCRIPTION
/// 
/// Represents a global manager for textures.

#ifndef APRIL_TEXTURE_MANAGER_H
#define APRIL_TEXTURE_MANAGER_H

#include <hltypes/harray.h>

#include "aprilExport.h"

namespace april
{
	class Texture;

	class aprilExport TextureManager
	{
	public:
		TextureManager();
		~TextureManager();

		harray<Texture*> getTextures() { return mTextures; }

		void registerTexture(Texture* texture);
		void unregisterTexture(Texture* texture);
		void unloadTextures();
		
	protected:
		harray<Texture*> mTextures;
		
	};
	
}
#endif
