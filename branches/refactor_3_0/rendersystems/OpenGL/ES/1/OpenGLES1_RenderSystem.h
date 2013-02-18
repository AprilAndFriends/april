/// @file
/// @author  Kresimir Spes
/// @author  Ivan Vucica
/// @author  Boris Mikic
/// @version 3.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/// 
/// @section DESCRIPTION
/// 
/// Defines an OpenGLES1 render system.

#ifdef _OPENGLES1
#ifndef APRIL_OPENGLES1_RENDER_SYSTEM_H
#define APRIL_OPENGLES1_RENDER_SYSTEM_H

#ifdef __APPLE__
#include <TargetConditionals.h>
#endif

#include "OpenGL_State.h"
#include "OpenGLES_RenderSystem.h"

namespace april
{
	class Image;
	class OpenGLES1_Texture;
	class Window;

	class OpenGLES1_RenderSystem : public OpenGLES_RenderSystem
	{
	public:
		friend class OpenGLES1_Texture;

		OpenGLES1_RenderSystem();
		~OpenGLES1_RenderSystem();
		bool destroy();

		void assignWindow(Window* window);
		
		Texture* createTexture(int w, int h, unsigned char* rgba);
		Texture* createTexture(int w, int h, Texture::Format format, Texture::Type type = Texture::TYPE_NORMAL, Color color = Color::Clear);

		// TODO - refactor
		int _getMaxTextureSize();

	protected:
		void _setupDefaultParameters();
		Texture* _createTexture(chstr filename);

		void _setVertexPointer(int stride, const void* pointer);
		void _setTextureBlendMode(BlendMode mode);

#ifdef _WIN32
		void _releaseWindow();
#endif
		
	};
	
}

#endif
#endif
