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
		if (this->commands.size() == 0)
		{
			return false;
		}
		AsyncCommand* command = this->commands.last();
		return (command->isFinalizer() && command->isRepeatable());
	}

	void AsyncCommandQueue::applyRepeatQueue(AsyncCommandQueue* other)
	{
		if (other->commands.size() > 0)
		{
			HL_LAMBDA_CLASS(_repeatableCommands, bool, ((AsyncCommand* const& command) { return command->isRepeatable(); }));
			harray<AsyncCommand*> repeatableCommands = other->commands.findAll(&_repeatableCommands::lambda);
			if (repeatableCommands.size() > 0)
			{
				RenderCommand* renderCommand = NULL;
				RenderState* state = NULL;
				if (this->commands.size() == 0)
				{
					foreach (AsyncCommand*, it, repeatableCommands)
					{
						if (dynamic_cast<StateUpdateCommand*>(*it) != NULL)
						{
							break;
						}
						renderCommand = dynamic_cast<RenderCommand*>(*it);
						if (renderCommand != NULL)
						{
							state = renderCommand->getState();
							state->viewportChanged = false;
							state->modelviewMatrixChanged = false;
							state->projectionMatrixChanged = false;
							this->commands += new StateUpdateCommand(*state);
							break;
						}
					}
				}
				this->commands += repeatableCommands;
				DestroyTextureCommand* destroyTextureCommand = dynamic_cast<DestroyTextureCommand*>(other->commands.last());
				other->commands -= repeatableCommands;
				if (destroyTextureCommand != NULL)
				{
					foreach (AsyncCommand*, it, this->commands)
					{
						renderCommand = dynamic_cast<RenderCommand*>(*it);
						if (renderCommand != NULL)
						{
							state = renderCommand->getState();
							if (state->texture != NULL && state->texture == destroyTextureCommand->getTexture())
							{
								state->texture = NULL;
							}
						}
					}
				}
				else if (dynamic_cast<PresentFrameCommand*>(this->commands.last()) != NULL)
				{
					++this->repeatCount;
				}
			}
		}
	}

	void AsyncCommandQueue::setupNextRepeat()
	{
		++this->repeatCount;
	}

	void AsyncCommandQueue::clearRepeat()
	{
		this->repeatCount = 0;
		foreach (AsyncCommand*, it, this->commands)
		{
			delete (*it);
		}
		this->commands.clear();
	}
	
}
