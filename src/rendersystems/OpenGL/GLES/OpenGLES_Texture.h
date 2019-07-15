/// @file
/// @version 5.2
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Defines a generic OpenGLES texture.

#ifdef _OPENGLES
#ifndef APRIL_OPENGLES_TEXTURE_H
#define APRIL_OPENGLES_TEXTURE_H

#include "Application.h"
#include "OpenGL_Texture.h"

namespace april
{
	class OpenGLES_RenderSystem;

	class OpenGLES_Texture : public OpenGL_Texture
	{
	public:
		friend class OpenGLES_RenderSystem;

		OpenGLES_Texture(bool fromResource);

	protected:
		unsigned int framebufferId;
#ifdef __ANDROID__
		unsigned int alphaTextureId;
#endif

		bool _deviceCreateTexture(unsigned char* data, int size);
		bool _deviceDestroyTexture();

	};

}

#endif
#endif
