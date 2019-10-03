/// @file
/// @version 5.2
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

#include "OpenGL_RenderSystem.h"

namespace april
{
	class OpenGL1_Texture;
	class Window;

	class OpenGL1_RenderSystem : public OpenGL_RenderSystem
	{
	public:
		friend class OpenGL1_Texture;

		OpenGL1_RenderSystem();
		~OpenGL1_RenderSystem();

	protected:
		Color deviceState_color;
		unsigned int deviceState_matrixMode;

		void _deviceInit();
		void _deviceSetupCaps();
		void _deviceSetup();

		Texture* _deviceCreateTexture(bool fromResource);

		void _setDeviceModelviewMatrix(const gmat4& matrix);
		void _setDeviceProjectionMatrix(const gmat4& matrix);
		void _setDeviceDepthBuffer(bool enabled, bool writeEnabled);
		void _setDeviceTexture(Texture* texture);
		void _setDeviceBlendMode(const BlendMode& blendMode);
		void _setDeviceColorMode(const ColorMode& colorMode, float colorModeFactor, bool useTexture, bool useColor, const Color& systemColor);
		void _setDeviceRenderTarget(Texture* texture);

		void _setDeviceColor(const Color& color, bool forceUpdate = false);
		void _setDeviceMatrixMode(unsigned int mode, bool forceUpdate = false);
		void _setGlTextureEnabled(bool enabled);
		void _setGlColorEnabled(bool enabled);
		void _setGlVertexPointer(int stride, const void* pointer);
		void _setGlTexturePointer(int stride, const void* pointer);
		void _setGlColorPointer(int stride, const void* pointer);


#if defined(_WIN32) && !defined(_WINRT)
		HGLRC hRC;

		void _releaseWindow();
		bool _initWin32(Window* window);
#endif

	};
	
}

#endif
#endif
