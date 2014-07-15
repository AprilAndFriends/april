/// @file
/// @version 3.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Defines a delegate for the mouse input callbacks.

#ifndef APRIL_MOUSE_DELEGATE_H
#define APRIL_MOUSE_DELEGATE_H

#include "aprilExport.h"
#include "Keys.h"

namespace april
{
	class aprilExport MouseDelegate
	{
	public:
		MouseDelegate();
		virtual ~MouseDelegate();

		virtual void onMouseDown(april::Key keyCode);
		virtual void onMouseUp(april::Key keyCode);
		virtual void onMouseCancel(april::Key keyCode);
		virtual void onMouseMove();
		virtual void onMouseScroll(float x, float y);

	};

}
#endif
