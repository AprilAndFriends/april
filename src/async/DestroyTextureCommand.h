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
/// Defines a destroy texture command.

#ifndef APRIL_DESTROY_TEXTURE_COMMAND_H
#define APRIL_DESTROY_TEXTURE_COMMAND_H

#include "AsyncCommand.h"

namespace april
{
	class Texture;

	class DestroyTextureCommand : public AsyncCommand
	{
	public:
		DestroyTextureCommand(Texture* texture);
		~DestroyTextureCommand();

		bool isFinalizer() const { return true; }
		bool isSystemCommand() const { return true; }

		void execute();

	protected:
		Texture* texture;

	};

}
#endif
