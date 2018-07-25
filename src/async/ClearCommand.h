/// @file
/// @version 5.2
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Defines a clear command.

#ifndef APRIL_CLEAR_COMMAND_H
#define APRIL_CLEAR_COMMAND_H

#include "RenderCommand.h"

namespace april
{
	class ClearCommand : public RenderCommand
	{
	public:
		ClearCommand(const RenderState& state, bool useDepth);
		
		void execute();

	protected:
		bool useDepth;

	};
	
}
#endif
