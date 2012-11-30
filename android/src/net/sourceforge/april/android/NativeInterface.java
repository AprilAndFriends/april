package net.sourceforge.april.android;

// version 2.5

import android.app.AlertDialog;
import android.content.Context;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.ResultReceiver;
import android.view.inputmethod.InputMethodManager;
import android.view.View;
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
	public static String DataPath = ".";
	public static String PackageName = "";
	public static String VersionCode = "0";
	public static String ApkPath = "";
	public static AlertDialog.Builder DialogBuilder = null;
	
	private static boolean htcKeyboardHack = false;
	private static ResultReceiver keyboardResultReceiver = new ResultReceiver(new Handler()
	{
		protected void onReceiveResult(int resultCode, Bundle resultData)
		{
			boolean keyboardShown = true;
			if (resultCode == InputMethodManager.RESULT_UNCHANGED_HIDDEN ||
				resultCode == InputMethodManager.RESULT_HIDDEN)
			{
				keyboardShown = false;
			}
			if (keyboardShown && htcKeyboardHack)
			{
				htcKeyboardHack = false;
				InputMethodManager inputMethodManager = NativeInterface._getInputMethodManager();
				View view = NativeInterface.Activity.getView();
				inputMethodManager.hideSoftInputFromWindow(view.getWindowToken(), 0, NativeInterface.keyboardResultReceiver);
				inputMethodManager.showSoftInput(view, 0, NativeInterface.keyboardResultReceiver);
			}
		}
	});

	
	public static native void setVariables(String dataPath, String forcedArchivePath);
	public static native void init(String[] args);
	public static native boolean render();
	public static native void destroy();
	public static native void onTouch(int type, float x, float y, int index);
	public static native void onKeyDown(int keyCode, int charCode);
	public static native void onKeyUp(int keyCode);
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
	
	public static String getUserDataPath()
	{
		return NativeInterface.Activity.getFilesDir().getAbsolutePath();
	}
	
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
		return (int)Math.sqrt((metrics.xdpi * metrics.xdpi + metrics.ydpi * metrics.ydpi) / 2.0);
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
		Locale locale = Locale.getDefault();
		String result = locale.getLanguage();
		String country = locale.getCountry();
		if (result.equals("pt") && country.equals("PT")) // Java is stupid and needs "equals" instead of "=="
		{
			result += "-" + country;
		}
		return result;
	}
	
	public static void showVirtualKeyboard()
	{
		NativeInterface.Activity.runOnUiThread(new Runnable()
		{
			public void run()
			{
				View view = NativeInterface.Activity.getView();
				NativeInterface._getInputMethodManager().showSoftInput(view, 0, NativeInterface.keyboardResultReceiver);
			}
		});
	}
	
	public static void hideVirtualKeyboard()
	{
		NativeInterface.Activity.runOnUiThread(new Runnable()
		{
			public void run()
			{
				View view = NativeInterface.Activity.getView();
				NativeInterface._getInputMethodManager().hideSoftInputFromWindow(view.getWindowToken(), 0,
					NativeInterface.keyboardResultReceiver);
			}
		});
	}
	
	public static void updateKeyboard()
	{
		// TODO - detect broken versions of com.htc.android.htcime
		if (Build.BOARD.equals("mecha") ||		// Thunderbolt
			Build.BOARD.equals("marvel") ||		// Wildfire S
			Build.BOARD.equals("marvelc"))		// Wildfire S
		{
			htcKeyboardHack = true;
		}
		else if (Build.VERSION.SDK_INT < 10 &&
			Build.BOARD.equals("shooteru") ||	// EVO 3D
			Build.BOARD.equals("supersonic"))	// EVO 4G
		{
			htcKeyboardHack = true;
		}
		else if (Build.VERSION.SDK_INT >= 10 &&
			Build.BOARD.equals("inc"))			// Droid Incredible
		{
			htcKeyboardHack = true;
		}
	}
	
	private static InputMethodManager _getInputMethodManager()
	{
		return (InputMethodManager)NativeInterface.Activity.getSystemService(Context.INPUT_METHOD_SERVICE);
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
