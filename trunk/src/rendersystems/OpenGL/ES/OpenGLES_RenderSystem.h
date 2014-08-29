/// @file
/// @version 3.5
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

#include <hltypes/hplatform.h>
#if __APPLE__
	#include <TargetConditionals.h>
#endif
#if TARGET_OS_IPHONE
	#ifdef _OPENGLES1
		#include <OpenGLES/ES1/gl.h>
		#include <OpenGLES/ES1/glext.h>
	#elif defined(_OPENGLES2)
		#include <OpenGLES/ES2/gl.h>
		#include <OpenGLES/ES2/glext.h>
		extern GLint _positionSlot;
	#endif
#else
	#include <GLES/gl.h>
	#ifdef _ANDROID
		#define GL_GLEXT_PROTOTYPES
		#include <GLES/glext.h>
	#else
		#include <EGL/egl.h>
	#endif
#endif

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

	};
	
}
#endif
#endif
