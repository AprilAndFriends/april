/// @file
/// @version 3.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <hltypes/hlog.h>
#include <hltypes/hstring.h>

#include "april.h"
#include "UpdateDelegate.h"

namespace april
{
	UpdateDelegate::UpdateDelegate()
	{
	}

	UpdateDelegate::~UpdateDelegate()
	{
	}

	bool UpdateDelegate::onUpdate(float timeDelta)
	{
		hlog::debug(april::logTag, "Event onUpdate() was not implemented.");
		return false; // quits immediately
	}
	
}
