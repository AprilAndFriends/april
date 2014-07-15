package com.april.DialogListener;

/// @version 3.5

import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import com.april.NativeInterface;

public class Cancel implements OnClickListener
{
	public void onClick(DialogInterface dialog, int id)
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
