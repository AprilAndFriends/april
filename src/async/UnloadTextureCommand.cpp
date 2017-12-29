/// @file
/// @version 5.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include "Texture.h"
#include "UnloadTextureCommand.h"

namespace april
{
	UnloadTextureCommand::UnloadTextureCommand(Texture* texture) : AsyncCommand()
	{
		this->texture = texture;
	}

	void UnloadTextureCommand::execute()
	{
		this->texture->_deviceUnloadTexture();
	}

}
