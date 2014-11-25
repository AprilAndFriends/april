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
	/// @brief Defines a delegate for the controller input callbacks.
	class aprilExport ControllerDelegate
	{
	public:
		/// @brief Basic constructor.
		ControllerDelegate();
		/// @brief Destructor.
		virtual ~ControllerDelegate();

		/// @brief Called when a button is pressed.
		/// @param[in] buttonCode The button's number code.
		virtual void onButtonDown(april::Button buttonCode);
		/// @brief Called when a button is released.
		/// @param[in] buttonCode The button's number code.
		virtual void onButtonUp(april::Button buttonCode);
		/// @brief Called when an axis' value changes.
		/// @param[in] buttonCode The axis' number code.
		/// @param[in] axisValue The new value of the axis.
		virtual void onControllerAxisChange(april::Button buttonCode, float axisValue);

	};

}
#endif
