/// @file
/// @version 5.1
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include "DestroyWindowCommand.h"
#include "Window.h"

namespace april
{
	DestroyWindowCommand::DestroyWindowCommand() : AsyncCommand()
	{
	}

	void DestroyWindowCommand::execute()
	{
		april::window->_systemDestroy();
	}

}
