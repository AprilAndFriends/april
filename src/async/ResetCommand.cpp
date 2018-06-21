/// @file
/// @version 5.1
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <gtypes/Rectangle.h>
#include <hltypes/hlog.h>

#include "RenderSystem.h"
#include "ResetCommand.h"
#include "Window.h"

namespace april
{
	ResetCommand::ResetCommand(const RenderState& state, cgvec2i windowSize) :
		StateUpdateCommand(state)
	{
		this->windowSize = windowSize;
	}
	
	void ResetCommand::execute()
	{
		april::rendersys->_deviceReset();
		april::rendersys->_deviceSetup();
		StateUpdateCommand::execute();
	}
	
}
