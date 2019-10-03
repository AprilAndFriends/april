/// @file
/// @version 5.2
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
		this->reset();
	}
	
	RenderState::~RenderState()
	{
	}
	
	void RenderState::reset()
	{
		this->viewport.set(0, 0, 1, 1);
		this->viewportChanged = true;
		this->modelviewMatrix.setIdentity();
		this->modelviewMatrixChanged = true;
		this->projectionMatrix.setIdentity();
		this->projectionMatrixChanged = true;
		this->depthBuffer = false;
		this->depthBufferWrite = false;
		this->useTexture = false;
		this->useColor = false;
		this->texture = NULL;
		this->blendMode = BlendMode::Alpha;
		this->colorMode = ColorMode::Multiply;
		this->colorModeFactor = 1.0f;
		this->systemColor = Color::White;
		this->renderTarget = NULL;
	}

}
