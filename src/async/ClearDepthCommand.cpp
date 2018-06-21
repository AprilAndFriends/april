/// @file
/// @version 5.1
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include "ClearDepthCommand.h"
#include "RenderSystem.h"

namespace april
{
	ClearDepthCommand::ClearDepthCommand(const RenderState& state) :
		RenderCommand(state)
	{
	}
	
	void ClearDepthCommand::execute()
	{
		RenderCommand::execute();
		april::rendersys->_deviceClearDepth();
	}
	
}
