/// @file
/// @version 3.4
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include "EventDelegate.h"

namespace april
{
	EventDelegate::EventDelegate() : InputDelegate(), UpdateDelegate(), SystemDelegate()
	{
	}

	EventDelegate::~EventDelegate()
	{
	}

}
