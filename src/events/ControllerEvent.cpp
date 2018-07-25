/// @file
/// @version 5.2
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include "ControllerEvent.h"

namespace april
{
	HL_ENUM_CLASS_DEFINE(ControllerEvent::Type,
	(
		HL_ENUM_DEFINE(ControllerEvent::Type, Down);
		HL_ENUM_DEFINE(ControllerEvent::Type, Up);
		HL_ENUM_DEFINE(ControllerEvent::Type, Axis);
		HL_ENUM_DEFINE(ControllerEvent::Type, Connected);
		HL_ENUM_DEFINE(ControllerEvent::Type, Disconnected);
	));

	ControllerEvent::ControllerEvent(Type type, int controllerIndex, Button buttonCode, float axisValue)
	{
		this->type = type;
		this->controllerIndex = controllerIndex;
		this->buttonCode = buttonCode;
		this->axisValue = axisValue;
	}

}
