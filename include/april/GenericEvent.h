/// @file
/// @version 5.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Defines a generic event.

#ifndef APRIL_GENERIC_EVENT_H
#define APRIL_GENERIC_EVENT_H

#include <hltypes/henum.h>
#include <gtypes/Vector2.h>

#include "aprilExport.h"

namespace april
{
	// TODOx - this class might benefit from using "union"
	/// @brief Defines a generic event.
	class aprilExport GenericEvent
	{
	public:
		/// @class Type
		/// @brief Defines keyboard key event types.
		HL_ENUM_CLASS_PREFIX_DECLARE(aprilExport, Type,
		(
			/// @var static const Type Type::QuitRequest
			/// @brief When app was requested to quit.
			HL_ENUM_DECLARE(Type, QuitRequest);
			/// @var static const Type Type::FocusChange
			/// @brief When app focus was changed.
			HL_ENUM_DECLARE(Type, FocusChange);
			/// @var static const Type Type::ActivityChange
			/// @brief When app activity was changed.
			/// @note This is only available on certain platforms.
			HL_ENUM_DECLARE(Type, ActivityChange);
			/// @var static const Type Type::SizeChange
			/// @brief When app size was changed (usually window size).
			HL_ENUM_DECLARE(Type, SizeChange);
			/// @var static const Type Type::InputModeChange
			/// @brief When input mode was changed.
			HL_ENUM_DECLARE(Type, InputModeChange);
			/// @var static const Type Type::VirtualKeyboardChange
			/// @brief When virtual keyboard display changed.
			HL_ENUM_DECLARE(Type, VirtualKeyboardChange);
			/// @var static const Type Type::LowMemoryWarning
			/// @brief When OS is running out of memory.
			HL_ENUM_DECLARE(Type, LowMemoryWarning);
		));

		/// @brief The event type.
		Type type;
		/// @brief An int value related to the event.
		int intValue;
		/// @brief Another int value related to the event.
		int intValueOther;
		/// @brief A float value related to the event.
		float floatValue;
		/// @brief A bool value related to the event.
		bool boolValue;
		/// @brief A gvec2 value related to the event.
		gvec2 gvec2Value;

		/// @brief Basic constructor.
		GenericEvent();
		/// @brief Constructor.
		/// @param[in] type The event type.
		GenericEvent(Type type);
		/// @brief Constructor.
		/// @param[in] type The event type.
		/// @param[in] intValue An in value related to the event.
		GenericEvent(Type type, int intValue);
		/// @brief Constructor.
		/// @param[in] type The event type.
		/// @param[in] intValue An int value related to the event.
		/// @param[in] intValueOther Another int value related to the event.
		/// @param[in] boolValue A bool value related to the event.
		GenericEvent(Type type, int intValue, int intValueOther, bool boolValue);
		/// @brief Constructor.
		/// @param[in] type The event type.
		/// @param[in] boolValue A bool value related to the event.
		GenericEvent(Type type, bool boolValue);
		/// @brief Constructor.
		/// @param[in] type The event type.
		/// @param[in] boolValue A bool value related to the event.
		/// @param[in] floatValue A float value related to the event.
		GenericEvent(Type type, bool boolValue, float floatValue);
		/// @brief Constructor.
		/// @param[in] type The event type.
		/// @param[in] gvec2Value A gvec2 value related to the event.
		GenericEvent(Type type, cgvec2 gvec2Value);

	};

}
#endif
