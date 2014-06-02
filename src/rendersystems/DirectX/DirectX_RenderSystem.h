/// @file
/// @version 3.4
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Defines a generic DirectX render system.

#ifdef _DIRECTX
#ifndef APRIL_DIRECTX_RENDER_SYSTEM_H
#define APRIL_DIRECTX_RENDER_SYSTEM_H

#include "RenderSystem.h"

namespace april
{
	class DirectX_RenderSystem : public RenderSystem
	{
	public:
		DirectX_RenderSystem();
		~DirectX_RenderSystem();

	};

}
#endif
#endif
