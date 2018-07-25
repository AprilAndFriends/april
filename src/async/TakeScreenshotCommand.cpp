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
	TakeScreenshotCommand::TakeScreenshotCommand(Image::Format format) :
		AsyncCommand()
	{
		this->format = format;
	}
	
	void TakeScreenshotCommand::execute()
	{
		april::rendersys->_deviceTakeScreenshot(this->format);
	}
	
}
