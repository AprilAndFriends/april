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
/// Defines a delegate for the controller input callbacks.

#ifndef APRIL_CONTROLLER_DELEGATE_H
#define APRIL_CONTROLLER_DELEGATE_H

#include "aprilExport.h"
#include "Keys.h"

namespace april
{
	class aprilExport ControllerDelegate
	{
	public:
		ControllerDelegate();
		virtual ~ControllerDelegate();

		virtual void onButtonDown(april::Button buttonCode);
		virtual void onButtonUp(april::Button buttonCode);
		virtual void onControllerAxisChange(april::Button buttonCode, float axisValue);
	};

}
#endif
