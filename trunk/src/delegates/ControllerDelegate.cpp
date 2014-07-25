/// @file
/// @version 3.5
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

	void ControllerDelegate::onButtonDown(Button buttonCode)
	{
		hlog::debug(april::logTag, "Event onButtonDown() was not implemented.");
	}

	void ControllerDelegate::onButtonUp(Button buttonCode)
	{
		hlog::debug(april::logTag, "Event onButtonUp() was not implemented.");
	}
	
	void ControllerDelegate::onControllerAxisChange(april::Button buttonCode, float axisValue)
	{
		hlog::debug(april::logTag, "Event onControllerAxisChange() was not implemented.");
	}
}
