/// @file
/// @version 4.1
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

	void VirtualKeyboard::show()
	{
		if (!this->visible)
		{
			this->heightRatio = this->_show();
			if (this->heightRatio > 0.0f)
			{
				this->visible = true;
			}
		}
	}

	void VirtualKeyboard::hide()
	{
		if (this->visible && this->_hide())
		{
			this->visible = false;
		}
	}

	void VirtualKeyboard::draw()
	{
		this->_draw();
	}

}
