package com.april;

/// @version 5.1

import android.view.inputmethod.BaseInputConnection;
import android.view.KeyEvent;
import android.view.View;
import android.text.Editable;
import android.text.Selection;
import android.text.SpannableStringBuilder;
import java.lang.CharSequence;

class InputConnection extends BaseInputConnection
{
	// required due to some keyboards requiring an unprocessed character in order to be able to call deleteSurroundingText()
	public static CharSequence ONE_UNPROCESSED_CHARACTER = "\0";
	
	// required due to some keyboards requiring an unprocessed character in order to be able to call deleteSurroundingText()
	private class EditableWorkaround extends SpannableStringBuilder
	{
		
		EditableWorkaround(CharSequence source)
		{
			super(source);
		}

		@Override
		public SpannableStringBuilder replace(final int spannableStringStart, final int spannableStringEnd, CharSequence replacementSequence, int replacementStart, int replacementEnd)
		{
			if (replacementEnd > replacementStart)
			{
				super.replace(0, this.length(), "", 0, 0);
				return super.replace(0, 0, replacementSequence, replacementStart, replacementEnd);
			}
			if (spannableStringEnd > spannableStringStart)
			{
				super.replace(0, this.length(), "", 0, 0);
				return super.replace(0, 0, ONE_UNPROCESSED_CHARACTER, 0, 1);
			}
			return super.replace(spannableStringStart, spannableStringEnd, replacementSequence, replacementStart, replacementEnd);
		}
		
	}	
	
	private final KeyEvent delKeyDownEvent = new KeyEvent(KeyEvent.ACTION_DOWN, KeyEvent.KEYCODE_DEL);
	private final KeyEvent delKeyUpEvent = new KeyEvent(KeyEvent.ACTION_UP, KeyEvent.KEYCODE_DEL);
	private final View view;
	private Editable editable = new EditableWorkaround(ONE_UNPROCESSED_CHARACTER);
	
	public InputConnection(View view)
	{
		super(view, false);
		this.view = view;
		// required due to some keyboards requiring an unprocessed character in order to be able to call deleteSurroundingText()
		Selection.setSelection(this.editable, 1);
	}
	
	@Override
	public boolean deleteSurroundingText(int leftLength, int rightLength)
	{
		this.view.onKeyDown(KeyEvent.KEYCODE_DEL, this.delKeyDownEvent);
		this.view.onKeyUp(KeyEvent.KEYCODE_DEL, this.delKeyUpEvent);
		return super.deleteSurroundingText(leftLength, rightLength);
	}
	
	// required due to some keyboards requiring an unprocessed character in order to be able to call deleteSurroundingText()
	@Override
	public Editable getEditable()
	{
		if (this.editable.length() == 0)
		{
			this.editable.append(ONE_UNPROCESSED_CHARACTER);
			Selection.setSelection(this.editable, 1);
		}
		return this.editable;
	}	
	
}
