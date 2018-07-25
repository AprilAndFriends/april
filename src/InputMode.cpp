/// @file
/// @version 5.2
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include "InputMode.h"

namespace april
{
	HL_ENUM_CLASS_DEFINE(InputMode,
	(
		HL_ENUM_DEFINE(InputMode, Mouse);
		HL_ENUM_DEFINE(InputMode, Touch);
		HL_ENUM_DEFINE(InputMode, Controller);
	));

}
