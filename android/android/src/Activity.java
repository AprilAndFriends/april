package com.april;

/// @version 5.0

import android.app.ActivityManager;
import android.app.Dialog;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.content.pm.ConfigurationInfo;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager.NameNotFoundException;
import android.content.res.Configuration;
import android.hardware.Sensor;
import android.hardware.SensorManager;
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

public class Activity extends android.app.Activity implements IActivityEvents
{
	private static final String LOG_TAG = "april";
	
	protected boolean useHardExit = true; // set this to false to prevent application from fully exiting
	private boolean enabledNavigationBarHiding = true;
	protected boolean waiting = false;
	
	protected SystemSettingsObserver systemSettingsObserver = null;
	protected SensorEventListener sensorEventListener = new SensorEventListener();
	protected SensorManager sensorManager = null;
	protected Sensor sensorAccelerometer = null;
	protected Sensor sensorLinearAccelerometer = null;
	protected Sensor sensorGravity = null;
	protected Sensor sensorRotation = null;
	protected Sensor sensorGyroscope = null;
	
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
	
	public void setSensorsEnabled(final boolean accelerometer, final boolean linearAccelerometer, final boolean gravity, final boolean rotation, final boolean gyroscope)
	{
		if (this.sensorManager == null && (accelerometer || linearAccelerometer || gravity || rotation || gyroscope))
		{
			this.sensorManager = (SensorManager)this.getSystemService(Context.SENSOR_SERVICE);
		}
		if (accelerometer)
		{
			if (this.sensorAccelerometer == null)
			{
				this.sensorAccelerometer = this.sensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER);
			}
		}
		else
		{
			this.sensorAccelerometer = null;
		}
		if (linearAccelerometer)
		{
			if (this.sensorLinearAccelerometer == null)
			{
				this.sensorLinearAccelerometer = this.sensorManager.getDefaultSensor(Sensor.TYPE_LINEAR_ACCELERATION);
			}
		}
		else
		{
			this.sensorLinearAccelerometer = null;
		}
		if (gravity)
		{
			if (this.sensorGravity == null)
			{
				this.sensorGravity = this.sensorManager.getDefaultSensor(Sensor.TYPE_GRAVITY);
			}
		}
		else
		{
			this.sensorGravity = null;
		}
		if (rotation)
		{
			if (this.sensorRotation == null)
			{
				this.sensorRotation = this.sensorManager.getDefaultSensor(Sensor.TYPE_ROTATION_VECTOR);
			}
		}
		else
		{
			this.sensorRotation = null;
		}
		if (gyroscope)
		{
			if (this.sensorGyroscope == null)
			{
				this.sensorGyroscope = this.sensorManager.getDefaultSensor(Sensor.TYPE_GYROSCOPE);
			}
		}
		else
		{
			this.sensorGyroscope = null;
		}
		if (this.sensorManager != null)
		{
			this.registerSensorEventListener();
		}
	}
	
	public String createDataPath()
	{
		return (Environment.getExternalStorageDirectory() + "/Android/obb/" +
			NativeInterface.packageName + "/main." + NativeInterface.versionCode + "." + NativeInterface.packageName + ".obb");
	}
	
	@Override
	protected void onCreate(Bundle savedInstanceState)
	{
		android.util.Log.i(LOG_TAG, "Creating APRIL Activity class.");
		android.util.Log.i(LOG_TAG, "Android device: '" + Build.MANUFACTURER + "' / '" + Build.MODEL + "'");
		this.hideNavigationBar();
		super.onCreate(savedInstanceState);
		ActivityManager manager = (ActivityManager)this.getSystemService(Context.ACTIVITY_SERVICE);
		int glesVersion = manager.getDeviceConfigurationInfo().reqGlEsVersion;
		if (glesVersion < 0x20000)
		{
			android.util.Log.w(LOG_TAG, "Minimum GLES version is " + glesVersion + ", but it should be 0x00020000! Unpredictable behavior possible!");
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
	
	protected void registerSensorEventListener()
	{
		if (this.sensorAccelerometer != null)
		{
			this.sensorManager.registerListener(this.sensorEventListener, this.sensorAccelerometer, SensorManager.SENSOR_DELAY_NORMAL);
		}
		if (this.sensorLinearAccelerometer != null)
		{
			this.sensorManager.registerListener(this.sensorEventListener, this.sensorLinearAccelerometer, SensorManager.SENSOR_DELAY_NORMAL);
		}
		if (this.sensorGravity != null)
		{
			this.sensorManager.registerListener(this.sensorEventListener, this.sensorGravity, SensorManager.SENSOR_DELAY_NORMAL);
		}
		if (this.sensorRotation != null)
		{
			this.sensorManager.registerListener(this.sensorEventListener, this.sensorRotation, SensorManager.SENSOR_DELAY_NORMAL);
		}
		if (this.sensorGyroscope != null)
		{
			this.sensorManager.registerListener(this.sensorEventListener, this.sensorGyroscope, SensorManager.SENSOR_DELAY_NORMAL);
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
		// native call is queued into render thread
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
		NativeInterface.activityOnResumeNotify();
		super.onResume();
		this.glView.onResume();
		for (int i = 0; i < this.callbacksOnResume.size(); ++i)
		{
			this.callbacksOnResume.get(i).execute();
		}
		this.hideNavigationBar();
		if (this.sensorManager != null)
		{
			this.registerSensorEventListener();
		}
		// native call is queued into render thread
		this.glView.queueEvent(new Runnable()
		{
			public void run()
			{
				NativeInterface.activityOnResume();
			}
		});
	}
	
	@Override
	protected void onPause()
	{
		NativeInterface.activityOnPauseNotify();
		if (this.sensorManager != null)
		{
			this.sensorManager.unregisterListener(this.sensorEventListener);
		}
		for (int i = 0; i < this.callbacksOnPause.size(); ++i)
		{
			this.callbacksOnPause.get(i).execute();
		}
		this.waiting = true;
		// native call is queued into render thread
		this.glView.queueEvent(new Runnable()
		{
			public void run()
			{
				NativeInterface.activityOnPause();
				waiting = false;
			}
		});
		while (this.waiting)
		{
		}
		this.glView.onPause();
		super.onPause();
	}
	
	@Override
	protected void onStop()
	{
		for (int i = 0; i < this.callbacksOnStop.size(); ++i)
		{
			this.callbacksOnStop.get(i).execute();
		}
		super.onStop();
		// native call is queued into render thread
		this.glView.queueEvent(new Runnable()
		{
			public void run()
			{
				NativeInterface.activityOnStop();
			}
		});
	}
	
	@Override
	public void onDestroy()
	{
		for (int i = 0; i < this.callbacksOnDestroy.size(); ++i)
		{
			this.callbacksOnDestroy.get(i).execute();
		}
		this.waiting = true;
		// native call is queued into render thread
		this.glView.queueEvent(new Runnable()
		{
			public void run()
			{
				NativeInterface.activityOnDestroy();
				waiting = false;
			}
		});
		while (this.waiting)
		{
		}
		// do not change the order of these!
		NativeInterface.destroy();
		this.glView.destroy();
		this.glView = null;
		System.gc();
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
		// native call is queued into render thread
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
