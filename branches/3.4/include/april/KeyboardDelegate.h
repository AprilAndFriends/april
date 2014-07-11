/// @file
/// @version 3.4
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Defines a delegate for the keyboard input callbacks.

#ifndef APRIL_KEYBOARD_DELEGATE_H
#define APRIL_KEYBOARD_DELEGATE_H

#include "aprilExport.h"
#include "Keys.h"

namespace april
{
	class aprilExport KeyboardDelegate
	{
	public:
		KeyboardDelegate();
		virtual ~KeyboardDelegate();

		virtual void onKeyDown(april::Key keyCode);
		virtual void onKeyUp(april::Key keyCode);
		virtual void onChar(unsigned int charCode);

	};

}
#endif
