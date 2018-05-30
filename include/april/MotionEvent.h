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
/// Defines a motion event.

#ifndef APRIL_MOTION_EVENT_H
#define APRIL_MOTION_EVENT_H

#include <hltypes/henum.h>
#include <gtypes/Vector3.h>

#include "aprilExport.h"

namespace april
{
	/// @brief Defines motion input event data.
	class aprilExport MotionEvent
	{
	public:
		/// @class Type
		/// @brief Defines motion event types.
		HL_ENUM_CLASS_PREFIX_DECLARE(aprilExport, Type,
		(
			/// @var static const Type Type::Accelerometer
			/// @brief Accelerometer vector.
			HL_ENUM_DECLARE(Type, Accelerometer);
			/// @var static const Type Type::LinearAccelerometer
			/// @brief Linear accelerometer vector.
			HL_ENUM_DECLARE(Type, LinearAccelerometer);
			/// @var static const Type Type::Gravity
			/// @brief Gravity vector.
			HL_ENUM_DECLARE(Type, Gravity);
			/// @var static const Type Type::Rotation
			/// @brief Device rotation.
			HL_ENUM_DECLARE(Type, Rotation);
			/// @var static const Type Type::Gyroscope
			/// @brief Gyroscope vector.
			HL_ENUM_DECLARE(Type, Gyroscope);
		));

		/// @brief The event type.
		Type type;
		/// @brief Motion data vector.
		gvec3f motionVector;

		/// @brief Constructor.
		/// @param[in] type The event type.
		/// @param[in] motionVector Motion data vector.
		MotionEvent(Type type, cgvec3f motionVector);

	};

}
#endif
