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
/// Defines a system set window resolution command.

#ifndef APRIL_SET_WINDOW_RESOLUTION_COMMAND_H
#define APRIL_SET_WINDOW_RESOLUTION_COMMAND_H

#include "AsyncCommand.h"

namespace april
{
	class SetWindowResolutionCommand : public AsyncCommand
	{
	public:
		SetWindowResolutionCommand(int width, int height, bool fullscreen);

		bool isSystemCommand() const { return true; }

		void execute();

	protected:
		int width;
		int height;
		bool fullscreen;

	};

}
#endif
