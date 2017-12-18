/// @file
/// @version 4.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <hltypes/harray.h>

#include "RenderCommand.h"
#include "RenderCommandQueue.h"

namespace april
{
	RenderCommandQueue::RenderCommandQueue()
	{
	}
	
	RenderCommandQueue::~RenderCommandQueue()
	{
		foreach (RenderCommand*, it, this->commands)
		{
			delete (*it);
		}
	}
	
}
