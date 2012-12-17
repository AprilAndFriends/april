package net.sourceforge.april.android;

// version 2.5

import android.view.inputmethod.BaseInputConnection;
import android.view.KeyEvent;
import android.view.View;

class InputConnection extends BaseInputConnection
{
	private final android.view.View view;
	private final KeyEvent delKeyDownEvent = new KeyEvent(KeyEvent.ACTION_DOWN, KeyEvent.KEYCODE_DEL);
	private final KeyEvent delKeyUpEvent = new KeyEvent(KeyEvent.ACTION_UP, KeyEvent.KEYCODE_DEL);
	
	public InputConnection(View view, boolean fullEditor)
	{
		super(view, fullEditor);
		this.view = view;
	}
	
	@Override
	public boolean deleteSurroundingText(int leftLength, int rightLength)
	{
		// Android SDK 16+ doesn't send key events for backspace but calls this method
		this.view.onKeyDown(KeyEvent.KEYCODE_DEL, this.delKeyDownEvent);
		this.view.onKeyUp(KeyEvent.KEYCODE_DEL, this.delKeyUpEvent);
		return super.deleteSurroundingText(leftLength, rightLength);
	}
	
}
