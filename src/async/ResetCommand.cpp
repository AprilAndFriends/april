/// @file
/// @version 4.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <gtypes/Rectangle.h>
#include <hltypes/hlog.h>

#include "RenderSystem.h"
#include "ResetCommand.h"
#include "Window.h"

namespace april
{
	ResetCommand::ResetCommand(const RenderState& state, cgvec2 windowSize) : StateUpdateCommand(state)
	{
		this->windowSize = windowSize;
	}
	
	void ResetCommand::execute()
	{
		april::rendersys->_deviceReset();
		april::rendersys->_deviceSetup();
		if (april::rendersys->deviceState->texture != NULL)
		{
			april::rendersys->deviceState->texture->loadAsync();
			april::rendersys->deviceState->texture->ensureLoaded();
			april::rendersys->deviceState->texture->upload();
		}
		// TODOx - is this still needed?
		//april::rendersys->setViewport(grect(0.0f, 0.0f, this->windowSize));
		StateUpdateCommand::execute();
	}
	
}
