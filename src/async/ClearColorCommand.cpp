/// @file
/// @version 5.1
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <hltypes/hlog.h>

#include "ClearColorCommand.h"
#include "Color.h"
#include "RenderSystem.h"

namespace april
{
	ClearColorCommand::ClearColorCommand(const RenderState& state, const april::Color& color, bool useDepth) : RenderCommand(state)
	{
		this->color = color;
		this->useDepth = useDepth;
	}
	
	void ClearColorCommand::execute()
	{
		RenderCommand::execute();
		april::rendersys->_deviceClear(this->color, this->useDepth);
	}

}
