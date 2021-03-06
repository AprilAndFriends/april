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
/// Defines a generic render command.

#ifndef APRIL_RENDER_COMMAND_H
#define APRIL_RENDER_COMMAND_H

#include "AsyncCommand.h"
#include "RenderState.h"

namespace april
{
	class RenderCommand : public AsyncCommand
	{
	public:
		RenderCommand(const RenderState& state);

		RenderState* getState() { return &this->state; }
		bool isUseState() const { return true; }
		bool isRepeatable() const { return true; }
		
		void execute();

	protected:
		RenderState state;

	};
	
}
#endif
