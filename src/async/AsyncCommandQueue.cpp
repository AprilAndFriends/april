/// @file
/// @version 5.2
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
	AsyncCommandQueue::AsyncCommandQueue() :
		repeatCount(0)
	{
	}
	
	AsyncCommandQueue::~AsyncCommandQueue()
	{
		foreach (AsyncCommand*, it, this->commands)
		{
			delete (*it);
		}
		foreach (UnloadTextureCommand*, it, this->unloadTextureCommands)
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

	bool AsyncCommandQueue::hasCommands() const
	{
		return (this->commands.size() > 0 || this->unloadTextureCommands.size() > 0);
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
				if (this->commands.size() == 0)
				{
					// adding a custom state command to make sure the proper device state is applied
					foreach (AsyncCommand*, it, repeatableCommands)
					{
						if (dynamic_cast<StateUpdateCommand*>(*it) != NULL)
						{
							break;
						}
						renderCommand = dynamic_cast<RenderCommand*>(*it);
						if (renderCommand != NULL)
						{
							this->commands += new StateUpdateCommand(*renderCommand->getState());
							break;
						}
					}
				}
				this->commands += repeatableCommands;
				other->commands -= repeatableCommands;
				this->unloadTextureCommands = other->unloadTextureCommands;
				other->unloadTextureCommands.clear();
				if (dynamic_cast<PresentFrameCommand*>(this->commands.last()) != NULL)
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
		foreach (UnloadTextureCommand*, it, this->unloadTextureCommands)
		{
			(*it)->execute();
			delete (*it);
		}
		this->unloadTextureCommands.clear();
	}
	
}
