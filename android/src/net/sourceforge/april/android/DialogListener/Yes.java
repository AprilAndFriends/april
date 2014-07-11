package net.sourceforge.april.android.DialogListener;

// version 3.0

import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import net.sourceforge.april.android.NativeInterface;

public class Yes implements OnClickListener
{
	public void onClick(DialogInterface dialog, int id)
	{
		NativeInterface.Activity.GlView.queueEvent(new Runnable()
		{
			public void run()
			{
				NativeInterface.onDialogYes();
			}
		});
	}
	
}
