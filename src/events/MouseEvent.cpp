/// @file
/// @version 5.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include "MouseEvent.h"

namespace april
{
	HL_ENUM_CLASS_DEFINE(MouseEvent::Type,
	(
		HL_ENUM_DEFINE(MouseEvent::Type, Down);
		HL_ENUM_DEFINE(MouseEvent::Type, Up);
		HL_ENUM_DEFINE(MouseEvent::Type, Cancel);
		HL_ENUM_DEFINE(MouseEvent::Type, Move);
		HL_ENUM_DEFINE(MouseEvent::Type, Scroll);
	));

	MouseEvent::MouseEvent(Type type, cgvec2f position, Key keyCode)
	{
		this->type = type;
		this->position = position;
		this->keyCode = keyCode;
	}

}
