/// @file
/// @version 4.3
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include "VirtualKeyboard.h"

namespace april
{
	VirtualKeyboard::VirtualKeyboard() : visible(false)
	{
	}

	VirtualKeyboard::~VirtualKeyboard()
	{
	}

	void VirtualKeyboard::showKeyboard(bool forced)
	{
		if (!this->visible)
		{
			this->heightRatio = this->_showKeyboard(forced);
			if (this->heightRatio > 0.0f)
			{
				this->visible = true;
			}
		}
	}

	void VirtualKeyboard::hideKeyboard(bool forced)
	{
		if (this->visible && this->_hideKeyboard(forced))
		{
			this->visible = false;
		}
	}

	void VirtualKeyboard::drawKeyboard()
	{
		this->_drawKeyboard();
	}

}
