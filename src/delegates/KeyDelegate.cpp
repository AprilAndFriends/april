/// @file
/// @version 5.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <hltypes/hlog.h>
#include <hltypes/hstring.h>

#include "april.h"
#include "KeyDelegate.h"

namespace april
{
	KeyDelegate::KeyDelegate()
	{
	}

	KeyDelegate::~KeyDelegate()
	{
	}

	void KeyDelegate::onKeyDown(Key keyCode)
	{
		hlog::debug(logTag, "Event onKeyDown() was not implemented.");
	}

	void KeyDelegate::onKeyUp(Key keyCode)
	{
		hlog::debug(logTag, "Event onKeyUp() was not implemented.");
	}

	void KeyDelegate::onChar(unsigned int charCode)
	{
		hlog::debug(logTag, "Event onChar() was not implemented.");
	}

}
