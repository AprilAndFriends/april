/// @file
/// @version 4.5
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
		grect viewport;
		bool viewportChanged;
		gmat4 modelviewMatrix;
		bool modelviewMatrixChanged;
		gmat4 projectionMatrix;
		bool projectionMatrixChanged;
		bool depthBuffer;
		bool depthBufferWrite;
		bool useTexture;
		bool useColor;
		Texture* texture;
		BlendMode blendMode;
		ColorMode colorMode;
		float colorModeFactor;
		Color systemColor;

		RenderState();
		virtual ~RenderState();
		
		virtual void reset();

	};
	
}
#endif
