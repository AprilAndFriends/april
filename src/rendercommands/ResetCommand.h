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
/// Defines a reset command.

#ifndef APRIL_RESET_COMMAND_H
#define APRIL_RESET_COMMAND_H

#include <gtypes/Vector2.h>

#include "RenderCommand.h"

namespace april
{
	class ResetCommand : public RenderCommand
	{
	public:
		ResetCommand(const RenderState& state, cgvec2 windowSize);
		
		void execute();

	protected:
		gvec2 windowSize;

	};
	
}
#endif
