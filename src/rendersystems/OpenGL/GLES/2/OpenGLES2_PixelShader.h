/// @file
/// @version 3.7
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Defines a OpenGLES 2 pixel shader.

#ifdef _OPENGLES2
#ifndef APRIL_OPENGLES2_PIXEL_SHADER_H
#define APRIL_OPENGLES2_PIXEL_SHADER_H

#include <hltypes/hplatform.h>
#include <hltypes/hstring.h>

#include "PixelShader.h"

namespace april
{
	class OpenGLES2_RenderSystem;
	
	class OpenGLES2_PixelShader : public PixelShader
	{
	public:
		friend class OpenGLES2_RenderSystem;

		OpenGLES2_PixelShader(chstr filename);
		OpenGLES2_PixelShader();
		~OpenGLES2_PixelShader();

		bool loadFile(chstr filename);
		bool loadResource(chstr filename);
		void setConstantsB(const int* quads, unsigned int quadCount);
		void setConstantsI(const int* quads, unsigned int quadCount);
		void setConstantsF(const float* quads, unsigned int quadCount);

	protected:
		unsigned int glShader;

		bool _compileShader(chstr filename, unsigned char* data, int size);

	};

}
#endif
#endif