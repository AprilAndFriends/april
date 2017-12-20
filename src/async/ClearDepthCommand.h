/// @file
/// @version 4.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Defines a clear command.

#ifndef APRIL_CLEAR_DEPTH_COMMAND_H
#define APRIL_CLEAR_DEPTH_COMMAND_H

#include "RenderCommand.h"

namespace april
{
	class ClearDepthCommand : public RenderCommand
	{
	public:
		ClearDepthCommand(const RenderState& state);
		
		void execute();

	};
	
}
#endif
