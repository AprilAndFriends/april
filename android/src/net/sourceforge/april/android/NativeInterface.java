package net.sourceforge.april.android;

// version 2.2

import android.app.AlertDialog;
import android.os.Build;
import android.util.DisplayMetrics;

import net.sourceforge.april.android.DialogListener.Cancel;
import net.sourceforge.april.android.DialogListener.Ok;
import net.sourceforge.april.android.DialogListener.OnCancel;
import net.sourceforge.april.android.DialogListener.No;
import net.sourceforge.april.android.DialogListener.Yes;

public class NativeInterface
{
	public static Activity Activity = null;
	public static boolean Running = false;
	public static String ArchivePath = "";
	public static String SystemPath = ".";
	public static String DataPath = ".";
	public static String PackageName = "";
	public static String VersionCode = "0";
	public static String ApkPath = "";
	public static AlertDialog.Builder DialogBuilder = null;
	
	public static native void setVariables(Object activity, String systemPath, String sharedPath, String packageName, String versionCode, String forceArchivePath);
	public static native void init(String[] args);
	public static native boolean render();
	public static native void destroy();
	public static native void onTouch(int type, float x, float y, int index);
	public static native boolean onKeyDown(int keyCode, int charCode);
	public static native boolean onKeyUp(int keyCode);
	public static native void onWindowFocusChanged(boolean focused);
	public static native void onLowMemory();
	public static native void onSurfaceCreated();
	
	public static native void activityOnCreate();
	public static native void activityOnStart();
	public static native void activityOnResume();
	public static native void activityOnPause();
	public static native void activityOnStop();
	public static native void activityOnDestroy();
	public static native void activityOnRestart();
	
	public static native void onDialogOk();
	public static native void onDialogYes();
	public static native void onDialogNo();
	public static native void onDialogCancel();
	
	public static Object getDisplayResolution()
	{
		DisplayMetrics metrics = new DisplayMetrics();
		NativeInterface.Activity.getWindowManager().getDefaultDisplay().getMetrics(metrics);
		int width = metrics.widthPixels;
		int height = metrics.heightPixels;
		if (height > width)
		{
			height = metrics.widthPixels;
			width = metrics.heightPixels;
		}
		// fixes problem with bottom 20 pixels being covered by Kindle Fire's menu
		if (Build.MANUFACTURER == "Amazon" && Build.MODEL == "Kindle Fire")
		{
			height -= 20;
		}
		int[] result = {width, height};
		return result;
	}
	
	public static void showMessageBox(String title, String text, String ok, String yes, String no, String cancel, int iconId)
	{
		NativeInterface.DialogBuilder = new AlertDialog.Builder(NativeInterface.Activity);
		NativeInterface.DialogBuilder.setTitle(title != null ? title : "");
		NativeInterface.DialogBuilder.setMessage(text != null ? text : "");
		if (ok != null)
		{
			NativeInterface.DialogBuilder.setPositiveButton(ok, new Ok());
		}
		else
		{
			if (yes != null)
			{
				NativeInterface.DialogBuilder.setPositiveButton(yes, new Yes());
			}
			if (no != null)
			{
				NativeInterface.DialogBuilder.setNegativeButton(no, new No());
			}
		}
		if (cancel != null)
		{
			NativeInterface.DialogBuilder.setNeutralButton(cancel, new Cancel());
			NativeInterface.DialogBuilder.setCancelable(true);
			NativeInterface.DialogBuilder.setOnCancelListener(new OnCancel());
		}
		else
		{
			NativeInterface.DialogBuilder.setCancelable(false);
		}
		switch (iconId)
		{
		case 1:
			NativeInterface.DialogBuilder.setIcon(android.R.drawable.ic_dialog_info);
			break;
		case 2:
			NativeInterface.DialogBuilder.setIcon(android.R.drawable.ic_dialog_alert);
			break;
		default:
			break;
		}
		NativeInterface.Activity.runOnUiThread(new Runnable()
		{
			public void run()
			{
				NativeInterface.Activity.showDialog(0);
			}
		});
	}
	
}
