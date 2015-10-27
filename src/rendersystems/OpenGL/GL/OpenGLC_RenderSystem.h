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
		unsigned int deviceState_matrixMode;

		void _setupDefaultParameters();

		void _setDeviceMatrixMode(unsigned int mode);
		void _setDeviceModelviewMatrix(const gmat4& matrix);
		void _setDeviceProjectionMatrix(const gmat4& matrix);
		void _setDeviceDepthBuffer(bool enabled, bool writeEnabled);
		void _setDeviceColorMode(ColorMode colorMode, float colorModeFactor, bool useTexture, bool useColor, const Color& systemColor);

		void _deviceRender(RenderOperation renderOperation, PlainVertex* v, int nVertices);
		void _deviceRender(RenderOperation renderOperation, PlainVertex* v, int nVertices, Color color);
		void _deviceRender(RenderOperation renderOperation, TexturedVertex* v, int nVertices);
		void _deviceRender(RenderOperation renderOperation, TexturedVertex* v, int nVertices, Color color);
		void _deviceRender(RenderOperation renderOperation, ColoredVertex* v, int nVertices);
		void _deviceRender(RenderOperation renderOperation, ColoredTexturedVertex* v, int nVertices);




		//void _applyStateChanges();
		void _setClientState(unsigned int type, bool enabled);

		//void _setTextureColorMode(ColorMode textureColorMode, float factor);
		//void _setDepthBuffer(bool enabled, bool writeEnabled);

		//void _loadIdentityMatrix();
		//void _setMatrixMode(unsigned int mode);
		void _setVertexPointer(int stride, const void* pointer);
		void _setTexCoordPointer(int stride, const void *pointer);
		void _setColorPointer(int stride, const void *pointer);

	};
	
}
#endif
#endif
