/// @file
/// @version 5.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <hltypes/harray.h>

#include "AsyncCommandQueue.h"
#include "AsyncCommands.h"
#include "RenderState.h"

namespace april
{
	AsyncCommandQueue::AsyncCommandQueue() : repeatCount(0)
	{
	}
	
	AsyncCommandQueue::~AsyncCommandQueue()
	{
		foreach (AsyncCommand*, it, this->commands)
		{
			delete (*it);
		}
	}

	bool AsyncCommandQueue::isRepeatable() const
	{
		return (this->commands.size() > 0 && this->commands.last()->isRepeatable());
	}

	void AsyncCommandQueue::setupNextRepeat()
	{
		if (this->repeatCount < 1)
		{
			harray<Texture*> destroyedTextures;
			DestroyTextureCommand* destroyTextureCommand = NULL;
			RenderCommand* renderCommand = NULL;
			StateUpdateCommand* stateUpdateCommand = NULL;
			bool initialStateDefined = false;
			// first find all destroyed textures
			foreach (AsyncCommand*, it, this->commands)
			{
				destroyTextureCommand = dynamic_cast<DestroyTextureCommand*>(*it);
				if (destroyTextureCommand != NULL)
				{
					destroyedTextures += destroyTextureCommand->getTexture();
				}
				else if (!initialStateDefined)
				{
					stateUpdateCommand = dynamic_cast<StateUpdateCommand*>(*it);
					if (stateUpdateCommand != NULL)
					{
						stateUpdateCommand = NULL;
						initialStateDefined = true;
					}
					else
					{
						renderCommand = dynamic_cast<RenderCommand*>(*it);
						if (renderCommand != NULL)
						{
							stateUpdateCommand = new StateUpdateCommand(*renderCommand->getState());
							initialStateDefined = true;
						}
					}
				}
			}
			destroyedTextures.removeAll(NULL);
			destroyedTextures.removeDuplicates();
			harray<AsyncCommand*> oldCommands = this->commands;
			this->commands.clear();
			// make initial command doesn't use a destroyed texture
			RenderState* state = NULL;
			if (initialStateDefined && stateUpdateCommand != NULL)
			{
				state = stateUpdateCommand->getState();
				if (state->texture != NULL && destroyedTextures.has(state->texture))
				{
					state->texture = NULL;
				}
				this->commands += stateUpdateCommand;
			}
			// make sure no command actually uses the texture
			foreach (AsyncCommand*, it, oldCommands)
			{
				if ((*it)->isRepeatable())
				{
					renderCommand = dynamic_cast<RenderCommand*>(*it);
					if (renderCommand != NULL)
					{
						state = renderCommand->getState();
						if (state->texture != NULL && destroyedTextures.has(state->texture))
						{
							state->texture = NULL;
						}
					}
					this->commands += (*it);
				}
				else
				{
					delete (*it);
				}
			}
		}
		++this->repeatCount;
	}
	
}
