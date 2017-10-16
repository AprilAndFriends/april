/// @file
/// @version 4.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Defines the input mode.

#ifndef APRIL_INPUT_MODE_H
#define APRIL_INPUT_MODE_H

#include <hltypes/henum.h>

#include "aprilExport.h"

namespace april
{
	/// @class InputMode
	/// @brief Defines the input mode.
	HL_ENUM_CLASS_PREFIX_DECLARE(aprilExport, InputMode,
	(
		/// @var static const InputMode InputMode::Mouse
		/// @brief Using a mouse for input.
		HL_ENUM_DECLARE(InputMode, Mouse);
		/// @var static const InputMode InputMode::Touch
		/// @brief Using a touch-based interface for input.
		HL_ENUM_DECLARE(InputMode, Touch);
		/// @var static const InputMode InputMode::Controller
		/// @brief Using a controller for input.
		HL_ENUM_DECLARE(InputMode, Controller);
	));

}
#endif
