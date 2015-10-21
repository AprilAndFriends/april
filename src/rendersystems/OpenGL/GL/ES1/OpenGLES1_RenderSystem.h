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
/// Defines an OpenGLES1 render system.

#ifdef _OPENGLES1
#ifndef APRIL_OPENGLES1_RENDER_SYSTEM_H
#define APRIL_OPENGLES1_RENDER_SYSTEM_H

#include <hltypes/hstring.h>

#include "OpenGLC_RenderSystem.h"

namespace april
{
	class OpenGLES1_Texture;
	class Texture;

	class OpenGLES1_RenderSystem : public OpenGLC_RenderSystem
	{
	public:
		friend class OpenGLES1_Texture;

		OpenGLES1_RenderSystem();
		~OpenGLES1_RenderSystem();

	protected:
		void _setupCaps();

		Texture* _createTexture(bool fromResource);

		void _setTextureBlendMode(BlendMode mode);

	};
	
}
#endif
#endif
