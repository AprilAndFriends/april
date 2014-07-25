package com.april.ouya;

/// @version 3.5

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.view.InputDevice;
import android.view.KeyEvent;
import android.view.MotionEvent;

import tv.ouya.console.api.OuyaController;

public class Activity extends com.april.Activity
{
	private float oldLSX, oldLSY, oldRSX, oldRSY, oldLT, oldRT;
	@Override
	protected void onCreate(Bundle savedInstanceState)
	{
		this.oldLSX = this.oldLSY = this.oldRSX = this.oldRSY = this.oldLT = this.oldRT = 0;
		this.OuyaKeyboardFix = true;
		super.onCreate(savedInstanceState);
		OuyaController.init(this);
		this.registerReceiver(new BroadcastReceiver()
		{
			@Override
			public void onReceive(Context context, Intent intent)
			{
				GlView.onWindowFocusChanged(false);
			}
		}, new IntentFilter(tv.ouya.console.api.OuyaIntent.ACTION_MENUAPPEARING));
	}
	
	protected int _convertOuyaButton(int buttonCode)
	{
		switch (buttonCode)
		{
		// as declared in Keys.h
		case OuyaController.BUTTON_O:		return 11;
		case OuyaController.BUTTON_U:		return 12;
		case OuyaController.BUTTON_Y:		return 13;
		case OuyaController.BUTTON_A:		return 14;
		case OuyaController.BUTTON_L1:		return 21;
		case OuyaController.BUTTON_L2:		return 22;
		case OuyaController.BUTTON_L3:		return 23;
		case OuyaController.BUTTON_R1:		return 24;
		case OuyaController.BUTTON_R2:		return 25;
		case OuyaController.BUTTON_R3:		return 26;
		case KeyEvent.KEYCODE_DPAD_DOWN:	return 32;
		case KeyEvent.KEYCODE_DPAD_LEFT:	return 34;
		case KeyEvent.KEYCODE_DPAD_RIGHT:	return 36;
		case KeyEvent.KEYCODE_DPAD_UP:		return 38;
		}
		return 0;
	}
	
	@Override
	public boolean onKeyDown(int keyCode, final KeyEvent event)
	{
		final int newKeyCode = this._convertOuyaButton(keyCode);
		if (newKeyCode != 0)
		{
			this.GlView.queueEvent(new Runnable()
			{
				public void run()
				{
					com.april.NativeInterface.onButtonDown(newKeyCode);
				}
			});
			return true;
		}
		return super.onKeyDown(keyCode, event);
	}
	
	@Override
	public boolean onKeyUp(int keyCode, final KeyEvent event)
	{
		final int newKeyCode = this._convertOuyaButton(keyCode);
		if (newKeyCode != 0)
		{
			this.GlView.queueEvent(new Runnable()
			{
				public void run()
				{
					com.april.NativeInterface.onButtonUp(newKeyCode);
				}
			});
			return true;
		}
		return super.onKeyUp(keyCode, event);
	}
	
	@Override
	public boolean onGenericMotionEvent(final MotionEvent event)
	{
		if ((event.getSource() & InputDevice.SOURCE_CLASS_POINTER) != 0)
		{
			this.GlView.queueEvent(new Runnable()
			{
				public void run()
				{
					com.april.NativeInterface.onTouch(2, event.getX(), event.getY(), 0);
				}
			});
			return true;
		}
		else if ((event.getSource() & InputDevice.SOURCE_CLASS_JOYSTICK) != 0)
		{
			final float lsx = event.getAxisValue(OuyaController.AXIS_LS_X);
			final float lsy = event.getAxisValue(OuyaController.AXIS_LS_Y);
			final float rsx = event.getAxisValue(OuyaController.AXIS_RS_X);
			final float rsy = event.getAxisValue(OuyaController.AXIS_RS_Y);
			final float lt = event.getAxisValue(OuyaController.AXIS_L2);
			final float rt = event.getAxisValue(OuyaController.AXIS_R2);
			
			if (lsx != this.oldLSX)
			{
				this.oldLSX = lsx;
				this.GlView.queueEvent(new Runnable()
				{
					public void run()
					{
						com.april.NativeInterface.onControllerAxis(100, lsx);
					}
				});
			}
			if (lsy != this.oldLSY)
			{
				this.oldLSY = lsy;
				this.GlView.queueEvent(new Runnable()
				{
					public void run()
					{
						com.april.NativeInterface.onControllerAxis(101, lsy);
					}
				});
			}
			if (rsx != this.oldRSX)
			{
				this.oldRSX = rsx;
				this.GlView.queueEvent(new Runnable()
				{
					public void run()
					{
						com.april.NativeInterface.onControllerAxis(102, rsx);
					}
				});
			}
			if (rsy != this.oldRSY)
			{
				this.oldRSY = rsy;
				this.GlView.queueEvent(new Runnable()
				{
					public void run()
					{
						com.april.NativeInterface.onControllerAxis(103, rsy);
					}
				});
			}
			if (lt != this.oldLT)
			{
				this.oldLT = lt;
				this.GlView.queueEvent(new Runnable()
				{
					public void run()
					{
						com.april.NativeInterface.onControllerAxis(104, lt);
					}
				});
			}
			if (rt != this.oldRT)
			{
				this.oldRT = rt;
				this.GlView.queueEvent(new Runnable()
				{
					public void run()
					{
						com.april.NativeInterface.onControllerAxis(105, rt);
					}
				});
			}
            return true;
        }
		else return super.onGenericMotionEvent(event);
	}
}
