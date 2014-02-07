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
/// Defines a generic OpenGLES texture.

#ifdef _OPENGLES
#ifndef APRIL_OPENGLES_TEXTURE_H
#define APRIL_OPENGLES_TEXTURE_H

#include <hltypes/hstring.h>

#include "Color.h"
#include "OpenGL_Texture.h"

namespace april
{
	class OpenGLES_Texture : public OpenGL_Texture
	{
	public:
		OpenGLES_Texture();
		~OpenGLES_Texture();

		bool copyPixelData(unsigned char** output, Image::Format format);

	};

}

#endif
#endif
