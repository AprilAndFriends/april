/// @file
/// @version 5.2
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include "TakeScreenshotCommand.h"
#include "RenderSystem.h"

namespace april
{
	TakeScreenshotCommand::TakeScreenshotCommand(const RenderState& state, Image::Format format, bool backBufferOnly) :
		StateUpdateCommand(state)
	{
		this->format = format;
		this->backBufferOnly = backBufferOnly;
	}
	
	void TakeScreenshotCommand::execute()
	{
		if (!this->backBufferOnly)
		{
			StateUpdateCommand::execute();
		}
		april::rendersys->_deviceTakeScreenshot(this->format, this->backBufferOnly);
	}
	
}
