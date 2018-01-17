/// @file
/// @version 5.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <hltypes/hlog.h>

#include "PresentFrameCommand.h"
#include "RenderSystem.h"

namespace april
{
	PresentFrameCommand::PresentFrameCommand(const RenderState& state, bool systemEnabled) : RenderCommand(state)
	{
		this->systemEnabled = systemEnabled;
	}
	
	void PresentFrameCommand::execute()
	{
		// TODO - removing texture usage due to an unusual crash on Mac (only when using "cageplayer") that would try to access potentially deleted textures
		this->state.useTexture = false;
		this->state.texture = NULL;
		RenderCommand::execute();
		april::rendersys->_devicePresentFrame(this->systemEnabled);
		april::rendersys->_updateDeviceState(&this->state, true);
	}
	
}
