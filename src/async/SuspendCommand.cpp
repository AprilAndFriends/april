/// @file
/// @version 5.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <hltypes/hlog.h>

#include "SuspendCommand.h"
#include "RenderSystem.h"

namespace april
{
	SuspendCommand::SuspendCommand() : AsyncCommand()
	{
	}
	
	void SuspendCommand::execute()
	{
		april::rendersys->_deviceSuspend();
	}
	
}
