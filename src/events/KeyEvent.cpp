/// @file
/// @version 5.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include "KeyEvent.h"

namespace april
{
	HL_ENUM_CLASS_DEFINE(KeyEvent::Type,
	(
		HL_ENUM_DEFINE(KeyEvent::Type, Down);
		HL_ENUM_DEFINE(KeyEvent::Type, Up);
	));

	KeyEvent::KeyEvent()
	{
		this->type = Type::Up;
		this->keyCode = Key::None;
		this->charCode = 0;
	}

	KeyEvent::KeyEvent(Type type, Key keyCode, unsigned int charCode)
	{
		this->type = type;
		this->keyCode = keyCode;
		this->charCode = charCode;
	}

}
