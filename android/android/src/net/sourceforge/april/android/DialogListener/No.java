package net.sourceforge.april.android.DialogListener;

// version 3.2

import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import net.sourceforge.april.android.NativeInterface;

public class No implements OnClickListener
{
	public void onClick(DialogInterface dialog, int id)
	{
		NativeInterface.AprilActivity.GlView.queueEvent(new Runnable()
		{
			public void run()
			{
				NativeInterface.onDialogNo();
			}
		});
	}
	
}