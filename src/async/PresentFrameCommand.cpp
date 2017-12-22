/// @file
/// @version 4.5
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
	PresentFrameCommand::PresentFrameCommand(const RenderState& state) : RenderCommand(state)
	{
	}
	
	void PresentFrameCommand::execute()
	{
		RenderCommand::execute();
		april::rendersys->_devicePresentFrame();
		april::rendersys->_updateDeviceState(&this->state, true);
	}
	
}
