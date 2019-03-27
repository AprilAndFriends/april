package com.april;

/// @version 5.2

import android.content.Context;
import android.graphics.PixelFormat;
import android.view.InputDevice;
import android.view.inputmethod.EditorInfo;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.text.InputType;
import java.util.List;
import java.util.ArrayList;

public class GLSurfaceView extends android.opengl.GLSurfaceView
{
	protected com.april.Renderer renderer = null;
	
	protected float lastAxisLX;
	protected float lastAxisLY;
	protected float lastAxisRX;
	protected float lastAxisRY;
	protected float lastTriggerL;
	protected float lastTriggerR;
	
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
		this.lastAxisLX = 0.0f;
		this.lastAxisLY = 0.0f;
		this.lastAxisRX = 0.0f;
		this.lastAxisRY = 0.0f;
		this.lastTriggerL = 0.0f;
		this.lastTriggerR = 0.0f;
	}
	
	@Override
	public void onWindowFocusChanged(final boolean focused)
	{
		if (focused)
		{
			this.requestFocus();
			this.requestFocusFromTouch();
		}
		// native call is queued into render thread, MUST NOT be called directly from this thread
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
		case MotionEvent.ACTION_MOVE:
			type = MOUSE_MOVE;
			break;
		}
		if (type >= 0)
		{
			if (type == MOUSE_MOVE) // Android batches multi-touch move events into a single move event
			{
				for (int i = 0; i < event.getPointerCount(); ++i)
				{
					NativeInterface.onTouch(type, event.getPointerId(i), event.getX(i), event.getY(i));
				}
			}
			else
			{
				final int index = event.getActionIndex();
				NativeInterface.onTouch(type, event.getPointerId(index), event.getX(index), event.getY(index));
			}
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
		if (eventKeyCode != KeyEvent.KEYCODE_BACK)
		{
			final int source = event.getSource();
			// some controllers send InputDevice.SOURCE_JOYSTICK even though the D-Pad buttons are used
			// some controllers send 0x501 instead of InputDevice.SOURCE_GAMEPAD (which is 0x401)
			if ((source & InputDevice.SOURCE_GAMEPAD) == InputDevice.SOURCE_GAMEPAD || (source & InputDevice.SOURCE_DPAD) == InputDevice.SOURCE_DPAD ||
				(source & InputDevice.SOURCE_JOYSTICK) == InputDevice.SOURCE_JOYSTICK || (source & 0x501) == 0x501)
			{
				return this._onButtonDown(eventKeyCode);
			}
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
		if (eventKeyCode != KeyEvent.KEYCODE_BACK)
		{
			final int source = event.getSource();
			// some controllers send InputDevice.SOURCE_JOYSTICK even though the D-Pad buttons are used
			// some controllers send 0x501 instead of InputDevice.SOURCE_GAMEPAD (which is 0x401)
			if ((source & InputDevice.SOURCE_GAMEPAD) == InputDevice.SOURCE_GAMEPAD || (source & InputDevice.SOURCE_DPAD) == InputDevice.SOURCE_DPAD ||
				(source & InputDevice.SOURCE_JOYSTICK) == InputDevice.SOURCE_JOYSTICK || (source & 0x501) == 0x501)
			{
				return this._onButtonUp(eventKeyCode);
			}
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
	
	// non-ASCII characters aren't sent as normal key codes
	@Override
	public boolean onKeyMultiple(final int keyCode, int repeatCount, KeyEvent event)
	{
		if (keyCode != 0 || event.getAction() != MotionEvent.ACTION_MOVE)
		{
			return super.onKeyMultiple(keyCode, repeatCount, event);
		}
		String chars = event.getCharacters();
		if (chars == null || chars.length() == 0)
		{
			return super.onKeyMultiple(keyCode, repeatCount, event);
		}
		final int eventUnicodeChar = chars.codePointAt(0);
		if (eventUnicodeChar == 0)
		{
			return super.onKeyMultiple(keyCode, repeatCount, event);
		}
		this.queueEvent(new Runnable()
		{
			public void run()
			{
				NativeInterface.onChar(eventUnicodeChar);
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
		case KeyEvent.KEYCODE_BUTTON_MODE:		return 3;
		case KeyEvent.KEYCODE_BUTTON_A:			return 11;
		case KeyEvent.KEYCODE_BUTTON_B:			return 12;
		case KeyEvent.KEYCODE_BUTTON_C:			return 13;
		case KeyEvent.KEYCODE_BUTTON_X:			return 14;
		case KeyEvent.KEYCODE_BUTTON_Y:			return 15;
		case KeyEvent.KEYCODE_BUTTON_Z:			return 16;
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
		// generic buttons
		case KeyEvent.KEYCODE_BUTTON_1:			return 14;
		case KeyEvent.KEYCODE_BUTTON_2:			return 11;
		case KeyEvent.KEYCODE_BUTTON_3:			return 12;
		case KeyEvent.KEYCODE_BUTTON_4:			return 15;
		case KeyEvent.KEYCODE_BUTTON_5:			return 21;
		case KeyEvent.KEYCODE_BUTTON_6:			return 22;
		case KeyEvent.KEYCODE_BUTTON_7:			return 23;
		case KeyEvent.KEYCODE_BUTTON_8:			return 24;
		case KeyEvent.KEYCODE_BUTTON_9:			return 1;
		case KeyEvent.KEYCODE_BUTTON_10:		return 2;
		case KeyEvent.KEYCODE_BUTTON_11:		return 31;
		case KeyEvent.KEYCODE_BUTTON_12:		return 32;
		}
		return 0;
	}
	
	private boolean _onButtonDown(final int keyCode)
	{
		final int buttonCode = this._convertControllerButton(keyCode);
		this.queueEvent(new Runnable()
		{
			public void run()
			{
				NativeInterface.onButtonDown(0, buttonCode);
			}
		});
		return true;
	}
	
	private boolean _onButtonUp(final int keyCode)
	{
		final int buttonCode = this._convertControllerButton(keyCode);
		this.queueEvent(new Runnable()
		{
			public void run()
			{
				NativeInterface.onButtonUp(0, buttonCode);
			}
		});
		return true;
	}
	
	@Override
	public boolean onGenericMotionEvent(final MotionEvent event)
	{
		final int source = event.getSource();
		if ((source & InputDevice.SOURCE_CLASS_POINTER) == InputDevice.SOURCE_CLASS_POINTER && event.getAction() == MotionEvent.ACTION_SCROLL)
		{
			// TODO - switch to new code and test
			/*
			final float x = -event.getAxisValue(MotionEvent.AXIS_HSCROLL);
			final float y = -event.getAxisValue(MotionEvent.AXIS_VSCROLL);
			this.queueEvent(new Runnable()
			{
				public void run()
				{
					NativeInterface.onScroll(x, y);
				}
			});
			*/
			NativeInterface.onScroll(-event.getAxisValue(MotionEvent.AXIS_HSCROLL), -event.getAxisValue(MotionEvent.AXIS_VSCROLL));
			return true;
		}
		if ((source & InputDevice.SOURCE_JOYSTICK) == InputDevice.SOURCE_JOYSTICK && event.getAction() == MotionEvent.ACTION_MOVE)
		{
			final float axisLX = event.getAxisValue(MotionEvent.AXIS_X);
			final float axisLY = event.getAxisValue(MotionEvent.AXIS_Y);
			final float axisRX = event.getAxisValue(MotionEvent.AXIS_Z);
			final float axisRY = event.getAxisValue(MotionEvent.AXIS_RZ);
			final float triggerL = event.getAxisValue(MotionEvent.AXIS_LTRIGGER);
			final float triggerR = event.getAxisValue(MotionEvent.AXIS_RTRIGGER);
			boolean handled = false;
			if (this.lastAxisLX != axisLX)
			{
				this.lastAxisLX = axisLX;
				this.queueEvent(new Runnable()
				{
					public void run()
					{
						NativeInterface.onControllerAxisChange(0, CONTROLLER_AXIS_LX, axisLX);
					}
				});
				handled = true;
			}
			if (this.lastAxisLY != axisLY)
			{
				this.lastAxisLY = axisLY;
				this.queueEvent(new Runnable()
				{
					public void run()
					{
						NativeInterface.onControllerAxisChange(0, CONTROLLER_AXIS_LY, axisLY);
					}
				});
				handled = true;
			}
			if (this.lastAxisRX != axisRX)
			{
				this.lastAxisRX = axisRX;
				this.queueEvent(new Runnable()
				{
					public void run()
					{
						NativeInterface.onControllerAxisChange(0, CONTROLLER_AXIS_RX, axisRX);
					}
				});
				handled = true;
			}
			if (this.lastAxisRY != axisRY)
			{
				this.lastAxisRY = axisRY;
				this.queueEvent(new Runnable()
				{
					public void run()
					{
						NativeInterface.onControllerAxisChange(0, CONTROLLER_AXIS_RY, axisRY);
					}
				});
				handled = true;
			}
			if (this.lastTriggerL != triggerL)
			{
				this.lastTriggerL = triggerL;
				this.queueEvent(new Runnable()
				{
					public void run()
					{
						NativeInterface.onControllerAxisChange(0, CONTROLLER_TRIGGER_L, triggerL);
					}
				});
				handled = true;
			}
			if (this.lastTriggerR != triggerR)
			{
				this.lastTriggerR = triggerR;
				this.queueEvent(new Runnable()
				{
					public void run()
					{
						NativeInterface.onControllerAxisChange(0, CONTROLLER_TRIGGER_R, triggerR);
					}
				});
				handled = true;
			}
			if (handled)
			{
				return true;
			}
		}
		return super.onGenericMotionEvent(event);
	}
	
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
	
	public void destroy()
	{
		// this special hack is required, because it seems that along the way somewhere Android "forgets" that the view was paused
		// and won't properly destroy the GLThread
		this.onResume();
		this.onPause();
	}
	
}

