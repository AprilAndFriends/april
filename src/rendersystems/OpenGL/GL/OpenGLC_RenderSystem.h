/// @file
/// @version 3.6
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Defines a generic OpenGL "Classic" render system.

#if (defined(_OPENGL) || defined(_OPENGLES1)) && !defined(_OPENGLES)
#ifndef APRIL_OPENGLC_RENDER_SYSTEM_H
#define APRIL_OPENGLC_RENDER_SYSTEM_H

#include "OpenGL_RenderSystem.h"

namespace april
{
	class OpenGLC_Texture;

	class OpenGLC_RenderSystem : public OpenGL_RenderSystem
	{
	public:
		friend class OpenGLC_Texture;

		OpenGLC_RenderSystem();
		~OpenGLC_RenderSystem();

	protected:
		void _setupDefaultParameters();
		void _applyStateChanges();
		void _setClientState(unsigned int type, bool enabled);

		void _setTextureColorMode(ColorMode textureColorMode, float factor);
		void _setDepthBuffer(bool enabled, bool writeEnabled);

		void _loadIdentityMatrix();
		void _setVertexPointer(int stride, const void* pointer);
		void _setTexCoordPointer(int stride, const void *pointer);
		void _setColorPointer(int stride, const void *pointer);

	};
	
}
#endif
#endif