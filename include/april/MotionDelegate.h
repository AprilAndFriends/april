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
/// Defines a delegate for the motion input callbacks.

#ifndef APRIL_MOTION_DELEGATE_H
#define APRIL_MOTION_DELEGATE_H

#include <gtypes/Vector3.h>

#include "aprilExport.h"
#include "Keys.h"

namespace april
{
	/// @brief Defines a delegate for the motion input callbacks.
	class aprilExport MotionDelegate
	{
	public:
		/// @brief Basic constructor.
		MotionDelegate();
		/// @brief Destructor.
		virtual ~MotionDelegate();

		/// @brief Whether linear accelerometer sensor is used.
		HL_DEFINE_ISSET(accelerometerEnabled, AccelerometerEnabled);
		/// @brief Whether linear accelerometer sensor is used.
		HL_DEFINE_ISSET(linearAccelerometerEnabled, LinearAccelerometerEnabled);
		/// @brief Whether gravity sensor is used.
		HL_DEFINE_ISSET(gravityEnabled, GravityEnabled);
		/// @brief Whether rotation sensor is used.
		HL_DEFINE_ISSET(rotationEnabled, RotationEnabled);
		/// @brief Whether gyroscope is used.
		HL_DEFINE_ISSET(gyroscopeEnabled, GyroscopeEnabled);

		/// @brief Called when the accelerometer of the device detects changes.
		/// @param[in] motionVector Accelerometer vector in the Cartesian coordinate system.
		virtual void onAccelerometer(cgvec3 motionVector);
		/// @brief Called when the linear accelerometer of the device detects changes.
		/// @param[in] motionVector Linear accelerometer vector in the Cartesian coordinate system.
		virtual void onLinearAccelerometer(cgvec3 motionVector);
		/// @brief Called when the gravity vector of the device changes.
		/// @param[in] motionVector Gravity vector in the Cartesian coordinate system.
		virtual void onGravity(cgvec3 motionVector);
		/// @brief Called when the rotation of the device changes.
		/// @param[in] motionVector Rotation vector equal to the last three components of a unit quaternion.
		virtual void onRotation(cgvec3 motionVector);
		/// @brief Called when the gyroscope of the device detects changes.
		/// @param[in] motionVector Movement vector in the Cartesian coordinate system.
		virtual void onGyroscope(cgvec3 motionVector);

	protected:
		/// @brief Whether accelerometer sensor is used.
		bool accelerometerEnabled;
		/// @brief Whether linear accelerometer sensor is used.
		bool linearAccelerometerEnabled;
		/// @brief Whether gravity sensor is used.
		bool gravityEnabled;
		/// @brief Whether rotation sensor is used.
		bool rotationEnabled;
		/// @brief Whether gyroscope is used.
		bool gyroscopeEnabled;

	};

}
#endif
