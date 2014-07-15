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
/// Defines a rendersystem state.

#ifndef APRIL_RENDER_STATE_H
#define APRIL_RENDER_STATE_H

#include <gtypes/Matrix4.h>

#include "aprilUtil.h"
#include "Color.h"
#include "Texture.h"

namespace april
{
	class RenderState
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

		RenderState();
		virtual ~RenderState();
		
		void reset();

	};
	
}
#endif
