package com.april;

/// @version 4.0

import android.content.Context;
import android.graphics.PixelFormat;
import android.view.InputDevice;
import android.view.inputmethod.EditorInfo;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.text.InputType;
import com.april.Touch;
import java.util.List;
import java.util.ArrayList;

public class GLSurfaceView extends android.opengl.GLSurfaceView
{
	protected com.april.Renderer renderer = null;
	
	private static final int MOUSE_DOWN = 0;
	private static final int MOUSE_UP = 1;
	private static final int MOUSE_MOVE = 3;
	private static final int CONTROLLER_AXIS_LX = 100;
	private static final int CONTROLLER_AXIS_LY = 101;
	private static final int CONTROLLER_AXIS_RX = 102;
	private static final int CONTROLLER_AXIS_RY = 103;
	private static final int CONTROLLER_TRIGGER_L = 111;
	private static final int CONTROLLER_TRIGGER_R = 112;
	
	public GLSurfaceView(Context context)
	{
		super(context);
		this.setEGLContextClientVersion(2);
		this.setPreserveEGLContextOnPause(false);
		this.setEGLConfigChooser(8, 8, 8, 8, 0, 0);
		this.getHolder().setFormat(PixelFormat.RGBA_8888);
		this.renderer = new com.april.Renderer();
		this.setRenderer(this.renderer);
		// view has to be properly focusable to be able to process input
		this.setFocusable(true);
		this.setFocusableInTouchMode(true);
		this.setId(0x0513BEEF); // who doesn't love half a kilogram of beef?
	}
	
	@Override
	public void onWindowFocusChanged(final boolean focused)
	{
		if (focused)
		{
			this.requestFocus();
			this.requestFocusFromTouch();
			NativeInterface.updateKeyboard();
		}
		NativeInterface.aprilActivity.glView.queueEvent(new Runnable()
		{
			public void run()
			{
				NativeInterface.onWindowFocusChanged(focused);
			}
		});
	}
	
	@Override
	public boolean onTouchEvent(final MotionEvent event)
	{
		// Java is broken. "final MotionEvent event" can still be modified after this method finished and hence queuing into the GLThread can cause a crash.
		final int action = event.getAction();
		int type = -1;
		switch (action & MotionEvent.ACTION_MASK)
		{
		case MotionEvent.ACTION_DOWN:
		case MotionEvent.ACTION_POINTER_DOWN: // handles non-primary touches
			type = MOUSE_DOWN;
			break;
		case MotionEvent.ACTION_UP:
		case MotionEvent.ACTION_POINTER_UP: // handles non-primary touches
			type = MOUSE_UP;
			break;
		case MotionEvent.ACTION_MOVE: // Android batches multi-touch move events into a single move event
			type = MOUSE_MOVE;
			break;
		}
		if (type >= 0)
		{
			final ArrayList<Touch> touches = new ArrayList<Touch>();
			for (int i = 0; i < event.getPointerCount(); i++)
			{
				touches.add(new Touch(type, event.getX(i), event.getY(i)));
			}
			this.queueEvent(new Runnable()
			{
				public void run()
				{
					for (int i = 0; i < touches.size(); i++)
					{
						NativeInterface.onTouch(touches.get(i).type, touches.get(i).x, touches.get(i).y, i);
					}
				}
			});
			return true;
		}
		return false;
	}
	
	@Override
	public boolean onKeyDown(int keyCode, final KeyEvent event)
	{
		// Java is broken. "final KeyEvent event" can still be modified after this method finished and hence queuing into the GLThread can cause a crash.
		final int eventKeyCode = event.getKeyCode();
		if (NativeInterface.aprilActivity.ignoredKeys.contains(eventKeyCode))
		{
			return super.onKeyDown(keyCode, event);
		}
		if ((event.getSource() & InputDevice.SOURCE_GAMEPAD) == InputDevice.SOURCE_GAMEPAD)
		{
			return this._onButtonDown(keyCode, event);
		}
		final int eventUnicodeChar = event.getUnicodeChar();
		this.queueEvent(new Runnable()
		{
			public void run()
			{
				NativeInterface.onKeyDown(eventKeyCode, eventUnicodeChar);
			}
		});
		return true;
	}
	
	@Override
	public boolean onKeyUp(int keyCode, final KeyEvent event)
	{
		// Java is broken. "final KeyEvent event" can still be modified after this method finished and hence queuing into the GLThread can cause a crash.
		final int eventKeyCode = event.getKeyCode();
		if (NativeInterface.aprilActivity.ignoredKeys.contains(eventKeyCode))
		{
			return super.onKeyUp(keyCode, event);
		}
		if ((event.getSource() & InputDevice.SOURCE_GAMEPAD) == InputDevice.SOURCE_GAMEPAD)
		{
			return this._onButtonUp(keyCode, event);
		}
		this.queueEvent(new Runnable()
		{
			public void run()
			{
				NativeInterface.onKeyUp(eventKeyCode);
			}
		});
		return true;
	}
	
	private int _convertControllerButton(int buttonCode)
	{
		switch (buttonCode)
		{
		// as declared in Keys.h
		case KeyEvent.KEYCODE_BUTTON_START:		return 1;
		case KeyEvent.KEYCODE_BUTTON_SELECT:	return 2;
		case KeyEvent.KEYCODE_BUTTON_A:			return 11;
		case KeyEvent.KEYCODE_BUTTON_B:			return 12;
		case KeyEvent.KEYCODE_BUTTON_X:			return 13;
		case KeyEvent.KEYCODE_BUTTON_Y:			return 14;
		case KeyEvent.KEYCODE_BUTTON_L1:		return 21;
		case KeyEvent.KEYCODE_BUTTON_R1:		return 22;
		case KeyEvent.KEYCODE_BUTTON_L2:		return 23;
		case KeyEvent.KEYCODE_BUTTON_R2:		return 24;
		case KeyEvent.KEYCODE_BUTTON_THUMBL:	return 31;
		case KeyEvent.KEYCODE_BUTTON_THUMBR:	return 32;
		case KeyEvent.KEYCODE_DPAD_DOWN:		return 42;
		case KeyEvent.KEYCODE_DPAD_LEFT:		return 44;
		case KeyEvent.KEYCODE_DPAD_RIGHT:		return 46;
		case KeyEvent.KEYCODE_DPAD_UP:			return 48;
		}
		return 0;
	}
	
