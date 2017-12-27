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
		CreateWindowCommand(int w, int h, bool fullscreen, chstr title, Window::Options options);

		bool isSystemCommand() const { return true; }

		void execute();

	protected:
		int w;
		int h;
		bool fullscreen;
		hstr title;
		Window::Options options;

	};

}
#endif
