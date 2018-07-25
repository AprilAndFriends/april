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
/// Defines a present frame command.

#ifndef APRIL_PRESENT_FRAME_COMMAND_H
#define APRIL_PRESENT_FRAME_COMMAND_H

#include "RenderCommand.h"

namespace april
{
	class PresentFrameCommand : public RenderCommand
	{
	public:
		PresentFrameCommand(const RenderState& state, bool systemEnabled);

		bool isFinalizer() const { return true; }
		bool isRepeatable() const { return true; }
		
		void execute();

	protected:
		bool systemEnabled;

	};
	
}
#endif
