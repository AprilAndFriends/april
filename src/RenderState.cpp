/// @file
/// @version 4.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <gtypes/Matrix4.h>

#include "Color.h"
#include "RenderState.h"
#include "RenderSystem.h"
#include "Texture.h"

namespace april
{
	RenderState::RenderState()
	{
	}
	
	RenderState::~RenderState()
	{
	}
	
	void RenderState::reset()
	{
		this->textureCoordinatesEnabled = false;
		this->colorEnabled = false;
		this->texture = NULL;
		this->textureFilter = Texture::FILTER_UNDEFINED;
		this->textureAddressMode = Texture::ADDRESS_UNDEFINED;
		this->systemColor = Color::Black;
		this->modelviewMatrixChanged = true;
		this->projectionMatrixChanged = true;
		this->projectionMatrix.setIdentity();
		this->modelviewMatrix.setIdentity();
		this->orthoProjection.set(0.0f, 0.0f, 1.0f, 1.0f);
		this->blendMode = BM_UNDEFINED;
		this->colorMode = CM_UNDEFINED;
		this->colorModeFactor = 1.0f;
		this->depthBuffer = false;
		this->depthBufferWrite = false;
		// derived attributes
		this->transformationMatrix.setIdentity();
	}

	void RenderState::update()
	{
		this->transformationMatrix = this->projectionMatrix * this->modelviewMatrix;
	}

}
