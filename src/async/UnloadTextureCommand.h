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
/// Defines a unload texture command.

#ifndef APRIL_UNLOAD_TEXTURE_COMMAND_H
#define APRIL_UNLOAD_TEXTURE_COMMAND_H

#include "AsyncCommand.h"

namespace april
{
	class Texture;

	class UnloadTextureCommand : public AsyncCommand
	{
	public:
		UnloadTextureCommand(Texture* texture);

		bool isFinalizer() const { return true; }

		void execute();

	protected:
		Texture* texture;

	};

}
#endif
