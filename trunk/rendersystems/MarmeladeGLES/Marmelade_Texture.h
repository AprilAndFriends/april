/// @file
/// @author  Domagoj Cerjan
/// @version 1.31
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/// 
/// @section DESCRIPTION
/// 
/// Defines a Marmalade specific texture.

#if defined(HAVE_MARMELADE)
#ifndef APRIL_MARMALADE_TEXTURE_H
#define APRIL_MARMALADE_TEXTURE_H

#include <s3e.h>
#include <GLES/gl.h>
#include <GLES/egl.h>

#include "RenderSystem.h"

namespace april
{
	class Marmelade_Texture : public Texture
	{
	public:
		GLuint mTexId;
		
		Marmelade_Texture(chstr filename, bool dynamic);
		Marmelade_Texture(unsigned char* rgba, int w, int h);
		~Marmelade_Texture();
		
		bool load();
		void unload();
		bool isLoaded();
		int getSizeInBytes();
		
	};

}

#endif
#endif
