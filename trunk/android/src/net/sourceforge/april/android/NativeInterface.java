package net.sourceforge.april.android;

// version 2.3

import android.app.AlertDialog;
import android.os.Build;
import android.util.DisplayMetrics;

import net.sourceforge.april.android.DialogListener.Cancel;
import net.sourceforge.april.android.DialogListener.Ok;
import net.sourceforge.april.android.DialogListener.OnCancel;
import net.sourceforge.april.android.DialogListener.No;
import net.sourceforge.april.android.DialogListener.Yes;

import java.io.File;
import java.io.InputStream;
import java.io.IOException;
import java.lang.Math;
import java.lang.Runtime;
import java.util.Locale;

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
	
	public static native void setVariables(String systemPath, String sharedPath, String packageName, String versionCode, String forceArchivePath);
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
	
	public static int getCpuCores()
	{
		return Runtime.getRuntime().availableProcessors();
	}
	
	public static int getDisplayDpi()
	{
		DisplayMetrics metrics = new DisplayMetrics();
		NativeInterface.Activity.getWindowManager().getDefaultDisplay().getMetrics(metrics);
		return metrics.densityDpi;
	}
	
	public static int getDeviceRam()
	{
		int result = 128;
		try
		{
			// it's not as nice a solution as MemoryInfo.totalMem, but this one is available before API level 16
			ProcessBuilder builder = new ProcessBuilder(new String[] {"/system/bin/cat", "/proc/meminfo"});
			builder.directory(new File("/system/bin/"));
			builder.redirectErrorStream(true);
			Process process = builder.start();
			InputStream input = process.getInputStream();
			byte[] buffer = new byte[1024];
			String output = "";
			while (input.read(buffer) != -1)
			{
				output += new String(buffer);
			}
			input.close();
			int startIndex = output.indexOf("MemTotal:");
			int endIndex = output.indexOf("\n", startIndex);
			String memory = output.substring(startIndex, endIndex).replace("MemTotal:", "").replace("kB", "").replace(" ", "");
			result = Math.round(Float.parseFloat(memory) / 1024);
		}
		catch (IOException e)
		{
			android.util.Log.e("april", e.toString());
		}
		catch (Exception e)
		{
			android.util.Log.e("april", e.toString());
		}
		return result;
	}
	
	public static String getLocale()
	{
		return Locale.getDefault().getLanguage();
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


// just a special helper class for getting the output of a process
class CmdExecute
{
	public synchronized String run(String[] cmd, String workdirectory)
	{
		String result = "";
		try
		{
			ProcessBuilder builder = new ProcessBuilder(cmd);
			// set working directory
			if (workdirectory != null)
			{
				builder.directory(new File(workdirectory));
				builder.redirectErrorStream(true);
				Process process = builder.start();
				InputStream in = process.getInputStream();
				byte[] re = new byte[1024];
				while (in.read(re) != -1)
				{
					result += new String(re);
				}
				in.close();
			}
		}
		catch (Exception ex)
		{
		}
		return result;
	}

}