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
/// Defines a system destroy command.

#ifndef APRIL_DESTROY_COMMAND_H
#define APRIL_DESTROY_COMMAND_H

#include "AsyncCommand.h"

namespace april
{
	class DestroyCommand : public AsyncCommand
	{
	public:
		DestroyCommand();

		bool isSystemCommand() const { return true; }

		void execute();

	};
	
}
#endif
