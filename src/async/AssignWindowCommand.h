/// @file
/// @version 5.1
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Defines a system assign window command.

#ifndef APRIL_ASSIGN_WINDOW_COMMAND_H
#define APRIL_ASSIGN_WINDOW_COMMAND_H

#include "AsyncCommand.h"

namespace april
{
	class Window;

	class AssignWindowCommand : public AsyncCommand
	{
	public:
		AssignWindowCommand(Window* window);

		bool isFinalizer() const { return true; }
		bool isSystemCommand() const { return true; }

		void execute();

	protected:
		Window* window;

	};

}
#endif
