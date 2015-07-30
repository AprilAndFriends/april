package com.april;

/// @version 3.5

import android.app.Dialog;
import android.content.pm.ActivityInfo;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager.NameNotFoundException;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.provider.Settings;
import android.view.KeyEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import com.april.DialogFactory;

import java.util.ArrayList;

public class Activity extends android.app.Activity
{
	public boolean ouyaKeyboardFix = false; // used directly by the OUYA portion of the code to fix the software keyboard
	
	protected boolean nookWorkaround = false; // set this to true in your activity if your are using a nook build in order to speed up new intent/activity calls
	protected boolean useHardExit = true; // set this to false to prevent application from fully exiting
	private boolean enabledNavigationBarHiding = true;
	
	protected SystemSettingsObserver systemSettingsObserver = null;
	
	public void forceArchivePath(String archivePath) // use this code in your Activity to force APK as archive file
	{
		NativeInterface.archivePath = archivePath;
	}
	
	public void disableNavigationBarHiding() // use this code in your Activity to prevent the navigation bar from being hidden on Android 4.4+
	{
		this.enabledNavigationBarHiding = false;
	}
	
	public boolean isEnabledNavigationBarHiding() // use this code in your Activity to prevent the navigation bar from being hidden on Android 4.4+
	{
		return (this.enabledNavigationBarHiding && Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT);
	}
	
	public GLSurfaceView glView = null;
	public ArrayList ignoredKeys = null;
	
	public Activity()
	{
		super();
		Thread.currentThread().setPriority(Thread.MAX_PRIORITY);
		this.ignoredKeys = new ArrayList();
		this.ignoredKeys.add(KeyEvent.KEYCODE_VOLUME_DOWN);
		this.ignoredKeys.add(KeyEvent.KEYCODE_VOLUME_UP);
		this.ignoredKeys.add(KeyEvent.KEYCODE_VOLUME_MUTE);
		this.systemSettingsObserver = new SystemSettingsObserver();
	}
	
	public View getView()
	{
		return this.glView;
	}
	
	public String createDataPath()
	{
		return (Environment.getExternalStorageDirectory() + "/Android/obb/" +
			NativeInterface.packageName + "/main." + NativeInterface.versionCode + "." + NativeInterface.packageName + ".obb");
	}
	
	@Override
	protected void onCreate(Bundle savedInstanceState)
	{
		android.util.Log.i("april", "Initializing april Activity class.");
		android.util.Log.i("april", "Android device: '" + Build.MANUFACTURER + "' / '" + Build.MODEL + "'");
		this.hideNavigationBar();
		super.onCreate(savedInstanceState);
		NativeInterface.activity = (android.app.Activity)this;
		NativeInterface.aprilActivity = this;
		this.getContentResolver().registerContentObserver(Settings.System.getUriFor(Settings.System.ACCELEROMETER_ROTATION), true, this.systemSettingsObserver);
		this.systemSettingsObserver.onChange(true);
		this.getWindow().setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_STATE_ALWAYS_HIDDEN);
		NativeInterface.packageName = this.getPackageName();
		try
		{
			PackageInfo info = this.getPackageManager().getPackageInfo(NativeInterface.packageName, 0);
			NativeInterface.versionCode = Integer.toString(info.versionCode);
			NativeInterface.versionName = info.versionName;
			NativeInterface.apkPath = info.applicationInfo.sourceDir;
			NativeInterface.dataPath = this.createDataPath();
		}
		catch (NameNotFoundException e)
		{
		}
		// creating a GL surface view
		this.glView = this.createGlView();
		this.setContentView(this.glView);
		// focusing this view allows proper input processing
		this.glView.requestFocus();
		this.glView.requestFocusFromTouch();
		if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT)
		{
			// hide navigation bar
			this.getWindow().getDecorView().setOnSystemUiVisibilityChangeListener(new View.OnSystemUiVisibilityChangeListener()
			{
				@Override
				public void onSystemUiVisibilityChange(int visibility)
				{
					hideNavigationBar();
				}
			});
		}
		else if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN)
		{
			this.getWindow().getDecorView().setSystemUiVisibility(
				View.SYSTEM_UI_FLAG_LAYOUT_STABLE |
				View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN |
				View.SYSTEM_UI_FLAG_FULLSCREEN | 
				View.SYSTEM_UI_FLAG_LOW_PROFILE);
		}
		else if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.ICE_CREAM_SANDWICH)
		{
			this.getWindow().getDecorView().setSystemUiVisibility(
				View.SYSTEM_UI_FLAG_FULLSCREEN | 
				View.SYSTEM_UI_FLAG_LOW_PROFILE);
		}
		NativeInterface.activityOnCreate();
	}
	
	protected void hideNavigationBar()
	{
		if (this.isEnabledNavigationBarHiding())
		{
			this.getWindow().getDecorView().setSystemUiVisibility(
				View.SYSTEM_UI_FLAG_LAYOUT_STABLE |
				View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN |
				View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION |
				View.SYSTEM_UI_FLAG_FULLSCREEN |
				View.SYSTEM_UI_FLAG_HIDE_NAVIGATION |
				View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY);
		}
	}
	
	@Override
	protected void onStart()
	{
		super.onStart();
		this.glView.queueEvent(new Runnable()
		{
			public void run()
			{
				NativeInterface.activityOnStart();
			}
		});
	}

	@Override
	protected void onResume()
	{
		super.onResume();
		this.glView.queueEvent(new Runnable()
		{
			public void run()
			{
				NativeInterface.activityOnResume();
			}
		});
		this.hideNavigationBar();
		if (!this.nookWorkaround)
		{
			this.glView.onResume();
		}
	}
	
	@Override
	protected void onPause()
	{
		if (!this.nookWorkaround)
		{
			this.glView.onPause();
		}
		this.glView.queueEvent(new Runnable()
		{
			public void run()
			{
				NativeInterface.activityOnPause();
			}
		});
		super.onPause();
	}
	
	@Override
	protected void onStop()
	{
		this.glView.queueEvent(new Runnable()
		{
			public void run()
			{
				NativeInterface.activityOnStop();
			}
		});
		super.onStop();
	}
	
	@Override
	public void onDestroy()
	{
		NativeInterface.activityOnDestroy();
		NativeInterface.destroy();
		NativeInterface.reset();
		super.onDestroy();
		if (this.useHardExit)
		{
			System.runFinalizersOnExit(true);
			System.exit(0);
		}
	}
	
	@Override
	protected void onRestart()
	{
		super.onRestart();
		this.glView.queueEvent(new Runnable()
		{
			public void run()
			{
				NativeInterface.activityOnRestart();
			}
		});
	}
	
	@Override
	public void onLowMemory()
	{
		this.glView.queueEvent(new Runnable()
		{
			public void run()
			{
				NativeInterface.onLowMemory();
			}
		});
		super.onLowMemory();
	}
	
	@Override
	public void onWindowFocusChanged(boolean focused)
	{
		super.onWindowFocusChanged(focused);
		if (focused)
		{
			this.hideNavigationBar();
		}
	}
	
	@Override
	protected Dialog onCreateDialog(int id)
	{
		return DialogFactory.show();
	}
	
	@Override
	protected Dialog onCreateDialog(int id, Bundle bundle)
	{
		return DialogFactory.show();
	}
	
	protected GLSurfaceView createGlView()
	{
		return new GLSurfaceView(this);
	}
	
}
