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
/// Defines a forced state update command.

#ifndef APRIL_STATE_UPDATE_COMMAND_H
#define APRIL_STATE_UPDATE_COMMAND_H

#include "AsyncCommand.h"
#include "RenderState.h"

namespace april
{
	class StateUpdateCommand : public AsyncCommand
	{
	public:
		StateUpdateCommand(const RenderState& state);

		bool isUseState() const { return true; }
		
		void execute();

	protected:
		RenderState state;

	};
	
}
#endif
