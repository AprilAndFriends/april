package com.april;

/// @version 3.5

import android.content.Context;
import android.graphics.PixelFormat;
import android.view.inputmethod.EditorInfo;
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
	
	public GLSurfaceView(Context context)
	{
		super(context);
		this.setEGLConfigChooser(8, 8, 8, 8, 0, 0);
		this.getHolder().setFormat(PixelFormat.RGBA_8888);
		this.renderer = new com.april.Renderer();
		this.setRenderer(this.renderer);
		// view has to be properly focusable to be able to process input
		this.setFocusable(true);
		this.setFocusableInTouchMode(true);
		this.setId(0x0513BEEF); // who doesn't love half a kilo of beef?
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
		NativeInterface.AprilActivity.GlView.queueEvent(new Runnable()
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
	public InputConnection onCreateInputConnection(EditorInfo outAttributes)  // required for creation of soft keyboard
	{ 
		outAttributes.actionId = EditorInfo.IME_ACTION_DONE;
		outAttributes.imeOptions = EditorInfo.IME_FLAG_NO_EXTRACT_UI;
		if (NativeInterface.AprilActivity.OuyaKeyboardFix) // OUYA software keyboard doesn't appear unless TYPE_CLASS_TEXT is specified as input type
		{
			outAttributes.inputType = InputType.TYPE_CLASS_TEXT;
		}
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

