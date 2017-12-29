/// @file
/// @version 5.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include "DestroyTextureCommand.h"
#include "Texture.h"

namespace april
{
	DestroyTextureCommand::DestroyTextureCommand(Texture* texture) : AsyncCommand()
	{
		this->texture = texture;
	}

	DestroyTextureCommand::~DestroyTextureCommand()
	{
		this->execute(); // to prevent potential memory leaks
	}

	void DestroyTextureCommand::execute()
	{
		if (this->texture != NULL)
		{
			this->texture->_deviceUnloadTexture();
			this->texture->waitForAsyncLoad(); // waiting for all async stuff to finish
			delete this->texture;
			this->texture = NULL;
		}
	}

}
