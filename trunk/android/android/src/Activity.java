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
import android.view.WindowManager;
import com.april.DialogFactory;

import java.util.ArrayList;

public class Activity extends android.app.Activity
{
	public boolean OuyaKeyboardFix = false; // used directly by the OUYA portion of the code to fix the software keyboard
	
	protected boolean nookWorkaround = false; // set this to true in your activity if your are using a nook build in order to speed up new intent/activity calls
	protected boolean useHardExit = true; // set this to false to prevent application from fully exiting
	private boolean enabledNavigationBarHiding = true;
	
	protected SystemSettingsObserver systemSettingsObserver = null;
	
	public void forceArchivePath(String archivePath) // use this code in your Activity to force APK as archive file
	{
		NativeInterface.ArchivePath = archivePath;
	}
	
	public void disableNavigationBarHiding() // use this code in your Activity to prevent the navigation bar from being hidden on Android 4.4+
	{
		this.enabledNavigationBarHiding = false;
	}
	
	public boolean isEnabledNavigationBarHiding() // use this code in your Activity to prevent the navigation bar from being hidden on Android 4.4+
	{
		return (this.enabledNavigationBarHiding && Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT);
	}
	
	public GLSurfaceView GlView = null;
	private ArrayList ignoredKeys = null;
	
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
		return this.GlView;
	}
	
	public String createDataPath()
	{
		return (Environment.getExternalStorageDirectory() + "/Android/obb/" +
			NativeInterface.PackageName + "/main." + NativeInterface.VersionCode + "." + NativeInterface.PackageName + ".obb");
	}
	
	@Override
	protected void onCreate(Bundle savedInstanceState)
	{
		android.util.Log.i("april", "Initializing april Activity class.");
		android.util.Log.i("april", "Android device: '" + Build.MANUFACTURER + "' / '" + Build.MODEL + "'");
		this.hideNavigationBar();
		super.onCreate(savedInstanceState);
		NativeInterface.Activity = (android.app.Activity)this;
		NativeInterface.AprilActivity = this;
		this.getContentResolver().registerContentObserver(Settings.System.getUriFor(Settings.System.ACCELEROMETER_ROTATION), true, this.systemSettingsObserver);
		this.systemSettingsObserver.onChange(true);
		this.getWindow().setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_STATE_ALWAYS_HIDDEN);
		NativeInterface.PackageName = this.getPackageName();
		try
		{
			PackageInfo info = this.getPackageManager().getPackageInfo(NativeInterface.PackageName, 0);
			NativeInterface.VersionCode = Integer.toString(info.versionCode);
			NativeInterface.VersionName = Integer.toString(info.versionName);
			NativeInterface.ApkPath = info.applicationInfo.sourceDir;
			NativeInterface.DataPath = this.createDataPath();
		}
		catch (NameNotFoundException e)
		{
		}
		// creating a GL surface view
		this.GlView = this.createGlView();
		this.setContentView(this.GlView);
		// focusing this view allows proper input processing
		this.GlView.requestFocus();
		this.GlView.requestFocusFromTouch();
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
		NativeInterface.activityOnCreate();
	}
	
	protected void hideNavigationBar()
	{
		if (this.isEnabledNavigationBarHiding())
		{
			this.getWindow().getDecorView().setSystemUiVisibility(View.SYSTEM_UI_FLAG_LAYOUT_STABLE |
				View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION |
				View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN |
				View.SYSTEM_UI_FLAG_HIDE_NAVIGATION |
				View.SYSTEM_UI_FLAG_FULLSCREEN |
				View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY);
		}
	}
	
	@Override
	protected void onStart()
	{
		super.onStart();
		this.GlView.queueEvent(new Runnable()
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
		this.GlView.queueEvent(new Runnable()
		{
			public void run()
			{
				NativeInterface.activityOnResume();
			}
		});
		this.hideNavigationBar();
		if (!this.nookWorkaround)
		{
			this.GlView.onResume();
		}
	}
	
	@Override
	protected void onPause()
	{
		if (!this.nookWorkaround)
		{
			this.GlView.onPause();
		}
		this.GlView.queueEvent(new Runnable()
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
		this.GlView.queueEvent(new Runnable()
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
		this.GlView.queueEvent(new Runnable()
		{
			public void run()
			{
				NativeInterface.activityOnRestart();
			}
		});
	}
	
	@Override
	public boolean onKeyDown(int keyCode, final KeyEvent event)
	{
		if (this.ignoredKeys.contains(event.getKeyCode()))
		{
			return false;
		}
		this.GlView.queueEvent(new Runnable()
		{
			public void run()
			{
				NativeInterface.onKeyDown(event.getKeyCode(), event.getUnicodeChar());
			}
		});
		return true;
	}
	
	@Override
	public boolean onKeyUp(int keyCode, final KeyEvent event)
	{
		if (this.ignoredKeys.contains(event.getKeyCode()))
		{
			return false;
		}
		this.GlView.queueEvent(new Runnable()
		{
			public void run()
			{
				NativeInterface.onKeyUp(event.getKeyCode());
			}
		});
		return true;
	}
	
	@Override
	public void onLowMemory()
	{
		this.GlView.queueEvent(new Runnable()
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
