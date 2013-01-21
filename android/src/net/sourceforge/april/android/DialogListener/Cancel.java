package net.sourceforge.april.android.DialogListener;

// version 2.53

import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import net.sourceforge.april.android.NativeInterface;

public class Cancel implements OnClickListener
{
	public void onClick(DialogInterface dialog, int id)
	{
		NativeInterface.Activity.GlView.queueEvent(new Runnable()
		{
			public void run()
			{
				NativeInterface.onDialogCancel();
			}
		});
	}
	
}
