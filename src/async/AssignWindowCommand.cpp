/// @file
/// @version 5.1
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include "AssignWindowCommand.h"
#include "RenderSystem.h"

namespace april
{
	AssignWindowCommand::AssignWindowCommand(Window* window) : AsyncCommand()
	{
		this->window = window;
	}

	void AssignWindowCommand::execute()
	{
		april::rendersys->_systemAssignWindow(this->window);
	}

}
