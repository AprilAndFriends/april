/// @file
/// @version 5.1
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Defines a mouse event.

#ifndef APRIL_MOUSE_EVENT_H
#define APRIL_MOUSE_EVENT_H

#include <hltypes/henum.h>
#include <gtypes/Vector2.h>

#include "aprilExport.h"
#include "Keys.h"

namespace april
{
	/// @brief Defines mouse event data.
	class aprilExport MouseEvent
	{
	public:
		/// @class Type
		/// @brief Defines mouse event types.
		HL_ENUM_CLASS_PREFIX_DECLARE(aprilExport, Type,
		(
			/// @var static const Type Type::Down
			/// @brief Mouse button was pressed.
			HL_ENUM_DECLARE(Type, Down);
			/// @var static const Type Type::Up
			/// @brief Mouse button was released.
			HL_ENUM_DECLARE(Type, Up);
			/// @var static const Type Type::Cancel
			/// @brief Mouse button was canceled without an Up event.
			HL_ENUM_DECLARE(Type, Cancel);
			/// @var static const Type Type::Move
			/// @brief Mouse was moved.
			HL_ENUM_DECLARE(Type, Move);
			/// @var static const Type Type::Scroll
			/// @brief Mouse scroll was changed (usually a scroll wheel).
			HL_ENUM_DECLARE(Type, Scroll);
		));

		/// @brief The event type.
		Type type;
		/// @brief The pointer position.
		gvec2f position;
		/// @brief The key code.
		Key keyCode;
		
		/// @brief Constructor.
		/// @param[in] type The event type.
		/// @param[in] position The pointer position.
		/// @param[in] keyCode The key code.
		MouseEvent(Type type, cgvec2f position, Key keyCode);
		
	};

}
#endif
