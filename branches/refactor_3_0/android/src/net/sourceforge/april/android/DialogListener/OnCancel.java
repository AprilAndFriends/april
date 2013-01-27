package net.sourceforge.april.android.DialogListener;

// version 2.53

import android.content.DialogInterface;
import android.content.DialogInterface.OnCancelListener;
import net.sourceforge.april.android.NativeInterface;

public class OnCancel implements OnCancelListener
{
	public void onCancel(DialogInterface dialog)
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

