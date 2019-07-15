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
/// Defines an OpenGLES 2 pixel shader.

#ifdef _OPENGLES2
#ifndef APRIL_OPENGLES2_PIXEL_SHADER_H
#define APRIL_OPENGLES2_PIXEL_SHADER_H

#include <hltypes/hstring.h>

#include "OpenGLES_PixelShader.h"

namespace april
{
	class OpenGLES2_RenderSystem;
	
	class OpenGLES2_PixelShader : public OpenGLES_PixelShader
	{
	public:
		friend class OpenGLES2_RenderSystem;

		OpenGLES2_PixelShader();

	};

}
#endif
#endif