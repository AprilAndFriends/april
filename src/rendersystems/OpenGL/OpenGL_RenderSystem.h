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
/// Defines a generic OpenGL render system.

#ifdef _OPENGL
#ifndef APRIL_OPENGL_RENDER_SYSTEM_H
#define APRIL_OPENGL_RENDER_SYSTEM_H

#define __HL_INCLUDE_PLATFORM_HEADERS
#include <hltypes/hplatform.h>

#if __APPLE__
	#include <TargetConditionals.h>
#endif
#ifdef _IOS
	#ifdef _OPENGLES1
		#include <OpenGLES/ES1/gl.h>
		#include <OpenGLES/ES1/glext.h>
	#elif defined(_OPENGLES2)
		#include <OpenGLES/ES2/gl.h>
		#include <OpenGLES/ES2/glext.h>
	#endif
#elif defined(_OPENGLES)
	#ifdef _OPENGLES1
		#include <GLES/gl.h>
		#ifdef _ANDROID
			#define GL_GLEXT_PROTOTYPES
			#include <GLES/glext.h>
		#endif
	#elif defined(_OPENGLES2)
		#include <GLES2/gl2.h>
		#ifdef _ANDROID
			#define GL_GLEXT_PROTOTYPES
			#include <GLES2/gl2ext.h>
		#endif
	#endif
#else
	#include <stdlib.h>
	#include <stdio.h>
	#include <string.h>
	#ifndef __APPLE__
		#include <gl/GL.h>
		#define GL_GLEXT_PROTOTYPES
		#include <gl/glext.h>
	#else
		#include <OpenGL/gl.h>
	#endif
#endif

#include <hltypes/hstring.h>
#include <gtypes/Rectangle.h>
#include <gtypes/Vector2.h>

#include "Color.h"
#include "OpenGL_State.h"
#include "RenderSystem.h"

namespace april
{
	class OpenGL_Texture;
	class Window;

	class OpenGL_RenderSystem : public RenderSystem
	{
	public:
		friend class OpenGL_Texture;

		OpenGL_RenderSystem();
		~OpenGL_RenderSystem();
		bool create(Options options);
		bool destroy();

		void reset();
		void assignWindow(Window* window);

		inline float getPixelOffset() { return 0.0f; }
		inline int getVRam() { return 0; }
		void setViewport(grect value);

		void clear(bool useColor = true, bool depth = false);
		void clear(bool depth, grect rect, Color color = Color::Clear);

		void bindTexture(unsigned int textureId);

		void setMatrixMode(unsigned int mode);
		void setTexture(Texture* texture);
		void setTextureBlendMode(BlendMode mode);
		/// @note The parameter factor is only used when the color mode is LERP.
		void setTextureColorMode(ColorMode textureColorMode, float factor = 1.0f);
		void setTextureFilter(Texture::Filter textureFilter);
		void setTextureAddressMode(Texture::AddressMode textureAddressMode);
		void setDepthBuffer(bool enabled, bool writeEnabled = true);

		Image::Format getNativeTextureFormat(Image::Format format);
		unsigned int getNativeColorUInt(const april::Color& color);
		Image* takeScreenshot(Image::Format format);

	protected:
		OpenGL_State deviceState;
		OpenGL_State currentState;
		OpenGL_Texture* activeTexture;

		virtual void _setupDefaultParameters();
		virtual void _applyStateChanges();
		void _setModelviewMatrix(const gmat4& matrix);
		void _setProjectionMatrix(const gmat4& matrix);

		void _setupCaps();
		void _setResolution(int w, int h, bool fullscreen);

		virtual void _setTextureBlendMode(BlendMode textureBlendMode);
		/// @note The parameter factor is only used when the color mode is LERP.
		virtual void _setTextureColorMode(ColorMode textureColorMode, float factor) = 0;
		virtual void _setTextureFilter(Texture::Filter textureFilter);
		virtual void _setTextureAddressMode(Texture::AddressMode textureAddressMode);
		virtual void _setDepthBuffer(bool enabled, bool writeEnabled = true);

		virtual void _loadIdentityMatrix() = 0;
		virtual void _setMatrixMode(unsigned int mode) = 0;
		virtual void _setVertexPointer(int stride, const void* pointer) = 0;
		virtual void _setTexCoordPointer(int stride, const void* pointer) = 0;
		virtual void _setColorPointer(int stride, const void* pointer) = 0;

#if defined(_WIN32) && !defined(_WINRT)
	public:
		inline HDC getHDC() { return this->hDC; }

	protected:
		HWND hWnd;
		HDC hDC;

		virtual void _releaseWindow();
		virtual bool _initWin32(Window* window);
#endif

		// translation from abstract render ops to gl's render ops
		static int glRenderOperations[];

	};
	
}
#endif
#endif
