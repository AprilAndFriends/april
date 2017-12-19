/// @file
/// @version 4.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <hltypes/hlog.h>

#include "RenderCommand.h"
#include "RenderSystem.h"

namespace april
{
	RenderCommand::RenderCommand(const RenderState& state)
	{
		this->state = state;
	}
	
	RenderCommand::~RenderCommand()
	{
	}

	void RenderCommand::execute()
	{
		april::rendersys->_updateDeviceState(&this->state);
	}
	
}
