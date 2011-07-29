/************************************************************************************\
This source file is part of the Awesome Portable Rendering Interface Library         *
For latest info, see http://libapril.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes                                                     *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#if defined(HAVE_MARMELADE)
#ifndef APRIL_MARMELADE_TEXTURE_H
#define APRIL_MARMELADE_TEXTURE_H

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
