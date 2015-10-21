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
/// Defines an OpenGLES vertex shader.

#ifdef _OPENGLES
#ifndef APRIL_OPENGLES_VERTEX_SHADER_H
#define APRIL_OPENGLES_VERTEX_SHADER_H

#include <hltypes/hstring.h>

#include "VertexShader.h"

namespace april
{
	class OpenGLES_RenderSystem;

	class OpenGLES_VertexShader : public VertexShader
	{
	public:
		friend class OpenGLES_RenderSystem;

		OpenGLES_VertexShader();
		~OpenGLES_VertexShader();

		bool isLoaded();

		void setConstantsB(const int* quads, unsigned int quadCount);
		void setConstantsI(const int* quads, unsigned int quadCount);
		void setConstantsF(const float* quads, unsigned int quadCount);

	protected:
		unsigned int glShader;

		bool _createShader(chstr filename, const hstream& stream);

	};

}
#endif
#endif
