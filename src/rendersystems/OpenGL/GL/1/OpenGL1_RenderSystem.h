/// @file
/// @version 4.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Defines an OpenGL1 render system.

#ifdef _OPENGL1
#ifndef APRIL_OPENGL1_RENDER_SYSTEM_H
#define APRIL_OPENGL1_RENDER_SYSTEM_H

#include "OpenGLC_RenderSystem.h"

namespace april
{
	class OpenGL1_Texture;
	class Window;

	class OpenGL1_RenderSystem : public OpenGLC_RenderSystem
	{
	public:
		friend class OpenGL1_Texture;

		OpenGL1_RenderSystem();
		~OpenGL1_RenderSystem();

	protected:
		void _setupDefaultParameters();
		void _setupCaps();

		Texture* _deviceCreateTexture(bool fromResource);

		void _setTextureBlendMode(BlendMode mode);

#if defined(_WIN32) && !defined(_WINRT)
		HGLRC hRC;

		void _releaseWindow();
		bool _initWin32(Window* window);
#endif

	};
	
}

#endif
#endif
