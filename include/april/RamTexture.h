/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @version 3.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/// 
/// @section DESCRIPTION
/// 
/// Defines a special RAM texture.

#ifndef APRIL_RAM_TEXTURE_H
#define APRIL_RAM_TEXTURE_H

#include <hltypes/hstring.h>

#include "aprilExport.h"
#include "Color.h"
#include "Texture.h"

namespace april
{
	class Image;
	
	class aprilExport RamTexture : public Texture
	{
	public:
		RamTexture(chstr filename);
		RamTexture(int w, int h);
		~RamTexture();
		bool load();
		void unload();

		bool isLoaded();

		Color getPixel(int x, int y);
		void setPixel(int x, int y, Color c);
		
	protected:
		Image* source;
		
	};

}

#endif
