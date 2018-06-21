/// @file
/// @version 5.1
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include "RenderCommand.h"
#include "RenderSystem.h"

namespace april
{
	RenderCommand::RenderCommand(const RenderState& state) :
		AsyncCommand()
	{
		this->state = state;
	}
	
	void RenderCommand::execute()
	{
		april::rendersys->_updateDeviceState(&this->state);
	}
	
}
