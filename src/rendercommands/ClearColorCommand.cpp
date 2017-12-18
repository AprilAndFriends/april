/// @file
/// @version 4.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include "ClearColorCommand.h"
#include "Color.h"
#include "RenderSystem.h"

namespace april
{
	ClearColorCommand::ClearColorCommand(const april::Color& color, bool useDepth) : ClearCommand(useDepth)
	{
		this->color = color;
	}
	
	void ClearColorCommand::execute()
	{
		april::rendersys->_deviceClear(this->color, this->useDepth);
	}

}
