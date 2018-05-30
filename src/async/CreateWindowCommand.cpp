/// @file
/// @version 5.1
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include "CreateWindowCommand.h"
#include "Window.h"

namespace april
{
	CreateWindowCommand::CreateWindowCommand(int width, int height, bool fullscreen, chstr title, Window::Options options) : AsyncCommand()
	{
		this->width = width;
		this->height = height;
		this->fullscreen = fullscreen;
		this->title = title;
		this->options = options;
	}

	void CreateWindowCommand::execute()
	{
		april::window->_systemCreate(this->width, this->height, this->fullscreen, this->title, this->options);
	}

}
