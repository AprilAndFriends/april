package com.april;

/// @version 4.5
import android.app.Activity;
import android.content.Context;
import android.content.pm.ActivityInfo;
import android.hardware.Sensor;
import android.hardware.SensorEvent;

public class SensorEventListener implements android.hardware.SensorEventListener
{
	public void onAccuracyChanged(Sensor sensor, int accuracy)
	{
	}
	
	public void onSensorChanged(SensorEvent event)
	{
		float x = event.values[0];
		float y = event.values[1];
		float z = event.values[2];
		if (NativeInterface.activity.getRequestedOrientation() == ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE)
		{
			x = -event.values[1];
			y = event.values[0];
		}
		else if (NativeInterface.activity.getRequestedOrientation() == ActivityInfo.SCREEN_ORIENTATION_REVERSE_LANDSCAPE)
		{
			x = event.values[1];
			y = -event.values[0];
		}
		else if (NativeInterface.activity.getRequestedOrientation() == ActivityInfo.SCREEN_ORIENTATION_REVERSE_PORTRAIT)
		{
			x = -event.values[0];
			y = -event.values[1];
		}
		int type = event.sensor.getType();
		if (type == Sensor.TYPE_GRAVITY)
		{
			NativeInterface.onGravity(x, y, z);
			return;
		}
		if (type == Sensor.TYPE_LINEAR_ACCELERATION)
		{
			NativeInterface.onLinearAccelerometer(x, y, z);
			return;
		}
		if (type == Sensor.TYPE_ROTATION_VECTOR)
		{
			NativeInterface.onRotation(x, y, z);
			return;
		}
		if (type == Sensor.TYPE_GYROSCOPE)
		{
			NativeInterface.onGyroscope(x, y, z);
			return;
		}
	}
	
}
