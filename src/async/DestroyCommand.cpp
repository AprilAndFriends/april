/// @file
/// @version 4.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include "DestroyCommand.h"
#include "RenderSystem.h"

namespace april
{
	DestroyCommand::DestroyCommand() : AsyncCommand()
	{
	}
	
	DestroyCommand::~DestroyCommand()
	{
	}

	void DestroyCommand::execute()
	{
		april::rendersys->_systemDestroy();
	}
	
}
