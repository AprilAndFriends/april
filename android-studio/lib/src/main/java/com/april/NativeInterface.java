package com.april;

/// @version 5.2

import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.ResultReceiver;
import android.view.inputmethod.InputMethodManager;
import android.view.Display;
import android.view.DisplayCutout;
import android.view.View;
import android.view.WindowInsets;
import android.util.DisplayMetrics;

import com.april.DialogFragment;

import java.io.File;
import java.io.InputStream;
import java.io.IOException;
import java.lang.Math;
import java.lang.Runtime;
import java.util.Locale;

public class NativeInterface
{
	public static android.app.Activity activity = null;
	public static Activity aprilActivity = null;
	public static boolean running = false;
	public static boolean keyboardVisible = false;
	public static String archivePath = "";
	public static String dataPath = ".";
	public static String packageName = "";
	public static String versionCode = "0";
	public static String versionName = "";
	public static String apkPath = "";
	
	private static class KeyboardResultReceiver extends ResultReceiver
	{
		public KeyboardResultReceiver()
		{
			super(new Handler());
		}
		
		@Override
		protected void onReceiveResult(int resultCode, Bundle resultData)
		{
			boolean keyboardVisible = true;
			if (resultCode == InputMethodManager.RESULT_UNCHANGED_HIDDEN || resultCode == InputMethodManager.RESULT_HIDDEN)
			{
				keyboardVisible = false;
			}
		}
		
	};
	private static KeyboardResultReceiver keyboardResultReceiver = new KeyboardResultReceiver();
	
	public static native void setVariables(String dataPath, String forcedArchivePath);
	public static native void init(String[] args);
	public static native boolean update();
	public static native void destroy();
	public static native void onKeyDown(int keyCode, int charCode);
	public static native void onKeyUp(int keyCode);
	public static native void onChar(int charCode);
	public static native void onTouch(int type, int index, float x, float y);
	public static native void onScroll(float x, float y);
	public static native void onButtonDown(int controllerIndex, int buttonCode);
	public static native void onButtonUp(int controllerIndex, int buttonCode);
	public static native void onControllerAxisChange(int controllerIndex, int buttonCode, float axisValue);
	public static native void onAccelerometer(float x, float y, float z);
	public static native void onLinearAccelerometer(float x, float y, float z);
	public static native void onGravity(float x, float y, float z);
	public static native void onRotation(float x, float y, float z);
	public static native void onGyroscope(float x, float y, float z);
	public static native void onWindowFocusChanged(boolean focused);
	public static native void onVirtualKeyboardChanged(boolean visible, float heightRation);
	public static native void onLowMemory();
	public static native void onSurfaceCreated();
	
	public static native void activityOnCreate();
	public static native void activityOnStart();
	public static native void activityOnResume();
	public static native void activityOnResumeNotify();
	public static native void activityOnPause();
	public static native void activityOnPauseNotify();
	public static native void activityOnStop();
	public static native void activityOnDestroy();
	public static native void activityOnRestart();
	
	public static native void onDialogOk();
	public static native void onDialogYes();
	public static native void onDialogNo();
	public static native void onDialogCancel();
	
	public static void setSensorsEnabled(final boolean accelerometer, final boolean linearAccelerometer, final boolean gravity, final boolean rotation, final boolean gyroscope)
	{
		NativeInterface.aprilActivity.setSensorsEnabled(accelerometer, linearAccelerometer, gravity, rotation, gyroscope);
	}
	
