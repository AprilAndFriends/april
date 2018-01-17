/// @file
/// @version 5.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Defines a generic async command.

#ifndef APRIL_ASYNC_COMMAND_H
#define APRIL_ASYNC_COMMAND_H

namespace april
{
	class AsyncCommand
	{
	public:
		AsyncCommand();
		virtual ~AsyncCommand();

		virtual bool isFinalizer() const { return false; }
		virtual bool isUseState() const { return false; }
		virtual bool isSystemCommand() const { return false; }
		virtual bool isRepeatable() const { return false; }

		virtual void execute() = 0;

	};
	
}
#endif
