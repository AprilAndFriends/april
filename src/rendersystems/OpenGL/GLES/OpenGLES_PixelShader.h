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
/// Defines an OpenGLES pixel shader.

#ifdef _OPENGLES
#ifndef APRIL_OPENGLES_PIXEL_SHADER_H
#define APRIL_OPENGLES_PIXEL_SHADER_H

#ifdef _IOS
	#ifdef _OPENGLES2
		#include <OpenGLES/ES2/gl.h>
		#include <OpenGLES/ES2/glext.h>
	#endif
#else
	#ifdef _OPENGLES2
		#include <GLES2/gl2.h>
		#ifdef _ANDROID
			#define GL_GLEXT_PROTOTYPES
			#include <GLES2/gl2ext.h>
		#endif
	#endif
#endif

#include <hltypes/hstream.h>
#include <hltypes/hstring.h>

#include "PixelShader.h"

namespace april
{
	class OpenGLES_RenderSystem;

	class OpenGLES_PixelShader : public PixelShader
	{
	public:
		friend class OpenGLES_RenderSystem;

		OpenGLES_PixelShader();
		~OpenGLES_PixelShader();

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