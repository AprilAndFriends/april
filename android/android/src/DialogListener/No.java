package com.april.DialogListener;

/// @version 3.7

import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import com.april.NativeInterface;

public class No implements OnClickListener
{
	public void onClick(DialogInterface dialog, int id)
	{
		NativeInterface.aprilActivity.glView.queueEvent(new Runnable()
		{
			public void run()
			{
				NativeInterface.onDialogNo();
			}
		});
	}
	
}
