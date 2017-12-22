/// @file
/// @version 4.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <hltypes/hlog.h>

#include "ClearCommand.h"
#include "RenderSystem.h"

namespace april
{
	ClearCommand::ClearCommand(const RenderState& state, bool useDepth) : RenderCommand(state)
	{
		this->useDepth = useDepth;
	}
	
	void ClearCommand::execute()
	{
		RenderCommand::execute();
		april::rendersys->_deviceClear(this->useDepth);
	}
	
}
