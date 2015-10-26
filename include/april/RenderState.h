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
		/// @brief The current modelview matrix.
		gmat4 modelviewMatrix;
		/// @brief Whether the modelview matrix has changed.
		bool modelviewMatrixChanged;
		/// @brief The current projection matrix.
		gmat4 projectionMatrix;
		/// @brief Whether the projection matrix has changed.
		bool projectionMatrixChanged;
		/// @brief The current viewport.
		grect viewport;
		/// @brief Whether the viewport has changed.
		bool viewportChanged;
		/// @brief Whether texture coordinates are used in vertices.
		bool textureCoordinatesEnabled;
		/// @brief Whether color is used in vertices.
		bool colorEnabled;
		/// @brief The current unique texture ID.
		Texture* texture;
		/// @brief The current texture filter.
		Texture::Filter textureFilter;
		/// @brief The current texture address mode.
		Texture::AddressMode textureAddressMode;
		/// @brief The current system color.
		/// @note Usually used when clearing a buffer.
		Color systemColor;
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

		/// @brief The current projection matrix.
		/// @note This is a derived attribute that some systems require.
		gmat4 transformationMatrix;

		/// @brief Basic constructor.
		RenderState();
		/// @brief Destructor.
		virtual ~RenderState();
		
		/// @brief Resets or sets up the initial state.
		virtual void reset();

		/// @brief Updates the state and derived variables
		virtual void update();

	};
	
}
#endif
