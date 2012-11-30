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
		// Ericsson Xperia devices don't send key events but call this method for some reason when handling the delete key
		this.view.onKeyDown(KeyEvent.KEYCODE_DEL, this.delKeyDownEvent);
		this.view.onKeyUp(KeyEvent.KEYCODE_DEL, this.delKeyUpEvent);
		return super.deleteSurroundingText(leftLength, rightLength);
	}
	
}
