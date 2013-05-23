package net.sourceforge.april.android;

// version 3.0

import android.app.Dialog;
import android.os.Bundle;

public class DialogFragment extends android.app.DialogFragment
{
	public DialogFragment()
	{
		super();
	}
	
	@Override
	public Dialog onCreateDialog(Bundle savedInstanceState)
	{
		return DialogFactory.show();
	}
	
}
