/// @file
/// @author  Kresimir Spes
/// @author  Boris Mikic
/// @version 3.1
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/// 
/// @section DESCRIPTION
/// 
/// Defines an OpenGLES1 specific texture.

#ifdef _OPENGLES1
#ifndef APRIL_OPENGLES1_TEXTURE_H
#define APRIL_OPENGLES1_TEXTURE_H

#include <hltypes/hstring.h>

#include "Color.h"
#include "OpenGLES_Texture.h"

namespace april
{
	class OpenGLES1_Texture : public OpenGLES_Texture
	{
	public:
		OpenGLES1_Texture(chstr filename);
		OpenGLES1_Texture(int w, int h, unsigned char* rgba);
		OpenGLES1_Texture(int w, int h, Format format, Type type, Color color = Color::Clear);
		~OpenGLES1_Texture();

	};

}

#endif
#endif
