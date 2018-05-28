/// @file
/// @version 5.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include "TouchEvent.h"

namespace april
{
	TouchEvent::TouchEvent(const harray<gvec2f>& touches)
	{
		this->touches = touches;
	}

}
