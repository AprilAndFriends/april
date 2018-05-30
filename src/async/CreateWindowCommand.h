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
/// Defines a system create window command.

#ifndef APRIL_CREATE_WINDOW_COMMAND_H
#define APRIL_CREATE_WINDOW_COMMAND_H

#include "AsyncCommand.h"
#include "Window.h"

namespace april
{
	class CreateWindowCommand : public AsyncCommand
	{
	public:
		CreateWindowCommand(int width, int height, bool fullscreen, chstr title, Window::Options options);

		bool isSystemCommand() const { return true; }

		void execute();

	protected:
		int width;
		int height;
		bool fullscreen;
		hstr title;
		Window::Options options;

	};

}
#endif
