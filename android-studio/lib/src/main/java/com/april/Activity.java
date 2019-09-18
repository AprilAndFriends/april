package com.april;

/// @version 5.2

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
import android.view.DisplayCutout;
import android.view.KeyEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.view.WindowInsets;
import android.view.WindowManager;
import com.april.DialogFactory;

import java.lang.Thread;
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
	public List<ICallback1<Void, Bundle>> callbacksOnCreate = null;
	public List<ICallback<Void>> callbacksOnStart = null;
	public List<ICallback<Void>> callbacksOnResume = null;
	public List<ICallback<Void>> callbacksOnPause = null;
	public List<ICallback<Void>> callbacksOnStop = null;
	public List<ICallback<Void>> callbacksOnDestroy = null;
	public List<ICallback<Void>> callbacksOnRestart = null;
	public List<ICallback3<Boolean, Integer, Integer, Intent>> callbacksOnActivityResult = null;
	public List<ICallback1<Void, Intent>> callbacksOnNewIntent = null;
	public List<ICallback<Boolean>> callbacksOnBackPressed = null;
	public List<ICallback1<Void, Configuration>> callbacksOnConfigurationChanged = null;
	public List<ICallback3<Boolean, Integer, String[], Integer[]>> callbacksOnRequestPermissionsResult = null;
	
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
		this.callbacksOnCreate = new ArrayList<ICallback1<Void, Bundle>>();
		this.callbacksOnStart = new ArrayList<ICallback<Void>>();
		this.callbacksOnResume = new ArrayList<ICallback<Void>>();
		this.callbacksOnPause = new ArrayList<ICallback<Void>>();
		this.callbacksOnStop = new ArrayList<ICallback<Void>>();
		this.callbacksOnDestroy = new ArrayList<ICallback<Void>>();
		this.callbacksOnRestart = new ArrayList<ICallback<Void>>();
		this.callbacksOnActivityResult = new ArrayList<ICallback3<Boolean, Integer, Integer, Intent>>();
		this.callbacksOnNewIntent = new ArrayList<ICallback1<Void, Intent>>();
		this.callbacksOnBackPressed = new ArrayList<ICallback<Boolean>>();
		this.callbacksOnConfigurationChanged = new ArrayList<ICallback1<Void, Configuration>>();
		this.callbacksOnRequestPermissionsResult = new ArrayList<ICallback3<Boolean, Integer, String[], Integer[]>>();
	}
	
	public void registerOnCreate(ICallback1<Void, Bundle> callback)
	{
		this.callbacksOnCreate.add(callback);
	}
	
	public void registerOnStart(ICallback<Void> ICallback)
	{
		this.callbacksOnStart.add(ICallback);
	}
	
	public void registerOnResume(ICallback<Void> ICallback)
	{
		this.callbacksOnResume.add(ICallback);
	}
	
	public void registerOnPause(ICallback<Void> ICallback)
	{
		this.callbacksOnPause.add(ICallback);
	}
	
	public void registerOnStop(ICallback<Void> ICallback)
	{
		this.callbacksOnStop.add(ICallback);
	}
	
	public void registerOnDestroy(ICallback<Void> ICallback)
	{
		this.callbacksOnDestroy.add(ICallback);
	}
	
	public void registerOnRestart(ICallback<Void> ICallback)
	{
		this.callbacksOnRestart.add(ICallback);
	}
	
	public void registerOnActivityResult(ICallback3<Boolean, Integer, Integer, Intent> callback)
	{
		this.callbacksOnActivityResult.add(callback);
	}
	
	public void registerOnNewIntent(ICallback1<Void, Intent> callback)
	{
		this.callbacksOnNewIntent.add(callback);
	}
	
	public void registerOnBackPressed(ICallback<Boolean> ICallback)
	{
		this.callbacksOnBackPressed.add(ICallback);
	}
	
	public void registerOnConfigurationChanged(ICallback1<Void, Configuration> callback)
	{
		this.callbacksOnConfigurationChanged.add(callback);
	}
	
	public void registerOnRequestPermissionsResult(ICallback3<Boolean, Integer, String[], Integer[]> callback)
	{
		this.callbacksOnRequestPermissionsResult.add(callback);
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
		Window window = this.getWindow();
		if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P)
		{
			WindowManager.LayoutParams attributes = window.getAttributes();
			attributes.layoutInDisplayCutoutMode = WindowManager.LayoutParams.LAYOUT_IN_DISPLAY_CUTOUT_MODE_SHORT_EDGES;
			window.setAttributes(attributes);
		}
		super.onCreate(savedInstanceState);
		ActivityManager manager = (ActivityManager)this.getSystemService(Context.ACTIVITY_SERVICE);
		int glesVersion = manager.getDeviceConfigurationInfo().reqGlEsVersion;
		if (glesVersion < 0x20000)
		{
			android.util.Log.e(LOG_TAG, "Minimum GLES version is " + glesVersion + ", but it should be 0x00020000! Unpredictable behavior possible!");
		}
		this.getContentResolver().registerContentObserver(Settings.System.getUriFor(Settings.System.ACCELEROMETER_ROTATION), true, this.systemSettingsObserver);
		this.systemSettingsObserver.onChange(true);
		window.setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_STATE_ALWAYS_HIDDEN);
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
		View decorView = window.getDecorView();
		if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT)
		{
			// hide navigation bar
			decorView.setOnSystemUiVisibilityChangeListener(new View.OnSystemUiVisibilityChangeListener()
			{
				@Override
				public void onSystemUiVisibilityChange(int visibility)
				{
					hideNavigationBar();
				}
			});
		}
		else
		{
			decorView.setSystemUiVisibility(
				View.SYSTEM_UI_FLAG_LAYOUT_STABLE |
				View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN |
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
		// do not change this, it can cause issues with certain Android versions
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
		// native call is queued into render thread, MUST NOT be called directly from this thread
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
		this.glView.onPause();
		super.onPause();
		// native call is queued into render thread, MUST NOT be called directly from this thread
		this.glView.queueEvent(new Runnable()
		{
			public void run()
			{
				NativeInterface.activityOnPause();
			}
		});
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
			// required Thread.sleep here, otherwise can freeze on some devices for no reason
			try
			{
				Thread.sleep(1);
			}
			catch (Exception e)
			{
				break;
			}
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
			// TODO - deprecated, find new solution
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
	public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults)
	{
		Integer[] boxedGrantResults = new Integer[grantResults.length];
		for (int i = 0; i < grantResults.length; ++i)
		{
			boxedGrantResults[i] = Integer.valueOf(grantResults[i]);
		}
		for (int i = 0; i < this.callbacksOnRequestPermissionsResult.size(); ++i)
		{
			if (this.callbacksOnRequestPermissionsResult.get(i).execute(requestCode, permissions, boxedGrantResults))
			{
				return;
			}
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

	// TODO - deprecated, implement new solution
	@Override
	protected Dialog onCreateDialog(int id)
	{
		return DialogFactory.show();
	}

	// TODO - deprecated, implement new solution
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
