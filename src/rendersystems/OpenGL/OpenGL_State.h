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
/// Defines an OpenGL rendersystem state.

#ifdef _OPENGL
#ifndef APRIL_OPENGL_STATE_H
#define APRIL_OPENGL_STATE_H

#include <gtypes/Matrix4.h>

#include "Color.h"
#include "RenderSystem.h"
#include "Texture.h"

namespace april
{
	class OpenGL_State
	{
	public:
		bool textureCoordinatesEnabled;
		bool colorEnabled;
		unsigned int textureId;
		Texture::Filter textureFilter;
		Texture::AddressMode textureAddressMode;
		Color systemColor;
		bool modelviewMatrixChanged;
		bool projectionMatrixChanged;
		gmat4 modelviewMatrix;
		gmat4 projectionMatrix;
		BlendMode blendMode;
		ColorMode colorMode;
		float colorModeFactor;
		unsigned int modeMatrix;
		int strideVertex;
		const void* pointerVertex;
		int strideTexCoord;
		const void* pointerTexCoord;
		int strideColor;
		const void* pointerColor;

		OpenGL_State();
		~OpenGL_State();
		
		void reset();

	};
	
}
#endif
#endif
