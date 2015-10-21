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
/// Defines a generic OpenGL render system.

#ifdef _OPENGLES
#ifndef APRIL_OPENGLES_RENDER_SYSTEM_H
#define APRIL_OPENGLES_RENDER_SYSTEM_H

#include "OpenGL_RenderSystem.h"

namespace april
{
	class OpenGLES_RenderSystem : public OpenGL_RenderSystem
	{
	public:
		OpenGLES_RenderSystem();
		~OpenGLES_RenderSystem();

		void assignWindow(Window* window);
		
	protected:
		void _setupCaps();
		
		void _setTextureBlendMode(BlendMode mode);
		void _setTextureColorMode(ColorMode textureColorMode, float factor);

		void _setupDefaultParameters();
		void _applyStateChanges();
		void _setClientState(unsigned int type, bool enabled);

		void _loadIdentityMatrix();
		void _setVertexPointer(int stride, const void* pointer);
		void _setTexCoordPointer(int stride, const void *pointer);
		void _setColorPointer(int stride, const void *pointer);

	};
	
}
#endif
#endif