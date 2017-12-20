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
/// Defines a clear color command.

#ifndef APRIL_CLEAR_COLOR_COMMAND_H
#define APRIL_CLEAR_COLOR_COMMAND_H

#include "ClearCommand.h"
#include "Color.h"

namespace april
{
	class ClearColorCommand : public ClearCommand
	{
	public:
		ClearColorCommand(const RenderState& state, const april::Color& color, bool useDepth);
		
		void execute();

	protected:
		april::Color color;

	};
	
}
#endif
