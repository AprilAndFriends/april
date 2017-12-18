/// @file
/// @version 4.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Defines a render command queue.

#ifndef APRIL_RENDER_COMMAND_QUEUE_H
#define APRIL_RENDER_COMMAND_QUEUE_H

#include <hltypes/harray.h>

#include "RenderCommand.h"

namespace april
{
	class RenderCommand;

	class RenderCommandQueue
	{
	public:
		harray<RenderCommand*> commands;

		RenderCommandQueue();
		~RenderCommandQueue();
		
	};
	
}
#endif
