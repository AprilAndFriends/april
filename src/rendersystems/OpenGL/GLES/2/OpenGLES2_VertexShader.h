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
/// Defines a OpenGLES 2 vertex shader.

#ifdef _OPENGLES2
#ifndef APRIL_OPENGLES2_VERTEX_SHADER_H
#define APRIL_OPENGLES2_VERTEX_SHADER_H

#include <hltypes/hplatform.h>
#include <hltypes/hstring.h>

#include "VertexShader.h"

namespace april
{
	class OpenGLES2_RenderSystem;
	
	class OpenGLES2_VertexShader : public VertexShader
	{
	public:
		friend class OpenGLES2_RenderSystem;

		OpenGLES2_VertexShader(chstr filename);
		OpenGLES2_VertexShader();
		~OpenGLES2_VertexShader();

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
