/// @file
/// @version 4.3
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
#elif defined(_ANDROID)
	#ifdef _OPENGLES1
		#include <GLES/gl.h>
		#define GL_GLEXT_PROTOTYPES
		#include <GLES/glext.h>
	#elif defined(_OPENGLES2)
		#include <GLES2/gl2.h>
		#define GL_GLEXT_PROTOTYPES
		#include <GLES2/gl2ext.h>
	#endif
#else
	#ifdef _OPENGLES1
		#include <GLES/gl.h>
	#elif defined(_OPENGLES2)
		#include <GLES2/gl2.h>
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
#endif

#ifdef _ANDROID
#define GL_ETCX_RGBA8_OES_HACK (GL_ETC1_RGB8_OES | (1u << 31))
#endif

#include <hltypes/hstring.h>
#include <gtypes/Rectangle.h>
#include <gtypes/Vector2.h>

#include "Color.h"
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

		int getVRam() const;

		Image::Format getNativeTextureFormat(Image::Format format) const;
		unsigned int getNativeColorUInt(const april::Color& color) const;
		Image* takeScreenshot(Image::Format format);

	protected:
		bool blendSeparationSupported;

		int deviceState_vertexStride;
		const void* deviceState_vertexPointer;
		int deviceState_textureStride;
		const void* deviceState_texturePointer;
		int deviceState_colorStride;
		const void* deviceState_colorPointer;

		void _deviceInit();
		bool _deviceCreate(Options options);
		bool _deviceDestroy();
		void _deviceAssignWindow(Window* window);
		void _deviceSetupCaps();
		void _deviceSetup();

		void _deviceChangeResolution(int w, int h, bool fullscreen);

		void _setDeviceViewport(const grect& rect);
		void _setDeviceDepthBuffer(bool enabled, bool writeEnabled);
		void _setDeviceRenderMode(bool useTexture, bool useColor);
		void _setDeviceTexture(Texture* texture);
		void _setDeviceTextureFilter(Texture::Filter textureFilter);
		void _setDeviceTextureAddressMode(Texture::AddressMode textureAddressMode);
		void _setDeviceBlendMode(BlendMode blendMode);

		void _deviceClear(bool depth);
		void _deviceClear(april::Color color, bool depth);
		void _deviceClearDepth();
		void _deviceRender(RenderOperation renderOperation, PlainVertex* vertices, int count);
		void _deviceRender(RenderOperation renderOperation, TexturedVertex* vertices, int count);
		void _deviceRender(RenderOperation renderOperation, ColoredVertex* vertices, int count);
		void _deviceRender(RenderOperation renderOperation, ColoredTexturedVertex* vertices, int count);

		void _setDeviceVertexPointer(int stride, const void* pointer, bool forceUpdate = false);
		void _setDeviceTexturePointer(int stride, const void* pointer, bool forceUpdate = false);
		void _setDeviceColorPointer(int stride, const void* pointer, bool forceUpdate = false);
		virtual void _setGlTextureEnabled(bool enabled) = 0;
		virtual void _setGlColorEnabled(bool enabled) = 0;
		virtual void _setGlVertexPointer(int stride, const void* pointer) = 0;
		virtual void _setGlTexturePointer(int stride, const void* pointer) = 0;
		virtual void _setGlColorPointer(int stride, const void* pointer) = 0;

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
		static int _glRenderOperations[];

	};
	
}
#endif
#endif
