/// @file
/// @version 5.2
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include "TouchesEvent.h"

namespace april
{
	TouchesEvent::TouchesEvent(const harray<gvec2f>& touches)
	{
		this->touches = touches;
	}

}
