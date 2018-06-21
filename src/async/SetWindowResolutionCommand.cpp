/// @file
/// @version 5.1
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include "SetWindowResolutionCommand.h"
#include "Window.h"

namespace april
{
	SetWindowResolutionCommand::SetWindowResolutionCommand(int width, int height, bool fullscreen) :
		AsyncCommand()
	{
		this->width = width;
		this->height = height;
		this->fullscreen = fullscreen;
	}

	void SetWindowResolutionCommand::execute()
	{
		april::window->_systemSetResolution(this->width, this->height, this->fullscreen);
		april::window->queueSizeChange(april::window->getWidth(), april::window->getHeight(), april::window->isFullscreen());
	}

}
