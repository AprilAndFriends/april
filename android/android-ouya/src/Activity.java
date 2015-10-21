package com.april.ouya;

/// @version 3.7

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.view.InputDevice;
import android.view.KeyEvent;
import android.view.MotionEvent;
import com.april.Touch;

import tv.ouya.console.api.OuyaController;
import tv.ouya.console.api.OuyaInputMapper;

public class Activity extends com.april.Activity
{
	private float oldLSX;
	private float oldLSY;
	private float oldRSX;
	private float oldRSY;
	private float oldLT;
	private float oldRT;
	
	@Override
	protected void onCreate(Bundle savedInstanceState)
	{
		this.oldLSX = 0;
		this.oldLSY = 0;
		this.oldRSX = 0;
		this.oldRSY = 0;
		this.oldLT = 0;
		this.oldRT = 0;
		this.ouyaKeyboardFix = true;
		super.onCreate(savedInstanceState);
		OuyaInputMapper.init(this);
		OuyaController.init(this);
		this.registerReceiver(new BroadcastReceiver()
		{
			@Override
			public void onReceive(Context context, Intent intent)
			{
				glView.onWindowFocusChanged(false);
			}
		}, new IntentFilter(tv.ouya.console.api.OuyaIntent.ACTION_MENUAPPEARING));
	}
	
	@Override
	public void onDestroy()
	{
		OuyaInputMapper.shutdown(this);
		super.onDestroy();
    }
	
	@Override
	public boolean dispatchKeyEvent(KeyEvent keyEvent)
	{
		boolean handled = false;
		if (OuyaInputMapper.shouldHandleInputEvent(keyEvent))
		{
			handled = OuyaInputMapper.dispatchKeyEvent(this, keyEvent);
		}
		return (handled || super.dispatchKeyEvent(keyEvent));
	}

	@Override
	public boolean dispatchGenericMotionEvent(MotionEvent motionEvent)
	{
		boolean handled = false;
		if (OuyaInputMapper.shouldHandleInputEvent(motionEvent))
		{
			handled = OuyaInputMapper.dispatchGenericMotionEvent(this, motionEvent);
		}
		return (handled || super.dispatchGenericMotionEvent(motionEvent));
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
		boolean result = OuyaController.onKeyDown(keyCode, event);
		if (newKeyCode != 0)
		{
			this.glView.queueEvent(new Runnable()
			{
				public void run()
				{
					com.april.NativeInterface.onButtonDown(newKeyCode);
				}
			});
			return true;
		}
		return (result || super.onKeyDown(keyCode, event));
	}
	
	@Override
	public boolean onKeyUp(int keyCode, final KeyEvent event)
	{
		final int newKeyCode = this._convertOuyaButton(keyCode);
		boolean result = OuyaController.onKeyUp(keyCode, event);
		if (newKeyCode != 0)
		{
			this.glView.queueEvent(new Runnable()
			{
				public void run()
				{
					com.april.NativeInterface.onButtonUp(newKeyCode);
				}
			});
			return true;
		}
		return (result || super.onKeyUp(keyCode, event));
	}
	
	@Override
	public boolean onGenericMotionEvent(final MotionEvent event)
	{
		boolean result = OuyaController.onGenericMotionEvent(event);
		if ((event.getSource() & InputDevice.SOURCE_CLASS_POINTER) != 0)
		{
			final Touch touch = new Touch(2, event.getX(), event.getY());
			this.glView.queueEvent(new Runnable()
			{
				public void run()
				{
					com.april.NativeInterface.onTouch(touch.type, touch.x, touch.y, 0);
				}
			});
			return true;
		}
		if ((event.getSource() & InputDevice.SOURCE_CLASS_JOYSTICK) != 0)
		{
			final float lsx = event.getAxisValue(OuyaController.AXIS_LS_X);
			final float lsy = event.getAxisValue(OuyaController.AXIS_LS_Y);
			final float rsx = event.getAxisValue(OuyaController.AXIS_RS_X);
			final float rsy = event.getAxisValue(OuyaController.AXIS_RS_Y);
			final float lt = event.getAxisValue(OuyaController.AXIS_L2);
			final float rt = event.getAxisValue(OuyaController.AXIS_R2);
			
			if (this.oldLSX != lsx)
			{
				this.oldLSX = lsx;
				this.glView.queueEvent(new Runnable()
				{
					public void run()
					{
						com.april.NativeInterface.onControllerAxis(100, lsx);
					}
				});
			}
			if (this.oldLSY != lsy)
			{
				this.oldLSY = lsy;
				this.glView.queueEvent(new Runnable()
				{
					public void run()
					{
						com.april.NativeInterface.onControllerAxis(101, lsy);
					}
				});
			}
			if (this.oldRSX != rsx)
			{
				this.oldRSX = rsx;
				this.glView.queueEvent(new Runnable()
				{
					public void run()
					{
						com.april.NativeInterface.onControllerAxis(102, rsx);
					}
				});
			}
			if (this.oldRSY != rsy)
			{
				this.oldRSY = rsy;
				this.glView.queueEvent(new Runnable()
				{
					public void run()
					{
						com.april.NativeInterface.onControllerAxis(103, rsy);
					}
				});
			}
			if (this.oldLT != lt)
			{
				this.oldLT = lt;
				this.glView.queueEvent(new Runnable()
				{
					public void run()
					{
						com.april.NativeInterface.onControllerAxis(104, lt);
					}
				});
			}
			if (this.oldRT != rt)
			{
				this.oldRT = rt;
				this.glView.queueEvent(new Runnable()
				{
					public void run()
					{
						com.april.NativeInterface.onControllerAxis(105, rt);
					}
				});
			}
			return true;
		}
		return (result || super.onGenericMotionEvent(event));
	}
}
