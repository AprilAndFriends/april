/// @file
/// @version 5.2
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Defines a delegate for all input callbacks.

#ifndef APRIL_INPUT_DELEGATE_H
#define APRIL_INPUT_DELEGATE_H

#include "aprilExport.h"
#include "ControllerDelegate.h"
#include "KeyDelegate.h"
#include "MotionDelegate.h"
#include "MouseDelegate.h"
#include "TouchDelegate.h"

namespace april
{
	/// @brief Defines a delegate for all input callbacks.
	class aprilExport InputDelegate : public KeyDelegate, public MouseDelegate, public TouchDelegate,
		public ControllerDelegate, public MotionDelegate
	{
	public:
		/// @brief Basic constructor.
		InputDelegate();

	};

}
#endif
