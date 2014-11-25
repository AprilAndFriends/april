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
	/// @brief Defines a delegate for the mouse input callbacks.
	class aprilExport MouseDelegate
	{
	public:
		/// @brief Basic constructor.
		MouseDelegate();
		/// @brief Destructor.
		virtual ~MouseDelegate();

		/// @brief Called when a mouse button is pressed.
		/// @param[in] keyCode The key's number code.
		virtual void onMouseDown(april::Key keyCode);
		/// @brief Called when a mouse button is released.
		/// @param[in] keyCode The key's number code.
		virtual void onMouseUp(april::Key keyCode);
		/// @brief Called when a mouse button press is canceled without being released.
		/// @param[in] keyCode The key's number code.
		virtual void onMouseCancel(april::Key keyCode);
		/// @brief Called when the mouse pointer is moved.
		virtual void onMouseMove();
		/// @brief Called when a mouse scroll wheel is rotated.
		/// @param[in] x The scroll wheel value change on the x-axis.
		/// @param[in] y The scroll wheel value change on the y-axis.
		virtual void onMouseScroll(float x, float y);

	};

}
#endif
