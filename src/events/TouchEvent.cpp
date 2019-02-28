/// @file
/// @version 5.2
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include "TouchEvent.h"

namespace april
{
	HL_ENUM_CLASS_DEFINE(TouchEvent::Type,
	(
		HL_ENUM_DEFINE(TouchEvent::Type, Down);
		HL_ENUM_DEFINE(TouchEvent::Type, Up);
		HL_ENUM_DEFINE(TouchEvent::Type, Cancel);
		HL_ENUM_DEFINE(TouchEvent::Type, Move);
	));

	TouchEvent::TouchEvent(Type type, int index, cgvec2f position)
	{
		this->type = type;
		this->index = index;
		this->position = position;
	}

}
