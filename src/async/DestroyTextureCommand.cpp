/// @file
/// @version 5.1
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include "DestroyTextureCommand.h"
#include "Texture.h"

namespace april
{
	DestroyTextureCommand::DestroyTextureCommand(Texture* texture) : UnloadTextureCommand(texture), executed(false)
	{
	}

	DestroyTextureCommand::~DestroyTextureCommand()
	{
		this->execute(); // to prevent potential memory leaks
	}

	void DestroyTextureCommand::execute()
	{
		if (!this->executed)
		{
			UnloadTextureCommand::execute();
			this->texture->_ensureAsyncCompleted(); // waiting for all async stuff to finish
			delete this->texture;
			// don't set texture to NULL, because it can accessed outside and the memory location is important
		}
		this->executed = true;
	}

}
