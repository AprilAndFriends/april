package net.sourceforge.april.android;

// version 2.56

import android.app.AlertDialog;
import android.app.Dialog;
import android.os.Build;

import net.sourceforge.april.android.DialogListener.Cancel;
import net.sourceforge.april.android.DialogListener.Ok;
import net.sourceforge.april.android.DialogListener.OnCancel;
import net.sourceforge.april.android.DialogListener.No;
import net.sourceforge.april.android.DialogListener.Yes;

public class DialogFactory
{
	static protected AlertDialog.Builder dialogBuilder = null;
	
	static public void create(String title, String text, String ok, String yes, String no, String cancel, int iconId)
	{
		DialogFactory.dialogBuilder = new AlertDialog.Builder(NativeInterface.Activity);
		DialogFactory.dialogBuilder.setTitle(title != null ? title : "");
		DialogFactory.dialogBuilder.setMessage(text != null ? text : "");
		if (ok != null)
		{
			DialogFactory.dialogBuilder.setPositiveButton(ok, new Ok());
		}
		else
		{
			if (yes != null)
			{
				DialogFactory.dialogBuilder.setPositiveButton(yes, new Yes());
			}
			if (no != null)
			{
				DialogFactory.dialogBuilder.setNegativeButton(no, new No());
			}
		}
		if (cancel != null)
		{
			DialogFactory.dialogBuilder.setNeutralButton(cancel, new Cancel());
			DialogFactory.dialogBuilder.setCancelable(true);
			DialogFactory.dialogBuilder.setOnCancelListener(new OnCancel());
		}
		else
		{
			DialogFactory.dialogBuilder.setCancelable(false);
		}
		switch (iconId)
		{
		case 1:
			DialogFactory.dialogBuilder.setIcon(android.R.drawable.ic_dialog_info);
			break;
		case 2:
			DialogFactory.dialogBuilder.setIcon(android.R.drawable.ic_dialog_alert);
			break;
		default:
			break;
		}
		if (Build.VERSION.SDK_INT >= 11) // use DialogFragment if available, because later APIs don't work any other way
		{
			DialogFragment dialogFragment = new DialogFragment();
			dialogFragment.show(NativeInterface.Activity.getFragmentManager(), "april-dialog");
		}
		else
		{
			NativeInterface.Activity.runOnUiThread(new Runnable()
			{
				public void run()
				{
					NativeInterface.Activity.showDialog(0);
				}
			});
		}
	}
	
	static public Dialog show()
	{
		return DialogFactory.dialogBuilder.create();
	}
	
}
