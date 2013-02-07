package net.sourceforge.april.android;

// version 2.55

import android.app.AlertDialog;
import android.app.Dialog;
import android.os.Bundle;

import net.sourceforge.april.android.DialogListener.Cancel;
import net.sourceforge.april.android.DialogListener.Ok;
import net.sourceforge.april.android.DialogListener.OnCancel;
import net.sourceforge.april.android.DialogListener.No;
import net.sourceforge.april.android.DialogListener.Yes;

public class DialogFragment extends android.app.DialogFragment
{
	protected String title;
	protected String text;
	protected String ok;
	protected String yes;
	protected String no;
	protected String cancel;
	protected int iconId;
	
	public DialogFragment(String title, String text, String ok, String yes, String no, String cancel, int iconId)
	{
		super();
		this.title = title;
		this.text = text;
		this.ok = ok;
		this.yes = yes;
		this.no = no;
		this.cancel = cancel;
		this.iconId = iconId;
	}
	
	@Override
	public Dialog onCreateDialog(Bundle savedInstanceState)
	{
		AlertDialog.Builder dialogBuilder = new AlertDialog.Builder(NativeInterface.Activity);
		dialogBuilder.setTitle(this.title != null ? this.title : "");
		dialogBuilder.setMessage(this.text != null ? this.text : "");
		android.util.Log.w("DEBUG-J", this.text);
		if (this.ok != null)
		{
			dialogBuilder.setPositiveButton(this.ok, new Ok());
		}
		else
		{
			if (this.yes != null)
			{
				dialogBuilder.setPositiveButton(this.yes, new Yes());
			}
			if (this.no != null)
			{
				dialogBuilder.setNegativeButton(this.no, new No());
			}
		}
		if (this.cancel != null)
		{
			dialogBuilder.setNeutralButton(this.cancel, new Cancel());
			dialogBuilder.setCancelable(true);
			dialogBuilder.setOnCancelListener(new OnCancel());
		}
		else
		{
			dialogBuilder.setCancelable(false);
		}
		switch (this.iconId)
		{
		case 1:
			dialogBuilder.setIcon(android.R.drawable.ic_dialog_info);
			break;
		case 2:
			dialogBuilder.setIcon(android.R.drawable.ic_dialog_alert);
			break;
		default:
			break;
		}
		return dialogBuilder.create();
	}
	
}
