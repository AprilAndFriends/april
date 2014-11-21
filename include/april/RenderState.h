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
	/// @brief Defines a rendersystem state.
	class RenderState
	{
	public:
		/// @brief Whether texture coordinates are used in vertices.
		bool textureCoordinatesEnabled;
		/// @brief Whether color is used in vertices.
		bool colorEnabled;
		/// @brief The current unique texture ID.
		unsigned int textureId;
		/// @brief The current texture filter.
		Texture::Filter textureFilter;
		/// @brief The current texture address mode.
		Texture::AddressMode textureAddressMode;
		/// @brief The current system color.
		/// @note Usually used when clearing a buffer.
		Color systemColor;
		/// @brief Whether the modelview matrix has changed.
		bool modelviewMatrixChanged;
		/// @brief Whether the projection matrix has changed.
		bool projectionMatrixChanged;
		/// @brief The current modelview matrix.
		gmat4 modelviewMatrix;
		/// @brief The current projection matrix.
		gmat4 projectionMatrix;
		/// @brief The current texture blend mode.
		BlendMode blendMode;
		/// @brief The current texture color mode.
		ColorMode colorMode;
		/// @brief The current texture color mode factor.
		float colorModeFactor;
		/// @brief Whether the depth buffer is currently enabled.
		bool depthBuffer;
		/// @brief Whether writing to the depth buffer is currently enabled.
		bool depthBufferWrite;

		/// @brief Basic constructor.
		RenderState();
		/// @brief Destructor.
		virtual ~RenderState();
		
		/// @brief Resets or sets up the initial state.
		virtual void reset();

	};
	
}
#endif
