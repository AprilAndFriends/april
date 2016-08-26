/// @file
/// @version 4.1
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
		/// @param[in] controllerIndex Index of the controller sending the event.
		/// @param[in] buttonCode The button's number code.
		virtual void onButtonDown(int controllerIndex, april::Button buttonCode);
		/// @brief Called when a button is released.
		/// @param[in] controllerIndex Index of the controller sending the event.
		/// @param[in] buttonCode The button's number code.
		virtual void onButtonUp(int controllerIndex, april::Button buttonCode);
		/// @brief Called when an axis' value changes.
		/// @param[in] controllerIndex Index of the controller sending the event.
		/// @param[in] buttonCode The axis' number code.
		/// @param[in] axisValue The new value of the axis.
		virtual void onControllerAxisChange(int controllerIndex, april::Button buttonCode, float axisValue);
		/// @brief Called when a controller is connected or disconnected.
		/// @param[in] controllerIndex Index of the controller sending the event.
		/// @param[in] connected The controller's current connected state.
		virtual void onControllerConnectionChanged(int controllerIndex, bool connected);

	};

}
#endif
