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
/// Defines a touch event.

#ifndef APRIL_TOUCH_EVENT_H
#define APRIL_TOUCH_EVENT_H

#include <hltypes/henum.h>
#include <gtypes/Vector2.h>

#include "aprilExport.h"
#include "Keys.h"

namespace april
{
	/// @brief Defines touch event data.
	class aprilExport TouchEvent
	{
	public:
		/// @class Type
		/// @brief Defines touch event types.
		HL_ENUM_CLASS_PREFIX_DECLARE(aprilExport, Type,
		(
			/// @var static const Type Type::Down
			/// @brief Touch was pressed down.
			HL_ENUM_DECLARE(Type, Down);
			/// @var static const Type Type::Up
			/// @brief Touch was released.
			HL_ENUM_DECLARE(Type, Up);
			/// @var static const Type Type::Cancel
			/// @brief Touch was cancel.
			HL_ENUM_DECLARE(Type, Cancel);
			/// @var static const Type Type::Move
			/// @brief Touch was moved.
			HL_ENUM_DECLARE(Type, Move);
		));

		/// @brief The event type.
		Type type;
		/// @brief The touch index.
		int index;
		/// @brief The touch position.
		gvec2f position;
		
		/// @brief Constructor.
		/// @param[in] type The event type.
		/// @param[in] index The touch index.
		/// @param[in] position The touch position.
		TouchEvent(Type type, int index, cgvec2f position);
		
	};

}
#endif
