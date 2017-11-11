/// @file
/// @version 4.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include <hltypes/hlog.h>
#include <hltypes/hstring.h>

#include "april.h"
#include "MotionDelegate.h"

namespace april
{
	MotionDelegate::MotionDelegate() : accelerometerEnabled(false), linearAccelerometerEnabled(false), gravityEnabled(false),
		rotationEnabled(false), gyroscopeEnabled(false)
	{
	}

	MotionDelegate::~MotionDelegate()
	{
	}

	void MotionDelegate::onAccelerometer(cgvec3 motionVector)
	{
		hlog::debug(logTag, "Event onAccelerometer() was not implemented.");
	}

	void MotionDelegate::onLinearAccelerometer(cgvec3 motionVector)
	{
		hlog::debug(logTag, "Event onLinearAccelerometer() was not implemented.");
	}

	void MotionDelegate::onGravity(cgvec3 motionVector)
	{
		hlog::debug(logTag, "Event onGravity() was not implemented.");
	}

	void MotionDelegate::onRotation(cgvec3 motionVector)
	{
		hlog::debug(logTag, "Event onRotation() was not implemented.");
	}

	void MotionDelegate::onGyroscope(cgvec3 motionVector)
	{
		hlog::debug(logTag, "Event onGyroscope() was not implemented.");
	}

}
