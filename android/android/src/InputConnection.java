package com.april;

/// @version 4.2

import android.view.inputmethod.BaseInputConnection;
import android.view.KeyEvent;
import android.view.View;

class InputConnection extends BaseInputConnection
{
	private final KeyEvent delKeyDownEvent = new KeyEvent(KeyEvent.ACTION_DOWN, KeyEvent.KEYCODE_DEL);
	private final KeyEvent delKeyUpEvent = new KeyEvent(KeyEvent.ACTION_UP, KeyEvent.KEYCODE_DEL);
	private final View view;
	
	public InputConnection(View view)
	{
		super(view, false);
		this.view = view;
		this.setSelection(0, 0);
	}
	
	@Override
	public boolean deleteSurroundingText(int leftLength, int rightLength)
	{
		// Android SDK 16+ doesn't send key events for backspace but calls this method, so calls are sent manually
		this.view.onKeyDown(KeyEvent.KEYCODE_DEL, this.delKeyDownEvent);
		this.view.onKeyUp(KeyEvent.KEYCODE_DEL, this.delKeyUpEvent);
		return super.deleteSurroundingText(leftLength, rightLength);
	}
	
}
