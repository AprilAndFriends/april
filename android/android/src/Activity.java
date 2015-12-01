package com.april;

/// @version 4.0

import android.app.ActivityManager;
import android.app.Dialog;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.content.pm.ConfigurationInfo;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager.NameNotFoundException;
import android.content.res.Configuration;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.provider.Settings;
import android.view.KeyEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import com.april.DialogFactory;

import java.util.List;
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
	public List<Callback1<Void, Bundle>> callbacksOnCreate = null;
	public List<Callback<Void>> callbacksOnStart = null;
	public List<Callback<Void>> callbacksOnResume = null;
	public List<Callback<Void>> callbacksOnPause = null;
	public List<Callback<Void>> callbacksOnStop = null;
	public List<Callback<Void>> callbacksOnDestroy = null;
	public List<Callback<Void>> callbacksOnRestart = null;
	public List<Callback3<Boolean, Integer, Integer, Intent>> callbacksOnActivityResult = null;
	public List<Callback1<Void, Intent>> callbacksOnNewIntent = null;
	public List<Callback<Boolean>> callbacksOnBackPressed = null;
	public List<Callback1<Void, Configuration>> callbacksOnConfigurationChanged = null;
	
	public Activity()
	{
		super();
		NativeInterface.activity = (android.app.Activity)this;
		NativeInterface.aprilActivity = this;
		Thread.currentThread().setPriority(Thread.MAX_PRIORITY);
		this.ignoredKeys = new ArrayList();
		this.ignoredKeys.add(KeyEvent.KEYCODE_VOLUME_DOWN);
		this.ignoredKeys.add(KeyEvent.KEYCODE_VOLUME_UP);
		this.ignoredKeys.add(KeyEvent.KEYCODE_VOLUME_MUTE);
		this.systemSettingsObserver = new SystemSettingsObserver();
		this.callbacksOnCreate = new ArrayList<Callback1<Void, Bundle>>();
		this.callbacksOnStart = new ArrayList<Callback<Void>>();
		this.callbacksOnResume = new ArrayList<Callback<Void>>();
		this.callbacksOnPause = new ArrayList<Callback<Void>>();
		this.callbacksOnStop = new ArrayList<Callback<Void>>();
		this.callbacksOnDestroy = new ArrayList<Callback<Void>>();
		this.callbacksOnRestart = new ArrayList<Callback<Void>>();
		this.callbacksOnActivityResult = new ArrayList<Callback3<Boolean, Integer, Integer, Intent>>();
		this.callbacksOnNewIntent = new ArrayList<Callback1<Void, Intent>>();
		this.callbacksOnBackPressed = new ArrayList<Callback<Boolean>>();
		this.callbacksOnConfigurationChanged = new ArrayList<Callback1<Void, Configuration>>();
	}
	
	public void registerOnCreate(Callback1<Void, Bundle> callback)
	{
		this.callbacksOnCreate.add(callback);
	}
	
	public void registerOnStart(Callback<Void> callback)
	{
		this.callbacksOnStart.add(callback);
	}
	
	public void registerOnResume(Callback<Void> callback)
	{
		this.callbacksOnResume.add(callback);
	}
	
	public void registerOnPause(Callback<Void> callback)
	{
		this.callbacksOnPause.add(callback);
	}
	
	public void registerOnStop(Callback<Void> callback)
	{
		this.callbacksOnStop.add(callback);
	}
	
	public void registerOnDestroy(Callback<Void> callback)
	{
		this.callbacksOnDestroy.add(callback);
	}
	
	public void registerOnRestart(Callback<Void> callback)
	{
		this.callbacksOnRestart.add(callback);
	}
	
	public void registerOnActivityResult(Callback3<Boolean, Integer, Integer, Intent> callback)
	{
		this.callbacksOnActivityResult.add(callback);
	}
	
	public void registerOnNewIntent(Callback1<Void, Intent> callback)
	{
		this.callbacksOnNewIntent.add(callback);
	}
	
	public void registerOnBackPressed(Callback<Boolean> callback)
	{
		this.callbacksOnBackPressed.add(callback);
	}
	
	public void registerOnConfigurationChanged(Callback1<Void, Configuration> callback)
	{
		this.callbacksOnConfigurationChanged.add(callback);
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
		ActivityManager manager = (ActivityManager)this.getSystemService(Context.ACTIVITY_SERVICE);
		if (manager.getDeviceConfigurationInfo().reqGlEsVersion < 0x20000)
		{
			android.util.Log.w("april", "Minimum GLES version should be 2! Unpredictable behavior possible!");
		}
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
		for (int i = 0; i < this.callbacksOnCreate.size(); ++i)
		{
			this.callbacksOnCreate.get(i).execute(savedInstanceState);
		}
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
		for (int i = 0; i < this.callbacksOnStart.size(); ++i)
		{
			this.callbacksOnStart.get(i).execute();
		}
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
		for (int i = 0; i < this.callbacksOnResume.size(); ++i)
		{
			this.callbacksOnResume.get(i).execute();
		}
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
		for (int i = 0; i < this.callbacksOnPause.size(); ++i)
		{
			this.callbacksOnPause.get(i).execute();
		}
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
		for (int i = 0; i < this.callbacksOnStop.size(); ++i)
		{
			this.callbacksOnStop.get(i).execute();
		}
		super.onStop();
	}
	
	@Override
	public void onDestroy()
	{
		for (int i = 0; i < this.callbacksOnDestroy.size(); ++i)
		{
			this.callbacksOnDestroy.get(i).execute();
		}
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
		for (int i = 0; i < this.callbacksOnRestart.size(); ++i)
		{
			this.callbacksOnRestart.get(i).execute();
		}
		this.glView.queueEvent(new Runnable()
		{
			public void run()
			{
				NativeInterface.activityOnRestart();
			}
		});
	}
	
	@Override
	public void onActivityResult(int requestCode, int resultCode, Intent data)
	{
		boolean handled = false;
		for (int i = 0; i < this.callbacksOnActivityResult.size(); ++i)
		{
			if (this.callbacksOnActivityResult.get(i).execute(requestCode, resultCode, data))
			{
				handled = true;
			}
		}
		if (!handled)
		{
			super.onActivityResult(requestCode, resultCode, data);
		}
	}
	
	@Override
	protected void onNewIntent(Intent intent)
	{
		super.onNewIntent(intent);
		for (int i = 0; i < this.callbacksOnNewIntent.size(); ++i)
		{
			this.callbacksOnNewIntent.get(i).execute(intent);
		}
	}
	
	@Override
	public void onBackPressed()
	{
		boolean handled = false;
		for (int i = 0; i < this.callbacksOnBackPressed.size(); ++i)
		{
			if (this.callbacksOnBackPressed.get(i).execute())
			{
				handled = true;
			}
		}
		if (!handled)
		{
			super.onBackPressed();
		}
	}
	
	@Override
	public void onConfigurationChanged(Configuration newConfiguration)
	{
		super.onConfigurationChanged(newConfiguration);
		for (int i = 0; i < this.callbacksOnConfigurationChanged.size(); ++i)
		{
			this.callbacksOnConfigurationChanged.get(i).execute(newConfiguration);
		}
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
