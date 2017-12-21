/// @file
/// @version 4.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include "CreateWindowCommand.h"
#include "Window.h"

namespace april
{
	CreateWindowCommand::CreateWindowCommand(int w, int h, bool fullscreen, chstr title, Window::Options options) : AsyncCommand()
	{
		this->w = w;
		this->h = h;
		this->fullscreen = fullscreen;
		this->title = title;
		this->options = options;
	}

	CreateWindowCommand::~CreateWindowCommand()
	{
	}

	void CreateWindowCommand::execute()
	{
		april::window->_systemCreate(w, h, fullscreen, title, options);
	}

}
