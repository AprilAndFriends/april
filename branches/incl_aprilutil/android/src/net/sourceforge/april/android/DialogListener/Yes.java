package net.sourceforge.april.android.DialogListener;

// version 2.1

import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import net.sourceforge.april.android.NativeInterface;

public class Yes implements OnClickListener
{
	public void onClick(DialogInterface dialog, int id)
	{
		NativeInterface.onDialogYes();
	}
}