	public static Object getDisplayResolution()
	{
		boolean enabledNavigationBarHiding = NativeInterface.aprilActivity.isEnabledNavigationBarHiding();
		DisplayMetrics metrics = new DisplayMetrics();
		int width = 0;
		int height = 0;
		Display display = NativeInterface.activity.getWindowManager().getDefaultDisplay();
		if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR1 && enabledNavigationBarHiding)
		{
			display.getRealMetrics(metrics);
			width = metrics.widthPixels;
			height = metrics.heightPixels;
		}
		else // older Android versions
		{
			display.getMetrics(metrics);
			width = metrics.widthPixels;
			height = metrics.heightPixels;
			if (enabledNavigationBarHiding)
			{
				// get the DecorView's size if navigation bar can be hidden
				android.graphics.Rect visibleFrame = new android.graphics.Rect();
				NativeInterface.activity.getWindow().getDecorView().getWindowVisibleDisplayFrame(visibleFrame);
				width = visibleFrame.right - visibleFrame.left;
				height = visibleFrame.bottom - visibleFrame.top;
			}
		}
		if (height > width)
		{
			int temp = height;
			height = width;
			width = temp;
		}
		// fixes problem with bottom 20 pixels being covered by Kindle Fire's menu
		if (Build.MANUFACTURER.equals("Amazon") && Build.MODEL.equals("Kindle Fire"))
		{
			height -= 20;
		}
		int[] result = {width, height};
		return result;
	}
	
	public static float getDisplayDpi()
	{
		// hardcoded exceptions for known devices that return wrong DPI
		if (Build.MANUFACTURER.equals("HTC") && Build.MODEL.equals("HTC One X"))
		{
			return 312.0f;
		}
		DisplayMetrics metrics = new DisplayMetrics();
		Display display = NativeInterface.activity.getWindowManager().getDefaultDisplay();
		if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR1)
		{
			display.getRealMetrics(metrics);
		}
		else // older Android versions
		{
			display.getMetrics(metrics);
		}
		return (float)Math.sqrt((metrics.xdpi * metrics.xdpi + metrics.ydpi * metrics.ydpi) / 2.0);
	}
	
	public static String getOsVersion()
	{
		return Build.VERSION.RELEASE;
	}
	
	public static String getLocale()
	{
		return Locale.getDefault().getLanguage();
	}
	
	public static String getLocaleVariant()
	{
		return Locale.getDefault().getCountry();
	}
	
	public static String getUserDataPath()
	{
		return NativeInterface.activity.getFilesDir().getAbsolutePath();
	}
	
	public static long getRamConsumption()
	{
		android.os.Debug.MemoryInfo info = new android.os.Debug.MemoryInfo();
		android.os.Debug.getMemoryInfo(info);
		return ((long)info.getTotalPrivateDirty() * 1024L); // because getTotalPrivateDirty() is in kB
	}
	
	public static Object getNotchOffsets()
	{
		int[] result = {0, 0, 0, 0};
		if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P)
		{
			WindowInsets insets = NativeInterface.activity.getWindow().getDecorView().getRootWindowInsets();
			if (insets != null)
			{
				DisplayCutout displayCutout = insets.getDisplayCutout();
				if (displayCutout != null)
				{
					result[0] = displayCutout.getSafeInsetLeft();
					result[1] = displayCutout.getSafeInsetTop();
					result[2] = displayCutout.getSafeInsetRight();
					result[3] = displayCutout.getSafeInsetBottom();
				}
			}
		}
		return result;
	}
	
	public static void openUrl(String url)
	{
		NativeInterface.activity.startActivity(new Intent(Intent.ACTION_VIEW, Uri.parse(url)));
	}
	
	public static void showVirtualKeyboard()
	{
		NativeInterface.activity.runOnUiThread(new Runnable()
		{
			public void run()
			{
				View view = NativeInterface.aprilActivity.getView();
				NativeInterface._getInputMethodManager().showSoftInput(view, 0, NativeInterface.keyboardResultReceiver);
			}
		});
	}
	
	public static void hideVirtualKeyboard()
	{
		NativeInterface.activity.runOnUiThread(new Runnable()
		{
			public void run()
			{
				View view = NativeInterface.aprilActivity.getView();
				NativeInterface._getInputMethodManager().hideSoftInputFromWindow(view.getWindowToken(), 0, NativeInterface.keyboardResultReceiver);
			}
		});
	}
	
	private static InputMethodManager _getInputMethodManager()
	{
		return (InputMethodManager)NativeInterface.activity.getSystemService(Context.INPUT_METHOD_SERVICE);
	}
	
	public static void showMessageBox(String title, String text, String ok, String yes, String no, String cancel, int iconId)
	{
		DialogFactory.create(title, text, ok, yes, no, cancel, iconId);
	}
	
	public static void swapBuffers()
	{
		NativeInterface.aprilActivity.glView.swapBuffers();
	}
	
	public static void reset()
	{
		NativeInterface.activity = null;
		NativeInterface.aprilActivity = null;
		NativeInterface.running = false;
		NativeInterface.archivePath = "";
		NativeInterface.dataPath = ".";
		NativeInterface.packageName = "";
		NativeInterface.versionCode = "0";
		NativeInterface.apkPath = "";
	}
	
}
