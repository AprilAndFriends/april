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
		Color deviceState_color;
		unsigned int deviceState_matrixMode;

		void _deviceInit();
		void _deviceSetup();

		void _setDeviceModelviewMatrix(const gmat4& matrix);
		void _setDeviceProjectionMatrix(const gmat4& matrix);
		void _setDeviceDepthBuffer(bool enabled, bool writeEnabled);
		void _setDeviceColorMode(const ColorMode& colorMode, float colorModeFactor, bool useTexture, bool useColor, const Color& systemColor);
		void _setDeviceTexture(Texture* texture);

		void _setDeviceColor(const Color& color, bool forceUpdate = false);
		void _setDeviceMatrixMode(unsigned int mode, bool forceUpdate = false);
		void _setGlTextureEnabled(bool enabled);
		void _setGlColorEnabled(bool enabled);
		void _setGlVertexPointer(int stride, const void* pointer);
		void _setGlTexturePointer(int stride, const void* pointer);
		void _setGlColorPointer(int stride, const void* pointer);

	};
	
}
#endif
#endif
