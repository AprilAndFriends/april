package net.sourceforge.april.android;

// version 2.53

import android.app.Dialog;
import android.content.pm.ActivityInfo;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager.NameNotFoundException;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.view.KeyEvent;
import android.view.View;
import android.view.WindowManager;

import java.util.ArrayList;

public class Activity extends android.app.Activity
{
	protected boolean nookWorkaround = false; // set this to true in your activity if your are using a nook build in order to speed up new intent/activity calls
	
	public void forceArchivePath(String archivePath) // use this code in your Activity to force APK as archive file
	{
		NativeInterface.ArchivePath = archivePath;
	}
	
	public GLSurfaceView GlView = null;
	private ArrayList ignoredKeys = null;
	
	public Activity()
	{
		super();
		this.ignoredKeys = new ArrayList();
		this.ignoredKeys.add(KeyEvent.KEYCODE_VOLUME_DOWN);
		this.ignoredKeys.add(KeyEvent.KEYCODE_VOLUME_UP);
		this.ignoredKeys.add(KeyEvent.KEYCODE_VOLUME_MUTE);
	}
	
	public View getView()
	{
		return this.GlView;
	}
	
	public String createDataPath()
	{
		return (Environment.getExternalStorageDirectory().getAbsolutePath() + "/Android/obb/" +
			NativeInterface.PackageName + "/main." + NativeInterface.VersionCode + "." + NativeInterface.PackageName + ".obb");
	}
	
	@Override
	protected void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		this.getWindow().setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_STATE_ALWAYS_HIDDEN);
		NativeInterface.Activity = this;
		NativeInterface.PackageName = this.getPackageName();
		try
		{
			PackageInfo info = this.getPackageManager().getPackageInfo(NativeInterface.PackageName, 0);
			NativeInterface.VersionCode = Integer.toString(info.versionCode);
			NativeInterface.ApkPath = info.applicationInfo.sourceDir;
			NativeInterface.DataPath = this.createDataPath();
		}
		catch (NameNotFoundException e)
		{
		}
		// cheap auto-rotation
		if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.GINGERBREAD)
		{
			int orientation = this.getRequestedOrientation();
			if (orientation == ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE)
			{
				this.setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_SENSOR_LANDSCAPE);
			}
			else if (orientation == ActivityInfo.SCREEN_ORIENTATION_PORTRAIT)
			{
				this.setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_SENSOR_PORTRAIT);
			}
		}
		// creating a GL surface view
		this.GlView = this.createGlView();
		this.setContentView(this.GlView);
		// focusing this view allows proper input processing
		this.GlView.requestFocus();
		this.GlView.requestFocusFromTouch();
		NativeInterface.activityOnCreate();
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
		super.onDestroy();
		System.runFinalizersOnExit(true);
		System.exit(0);
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
		NativeInterface.onLowMemory();
		super.onLowMemory();
	}
	
	protected Dialog onCreateDialog(int id)
	{
		return NativeInterface.DialogBuilder.create();
	}
	
	protected GLSurfaceView createGlView()
	{
		return new GLSurfaceView(this);
	}
	
}
