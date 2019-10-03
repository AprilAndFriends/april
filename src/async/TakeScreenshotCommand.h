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
/// Defines a take-screenshot command.

#ifndef APRIL_TAKE_SCREENSHOT_COMMAND_H
#define APRIL_TAKE_SCREENSHOT_COMMAND_H

#include "StateUpdateCommand.h"
#include "Image.h"

namespace april
{
	class TakeScreenshotCommand : public StateUpdateCommand
	{
	public:
		TakeScreenshotCommand(const RenderState& state, Image::Format format, bool backBufferOnly);
		
		void execute();

	protected:
		Image::Format format;
		bool backBufferOnly;

	};
	
}
#endif