	private boolean _onButtonDown(int keyCode, final KeyEvent event)
	{
		// Java is broken. "final KeyEvent event" can still be modified after this method finished and hence queuing into the GLThread can cause a crash.
		final int eventKeyCode = this._convertControllerButton(event.getKeyCode());
		this.queueEvent(new Runnable()
		{
			public void run()
			{
				NativeInterface.onButtonDown(eventKeyCode);
			}
		});
		return true;
	}
	
	private boolean _onButtonUp(int keyCode, final KeyEvent event)
	{
		// Java is broken. "final KeyEvent event" can still be modified after this method finished and hence queuing into the GLThread can cause a crash.
		final int eventKeyCode = this._convertControllerButton(event.getKeyCode());
		this.queueEvent(new Runnable()
		{
			public void run()
			{
				NativeInterface.onButtonUp(eventKeyCode);
			}
		});
		return true;
	}
	
	@Override
	public boolean onGenericMotionEvent(final MotionEvent event)
	{
		if ((event.getSource() & InputDevice.SOURCE_JOYSTICK) == InputDevice.SOURCE_JOYSTICK && event.getAction() == MotionEvent.ACTION_MOVE)
		{
			final float axisLX = event.getAxisValue(MotionEvent.AXIS_X);
			final float axisLY = event.getAxisValue(MotionEvent.AXIS_Y);
			final float axisRX = event.getAxisValue(MotionEvent.AXIS_Z);
			final float axisRY = event.getAxisValue(MotionEvent.AXIS_RZ);
			final float triggerL = event.getAxisValue(MotionEvent.AXIS_LTRIGGER);
			final float triggerR = event.getAxisValue(MotionEvent.AXIS_RTRIGGER);
			this.queueEvent(new Runnable()
			{
				public void run()
				{
					NativeInterface.onControllerAxisChange(CONTROLLER_AXIS_LX, axisLX);
					NativeInterface.onControllerAxisChange(CONTROLLER_AXIS_LY, axisLY);
					NativeInterface.onControllerAxisChange(CONTROLLER_AXIS_RX, axisRX);
					NativeInterface.onControllerAxisChange(CONTROLLER_AXIS_RY, axisRY);
					NativeInterface.onControllerAxisChange(CONTROLLER_TRIGGER_L, triggerL);
					NativeInterface.onControllerAxisChange(CONTROLLER_TRIGGER_R, triggerR);
				}
			});
			return true;
		}
		return super.onGenericMotionEvent(event);
	}
	
	/*
	private void processJoystickInput(MotionEvent event, int historyPosition)
	{
		InputDevice inputDevice = event.getDevice();

		// Calculate the horizontal distance to move by
		// using the input value from one of these physical controls:
		// the left control stick, hat axis, or the right control stick.
		float x = this.getCenteredAxis(event, inputDevice, MotionEvent.AXIS_X, historyPosition);
		if (x == 0.0)
		{
			x = this.getCenteredAxis(event, inputDevice, MotionEvent.AXIS_HAT_X, historyPosition);
		}
		if (x == 0.0)
		{
			x = this.getCenteredAxis(event, inputDevice, MotionEvent.AXIS_Z, historyPosition);
		}
		// Calculate the vertical distance to move by
		// using the input value from one of these physical controls:
		// the left control stick, hat switch, or the right control stick.
		float y = this.getCenteredAxis(event, inputDevice, MotionEvent.AXIS_Y, historyPosition);
		if (y == 0)
		{
			y = this.getCenteredAxis(event, inputDevice, MotionEvent.AXIS_HAT_Y, historyPosition);
		}
		if (y == 0)
		{
			y = this.getCenteredAxis(event, inputDevice, MotionEvent.AXIS_RZ, historyPosition);
		}
		this.queueEvent(new Runnable()
		{
			public void run()
			{
				NativeInterface.onButtonUp(eventKeyCode);
			}
		});
		// Update the ship object based on the new x and y values
	}
	*/
	
	@Override
	public InputConnection onCreateInputConnection(EditorInfo outAttributes) // required for creation of soft keyboard
	{ 
		outAttributes.actionId = EditorInfo.IME_ACTION_DONE;
		outAttributes.imeOptions = EditorInfo.IME_FLAG_NO_EXTRACT_UI;
		outAttributes.inputType = InputType.TYPE_CLASS_TEXT;
		outAttributes.inputType |= InputType.TYPE_TEXT_FLAG_NO_SUGGESTIONS;
		// required, because certain keyboards (e.g. Swype on Android 5.x on Samsung S6 and S6 Edge+) ignore TYPE_TEXT_FLAG_NO_SUGGESTIONS
		outAttributes.inputType |= InputType.TYPE_TEXT_VARIATION_VISIBLE_PASSWORD;
		return new InputConnection(this);
	}
	
	public void swapBuffers()
	{
		this.renderer.swapBuffers();
	}
	
	@Override
	public boolean onCheckIsTextEditor() // required for creation of soft keyboard
	{
		return true;
	}
	
}

