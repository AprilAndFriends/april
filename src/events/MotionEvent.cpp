/// @file
/// @version 5.0
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include "MotionEvent.h"

namespace april
{
	HL_ENUM_CLASS_DEFINE(MotionEvent::Type,
	(
		HL_ENUM_DEFINE(MotionEvent::Type, Accelerometer);
		HL_ENUM_DEFINE(MotionEvent::Type, LinearAccelerometer);
		HL_ENUM_DEFINE(MotionEvent::Type, Gravity);
		HL_ENUM_DEFINE(MotionEvent::Type, Rotation);
		HL_ENUM_DEFINE(MotionEvent::Type, Gyroscope);
	));

	MotionEvent::MotionEvent(Type type, cgvec3f motionVector)
	{
		this->type = type;
		this->motionVector = motionVector;
	}

}
