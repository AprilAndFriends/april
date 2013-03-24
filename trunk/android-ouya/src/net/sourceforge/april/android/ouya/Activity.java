package net.sourceforge.april.android.ouya;

// version 3.0

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;

public class Activity extends net.sourceforge.april.android.Activity
{
	@Override
	protected void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		tv.ouya.console.api.OuyaController.init(this);
		this.registerReceiver(new BroadcastReceiver()
		{
			@Override
			public void onReceive(Context context, Intent intent)
			{
				GlView.onWindowFocusChanged(false);
			}
		}, new IntentFilter(tv.ouya.console.api.OuyaIntent.ACTION_MENUAPPEARING));
	}
	
}
