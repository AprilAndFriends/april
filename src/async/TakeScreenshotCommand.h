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
/// Defines a take-screenshot command.

#ifndef APRIL_TAKE_SCREENSHOT_COMMAND_H
#define APRIL_TAKE_SCREENSHOT_COMMAND_H

#include "AsyncCommand.h"
#include "Image.h"

namespace april
{
	class TakeScreenshotCommand : public AsyncCommand
	{
	public:
		TakeScreenshotCommand(Image::Format format);
		
		void execute();

	protected:
		Image::Format format;

	};
	
}
#endif
