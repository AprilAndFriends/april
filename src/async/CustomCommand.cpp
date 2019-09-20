/// @file
/// @version 5.2
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include "CustomCommand.h"

namespace april
{
	CustomCommand::CustomCommand(void (*function)(const harray<void*>& args), const harray<void*>& args) :
		AsyncCommand()
	{
		this->function = function;
		this->args = args;
	}
	
	void CustomCommand::execute()
	{
		(*this->function)(this->args);
	}
	
}
