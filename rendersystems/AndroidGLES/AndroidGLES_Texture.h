/************************************************************************************\
This source file is part of the Awesome Portable Rendering Interface Library         *
For latest info, see http://libapril.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2011 Domagoj Cerjan                                                    *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#ifndef APRIL_ANDROID_GLES_TEXTURE_H
#define APRIL_ANDROID_GLES_TEXTURE_H

#include <android/log.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include "RenderSystem.h"

namespace april
{
	class AndroidGLES_Texture : public Texture
	{
	public:
		GLuint mTexId;
		
		AndroidGLES_Texture(chstr filename, bool dynamic);
		AndroidGLES_Texture(unsigned char* rgba, int w, int h);
		~AndroidGLES_Texture();
		
		bool load();
		void unload();
		bool isLoaded();
		int getSizeInBytes();
		
	};

}

#endif
#endif
