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
/// Defines a system create command.

#ifndef APRIL_CREATE_COMMAND_H
#define APRIL_CREATE_COMMAND_H

#include "AsyncCommand.h"
#include "RenderSystem.h"

namespace april
{
	class CreateCommand : public AsyncCommand
	{
	public:
		CreateCommand(const RenderSystem::Options& options);

		bool isSystemCommand() const { return true; }

		void execute();

	protected:
		RenderSystem::Options options;

	};

}
#endif
