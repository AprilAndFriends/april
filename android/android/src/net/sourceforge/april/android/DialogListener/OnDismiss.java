package net.sourceforge.april.android.DialogListener;

// version 3.0

import android.content.DialogInterface;
import android.content.DialogInterface.OnDismissListener;
import net.sourceforge.april.android.NativeInterface;

public class OnDismiss implements OnDismissListener
{
	public void onDismiss(DialogInterface dialog)
	{
		NativeInterface.AprilActivity.GlView.queueEvent(new Runnable()
		{
			public void run()
			{
				NativeInterface.onDialogCancel(); // dismiss is like cancel button in April
			}
		});
	}
	
}

