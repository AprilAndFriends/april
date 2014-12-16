package com.april;

/// @version 3.5

import android.content.Context;
import android.graphics.PixelFormat;
import android.view.inputmethod.EditorInfo;
import android.view.MotionEvent;
import android.text.InputType;
import java.util.List;
import java.util.ArrayList;

public class GLSurfaceView extends android.opengl.GLSurfaceView
{
	protected com.april.Renderer renderer = null;
	protected List<Integer> pointerIds = new ArrayList<Integer>();
	
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
		final int action = event.getAction();
		final int type;
		switch (action & MotionEvent.ACTION_MASK)
		{
		case MotionEvent.ACTION_DOWN:
		case MotionEvent.ACTION_POINTER_DOWN: // handles multi-touch
			type = MOUSE_DOWN;
			break;
		case MotionEvent.ACTION_UP:
		case MotionEvent.ACTION_POINTER_UP: // handles multi-touch
			type = MOUSE_UP;
			break;
		case MotionEvent.ACTION_MOVE: // Android batches multi-touch move events into a single move event
			type = MOUSE_MOVE;
			break;
		default:
			type = -1;
			break;
		}
		if (type >= 0)
		{
			this.queueEvent(new Runnable()
			{
				public void run()
				{
					final int pointerCount = event.getPointerCount();
					int id = 0;
					int index = 0;
					for (int i = 0; i < pointerCount; i++)
					{
						id = event.getPointerId(i);
						index = pointerIds.indexOf(id);
						switch (type)
						{
						case MOUSE_DOWN:
							if (index < 0)
							{
								index = pointerIds.size();
								pointerIds.add(id);
							}
							break;
						case MOUSE_UP:
							if (index < 0)
							{
								index = pointerIds.size();
							}
							else
							{
								pointerIds.remove(index);
							}
							break;
						case MOUSE_MOVE:
							if (index < 0)
							{
								index = pointerIds.size();
							}
							break;
						}
						NativeInterface.onTouch(type, event.getX(id), event.getY(id), index);
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

