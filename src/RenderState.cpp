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
		this->modelviewMatrixChanged = true;
		this->modelviewMatrix.setIdentity();
		this->projectionMatrix.setIdentity();
		this->projectionMatrixChanged = true;
		this->viewport.set(0.0f, 0.0f, 1.0f, 1.0f);
		this->viewportChanged = true;
		this->useTexture = false;
		this->useColor = false;
		this->texture = NULL;
		//this->textureFilter = Texture::FILTER_UNDEFINED;
		//this->textureAddressMode = Texture::ADDRESS_UNDEFINED;
		this->systemColor = Color::Black;
		this->blendMode = BM_ALPHA;
		this->colorMode = CM_MULTIPLY;
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
