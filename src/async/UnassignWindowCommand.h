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
/// Defines a system unassign window command.

#ifndef APRIL_UNASSIGN_WINDOW_COMMAND_H
#define APRIL_UNASSIGN_WINDOW_COMMAND_H

#include "AsyncCommand.h"

namespace april
{
	class UnassignWindowCommand : public AsyncCommand
	{
	public:
		UnassignWindowCommand();

		bool isSystemCommand() const { return true; }

		void execute();

	};

}
#endif
