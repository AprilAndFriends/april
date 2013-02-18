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

#include "OpenGL_State.h"
#include "OpenGLES_RenderSystem.h"

namespace april
{
	class OpenGLES1_Texture;
	class Window;

	class OpenGLES1_RenderSystem : public OpenGLES_RenderSystem
	{
	public:
		friend class OpenGLES1_Texture;

		OpenGLES1_RenderSystem();
		~OpenGLES1_RenderSystem();

		void assignWindow(Window* window);
		
		// TODO - refactor
		int getMaxTextureSize();

	protected:
		Texture* _createTexture(chstr filename);
		Texture* _createTexture(int w, int h, unsigned char* rgba);
		Texture* _createTexture(int w, int h, Texture::Format format, Texture::Type type = Texture::TYPE_NORMAL, Color color = Color::Clear);

		void _setVertexPointer(int stride, const void* pointer);
		void _setTextureBlendMode(BlendMode mode);

#ifdef _WIN32
		EGLDisplay eglDisplay;
		EGLConfig eglConfig;
		EGLSurface eglSurface;
		EGLContext eglContext;
		EGLint pi32ConfigAttribs[128];

		void _releaseWindow();
#endif

	};
	
}

#endif
#endif
