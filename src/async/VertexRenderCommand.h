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
/// Defines a generic vertex render command.

#ifndef APRIL_VERTEX_RENDER_COMMAND_H
#define APRIL_VERTEX_RENDER_COMMAND_H

#include <hltypes/harray.h>
#include <hltypes/hlog.h>

#include "RenderCommand.h"
#include "RenderState.h"

namespace april
{
	template <typename T>
	class VertexRenderCommand : public RenderCommand
	{
	public:
		VertexRenderCommand(const RenderState& state, const RenderOperation& renderOperation, const T* vertices, int count) : RenderCommand(state)
		{
			this->renderOperation = renderOperation;
			this->vertices.add(vertices, count);
		}

		void execute()
		{
			RenderCommand::execute();
			april::rendersys->_deviceRender(this->renderOperation, (T*)this->vertices, this->vertices.size());
		}

	protected:
		RenderOperation renderOperation;
		harray<T> vertices;

	};
	
}
#endif
