/// @file
/// @version 5.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Defines a suspend command.

#ifndef APRIL_SUSPEND_COMMAND_H
#define APRIL_SUSPEND_COMMAND_H

#include "RenderCommand.h"

namespace april
{
	class SuspendCommand : public RenderCommand
	{
	public:
		SuspendCommand(const RenderState& state);

		bool isFinalizer() const { return true; }
		bool isSystemCommand() const { return true; }

		void execute();

	};
	
}
#endif
