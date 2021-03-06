/// @file
/// @version 5.2
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <hltypes/hlog.h>
#include <hltypes/hstring.h>

#include "april.h"
#include "ControllerDelegate.h"

namespace april
{
	ControllerDelegate::ControllerDelegate()
	{
	}

	ControllerDelegate::~ControllerDelegate()
	{
	}

	void ControllerDelegate::onButtonDown(int controllerIndex, Button buttonCode)
	{
		hlog::debug(logTag, "Event onButtonDown() was not implemented.");
	}

	void ControllerDelegate::onButtonUp(int controllerIndex, Button buttonCode)
	{
		hlog::debug(logTag, "Event onButtonUp() was not implemented.");
	}
	
	void ControllerDelegate::onControllerAxisChange(int controllerIndex, april::Button buttonCode, float axisValue)
	{
		hlog::debug(logTag, "Event onControllerAxisChange() was not implemented.");
	}

	void ControllerDelegate::onControllerConnectionChanged(int controllerIndex, bool connected)
	{
		hlog::debug(logTag, "Event onControllerConnectionChanged() was not implemented.");
	}

}
