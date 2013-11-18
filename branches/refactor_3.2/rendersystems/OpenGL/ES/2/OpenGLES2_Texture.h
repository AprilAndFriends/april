/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @version 3.2
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/// 
/// @section DESCRIPTION
/// 
/// Defines an OpenGLES2 specific texture.

#ifdef _OPENGLES2
#ifndef APRIL_OPENGLES2_TEXTURE_H
#define APRIL_OPENGLES2_TEXTURE_H

#include <hltypes/hstring.h>

#include "Color.h"
#include "OpenGLES_Texture.h"

namespace april
{
	class OpenGLES2_Texture : public OpenGLES_Texture
	{
	public:
		OpenGLES2_Texture(chstr filename);
		OpenGLES2_Texture(int w, int h, unsigned char* rgba);
		OpenGLES2_Texture(int w, int h, Format format, Type type, Color color = Color::Clear);
		~OpenGLES2_Texture();

	};

}

#endif
#endif
