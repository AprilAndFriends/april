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
/// Defines a touch event.

#ifndef APRIL_KEY_EVENT_H
#define APRIL_KEY_EVENT_H

#include <hltypes/henum.h>

#include "aprilExport.h"
#include "Keys.h"

namespace april
{
	/// @brief Defines keyboard input event data.
	class aprilExport KeyEvent
	{
	public:
		/// @class Type
		/// @brief Defines keyboard key event types.
		HL_ENUM_CLASS_PREFIX_DECLARE(aprilExport, Type,
		(
			/// @var static const Type Type::Down
			/// @brief Key was pressed.
			HL_ENUM_DECLARE(Type, Down);
			/// @var static const Type Type::Up
			/// @brief Key was released.
			HL_ENUM_DECLARE(Type, Up);
		));

		/// @brief The event type.
		Type type;
		/// @brief The key code.
		Key keyCode;
		/// @brief The character Unicode value.
		unsigned int charCode;

		/// @brief Constructor.
		/// @param[in] type The event type.
		/// @param[in] keyCode The key code.
		/// @param[in] charCode The character Unicode value.
		KeyEvent(Type type, Key keyCode, unsigned int charCode);
		
	};

}
#endif
