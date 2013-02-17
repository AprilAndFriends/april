/// @file
/// @author  Boris Mikic
/// @version 3.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

#include "UpdateDelegate.h"

namespace april
{
	UpdateDelegate::UpdateDelegate()
	{
	}

	UpdateDelegate::~UpdateDelegate()
	{
	}

	bool UpdateDelegate::updateRenderLoop(float timeSinceLastFrame)
	{
		return true;
	}
	
}
