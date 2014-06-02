/// @file
/// @version 3.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include "InputDelegate.h"

namespace april
{
	InputDelegate::InputDelegate() : KeyboardDelegate(), MouseDelegate(), TouchDelegate(), ControllerDelegate()
	{
	}

	InputDelegate::~InputDelegate()
	{
	}

}
