/// @file
/// @version 5.0
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
		// TODO - removing texture usage due to an unusual crash on Mac (only when using "cageplayer") that would try to access potentially deleted textures
		this->state.useTexture = false;
		this->state.texture = NULL;
		RenderCommand::execute();
		april::rendersys->_deviceClear(this->useDepth);
	}
	
}
