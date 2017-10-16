/// @file
/// @version 4.4
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
	MotionDelegate::MotionDelegate()
	{
	}

	MotionDelegate::~MotionDelegate()
	{
	}

	void MotionDelegate::onLinearAccelerometer(gvec3 motionVector)
	{
		hlog::debug(logTag, "Event onLinearAccelerometer() was not implemented.");
	}

	void MotionDelegate::onRotation(gvec3 rotationVector)
	{
		hlog::debug(logTag, "Event onRotation() was not implemented.");
	}

	void MotionDelegate::onGyroscope(gvec3 motionVector)
	{
		hlog::debug(logTag, "Event onGyroscope() was not implemented.");
	}

}
