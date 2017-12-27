/// @file
/// @version 4.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include "UnassignWindowCommand.h"
#include "Window.h"

namespace april
{
	UnassignWindowCommand::UnassignWindowCommand() : AsyncCommand()
	{
	}

	void UnassignWindowCommand::execute()
	{
		april::window->_systemUnassign();
	}

}
