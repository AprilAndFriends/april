package net.sourceforge.april.android;

// version 2.1

import android.content.Context;
import android.view.inputmethod.EditorInfo;
import android.view.MotionEvent;

public class GLSurfaceView extends android.opengl.GLSurfaceView
{
	private net.sourceforge.april.android.Renderer renderer;
	
	public GLSurfaceView(Context context)
	{
		super(context);
		this.setEGLConfigChooser(false);
		this.renderer = new net.sourceforge.april.android.Renderer();
		this.setRenderer(this.renderer);
		// view has to be properly focusable to be able to process input
		this.setFocusable(true);
		this.setFocusableInTouchMode(true);
	}
	
	public boolean onTouchEvent(final MotionEvent event)
	{
		this.queueEvent
		(
			new Runnable()
			{
				public void run()
				{
					final int action = event.getAction();
					final int index = (action & MotionEvent.ACTION_POINTER_INDEX_MASK) >> MotionEvent.ACTION_POINTER_INDEX_SHIFT;
					final int pointerCount = event.getPointerCount();
					switch (action & MotionEvent.ACTION_MASK)
					{
					case MotionEvent.ACTION_DOWN:
					case MotionEvent.ACTION_POINTER_DOWN: // handles multi-touch
						NativeInterface.onTouch(0, event.getX(index), event.getY(index), index);
						break;
					case MotionEvent.ACTION_UP:
					case MotionEvent.ACTION_POINTER_UP: // handles multi-touch
						NativeInterface.onTouch(1, event.getX(index), event.getY(index), index);
						break;
					case MotionEvent.ACTION_MOVE: // Android batches multitouch move events into a single move event
						for (int i = 0; i < pointerCount; i++)
						{
							NativeInterface.onTouch(2, event.getX(i), event.getY(i), i);
						}
						break;
					}
				}
			}
		);
		return true;
	}
	
	@Override 
	public InputConnection onCreateInputConnection(EditorInfo outAttributes)  // required for creation of soft keyboard
	{ 
		outAttributes.actionId = EditorInfo.IME_ACTION_DONE; 
		outAttributes.imeOptions = EditorInfo.IME_FLAG_NO_EXTRACT_UI; 
		return new InputConnection(this, false);
	}
	
	@Override
	public boolean onCheckIsTextEditor() // required for creation of soft keyboard
	{
		return true;
	}
	
}

