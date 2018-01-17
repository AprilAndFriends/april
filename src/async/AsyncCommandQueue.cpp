/// @file
/// @version 5.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <hltypes/harray.h>

#include "AsyncCommand.h"
#include "AsyncCommandQueue.h"

namespace april
{
	AsyncCommandQueue::AsyncCommandQueue()
	{
	}
	
	AsyncCommandQueue::~AsyncCommandQueue()
	{
		foreach (AsyncCommand*, it, this->commands)
		{
			delete (*it);
		}
	}

	bool AsyncCommandQueue::isRepeatable() const
	{
		return (this->commands.size() > 0 && this->commands.last()->isRepeatable());
	}
	
}
