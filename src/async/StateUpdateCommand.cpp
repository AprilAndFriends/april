/// @file
/// @version 4.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include "RenderSystem.h"
#include "StateUpdateCommand.h"

namespace april
{
	StateUpdateCommand::StateUpdateCommand(const RenderState& state) : AsyncCommand()
	{
	}
	
	void StateUpdateCommand::execute()
	{
		april::rendersys->_updateDeviceState(&this->state, true);
	}
	
}
