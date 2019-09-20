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
/// Defines a custom command.

#ifndef APRIL_CUSTOM_COMMAND_H
#define APRIL_CUSTOM_COMMAND_H

#include <hltypes/harray.h>

#include "AsyncCommand.h"

namespace april
{
	class CustomCommand : public AsyncCommand
	{
	public:
		CustomCommand(void (*function)(const harray<void*>& args), const harray<void*>& args);

		void execute();

	protected:
		void (*function)(const harray<void*>& args);
		harray<void*> args;

	};
	
}
#endif
