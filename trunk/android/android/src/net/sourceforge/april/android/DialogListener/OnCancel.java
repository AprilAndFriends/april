package net.sourceforge.april.android.DialogListener;

// version 3.0

import android.content.DialogInterface;
import android.content.DialogInterface.OnCancelListener;
import net.sourceforge.april.android.NativeInterface;

public class OnCancel implements OnCancelListener
{
	public void onCancel(DialogInterface dialog)
	{
		NativeInterface.AprilActivity.GlView.queueEvent(new Runnable()
		{
			public void run()
			{
				NativeInterface.onDialogCancel();
			}
		});
	}
	
}

