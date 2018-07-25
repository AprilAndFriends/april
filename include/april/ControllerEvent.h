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
/// Defines a controller event.

#ifndef APRIL_CONTROLLER_EVENT_H
#define APRIL_CONTROLLER_EVENT_H

#include <hltypes/henum.h>

#include "aprilExport.h"
#include "Keys.h"

namespace april
{
	/// @brief Defines controller input event data.
	class aprilExport ControllerEvent
	{
	public:
		/// @class Type
		/// @brief Defines controller input event types.
		HL_ENUM_CLASS_PREFIX_DECLARE(aprilExport, Type,
		(
			/// @var static const Type Type::Down
			/// @brief Controller button was pressed.
			HL_ENUM_DECLARE(Type, Down);
			/// @var static const Type Type::Up
			/// @brief Controller button was released.
			HL_ENUM_DECLARE(Type, Up);
			/// @var static const Type Type::Axis
			/// @brief Controller axis position was changed.
			HL_ENUM_DECLARE(Type, Axis);
			/// @var static const Type Type::Connected
			/// @brief Controller connected.
			HL_ENUM_DECLARE(Type, Connected);
			/// @var static const Type Type::Disconnected
			/// @brief Controller disconnected.
			HL_ENUM_DECLARE(Type, Disconnected);
		));

		/// @brief The event type.
		Type type;
		/// @brief Index of the controller.
		int controllerIndex;
		/// @brief The button code.
		Button buttonCode;
		/// @brief axisValue The axis value.
		float axisValue;

		/// @brief Constructor.
		/// @param[in] type The event type.
		/// @param[in] controllerIndex Index of the controller.
		/// @param[in] buttonCode The button code.
		/// @param[in] axisValue The axis value.
		ControllerEvent(Type type, int controllerIndex, Button buttonCode, float axisValue);
			
	};

}
#endif
